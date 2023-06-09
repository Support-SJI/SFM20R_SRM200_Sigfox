/**
 * Copyright (c) 2012 - 2017, Nordic Semiconductor ASA
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 * 
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 * 
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 * 
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
/**
  @defgroup dtm_standalone main.c
  @{
  @ingroup ble_sdk_app_dtm_serial
  @brief Stand-alone DTM application for UART interface.

 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "cfg_config.h"
#include "nrf.h"
#include "ble_dtm.h"
#include "boards.h"
#include "app_uart.h"
#include "nrf_fstorage.h"
#include "SEGGER_RTT.h"
#include "cfg_nvm_ctrl.h"


// @note: The BLE DTM 2-wire UART standard specifies 8 data bits, 1 stop bit, no flow control.
//        These parameters are not configurable in the BLE standard.

/**@details Maximum iterations needed in the main loop between stop bit 1st byte and start bit 2nd
 * byte. DTM standard allows 5000us delay between stop bit 1st byte and start bit 2nd byte.
 * As the time is only known when a byte is received, then the time between between stop bit 1st
 * byte and stop bit 2nd byte becomes:
 *      5000us + transmission time of 2nd byte.
 *
 * Byte transmission time is (Baud rate of 19200):
 *      10bits * 1/19200 = approx. 520 us/byte (8 data bits + start & stop bit).
 *
 * Loop time on polling UART register for received byte is defined in ble_dtm.c as:
 *   UART_POLL_CYCLE = 260 us
 *
 * The max time between two bytes thus becomes (loop time: 260us / iteration):
 *      (5000us + 520us) / 260us / iteration = 21.2 iterations.
 *
 * This is rounded down to 21.
 *
 * @note If UART bit rate is changed, this value should be recalculated as well.
 */
#define MAX_ITERATIONS_NEEDED_FOR_NEXT_BYTE ((5000 + 2 * UART_POLL_CYCLE) / UART_POLL_CYCLE)

#define MAX_TEST_DATA_BYTES     (15U)                /**< max number of test bytes to be used for tx and rx. */
#define UART_TX_BUF_SIZE 256                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 256                         /**< UART RX buffer size. */

#ifdef FEATURE_CFG_RTT_MODULE_CONTROL
bool m_cfg_dtm_uart_disabled = false;
#endif

// Error handler for UART
void uart_error_handle(app_uart_evt_t * p_event)
{
    if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
    {
//        APP_ERROR_HANDLER(p_event->data.error_communication);
#ifdef FEATURE_CFG_RTT_MODULE_CONTROL
        SEGGER_RTT_printf(0, "\n==dtm_uart comm Error! Uart Disabled!\n");
        m_cfg_dtm_uart_disabled = true;
#endif
    }
    else if (p_event->evt_type == APP_UART_FIFO_ERROR)
    {
//        APP_ERROR_HANDLER(p_event->data.error_code);
        m_cfg_dtm_uart_disabled = true;
#ifdef FEATURE_CFG_RTT_MODULE_CONTROL
        SEGGER_RTT_printf(0, "\n==dtm_uart fifo Error! Uart Disabled!\n");
        m_cfg_dtm_uart_disabled = true;
#endif
    }
}

/**@brief Function for UART initialization.
 */
static void uart_init(void)
{   
    uint32_t err_code;
    const app_uart_comm_params_t comm_params =
      {
          PIN_DEF_DTM_RX,
          PIN_DEF_DTM_TX,
          RTS_PIN_NUMBER,
          CTS_PIN_NUMBER,
          APP_UART_FLOW_CONTROL_DISABLED,
          false,
          DTM_BITRATE
      };

    APP_UART_FIFO_INIT(&comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uart_error_handle,
                       APP_IRQ_PRIORITY_LOWEST,
                       err_code);

    APP_ERROR_CHECK(err_code);
}


/**@brief Function for splitting UART command bit fields into separate command parameters for the DTM library.
 *
 * @param[in]   command   The packed UART command.
 * @return      result status from dtmlib.
 */
static uint32_t dtm_cmd_put(uint16_t command)
{
    dtm_cmd_t      command_code = (command >> 14) & 0x03;
    dtm_freq_t     freq         = (command >> 8) & 0x3F;
    uint32_t       length       = (command >> 2) & 0x3F;
    dtm_pkt_type_t payload      = command & 0x03;

    return dtm_cmd(command_code, freq, length, payload);
}


/**@brief Function for application main entry.
 *
 * @details This function serves as an adaptation layer between a 2-wire UART interface and the
 *          dtmlib. After initialization, DTM commands submitted through the UART are forwarded to
 *          dtmlib and events (i.e. results from the command) is reported back through the UART.
 */

int opCnt, ErCnt;
int dtm_mode(void)
{
    uint32_t    current_time;
    uint32_t    dtm_error_code;
    uint32_t    msb_time          = 0;     // Time when MSB of the DTM command was read. Used to catch stray bytes from "misbehaving" testers.
    bool        is_msb_read       = false; // True when MSB of the DTM command has been read and the application is waiting for LSB.
    uint16_t    dtm_cmd_from_uart = 0;     // Packed command containing command_code:freqency:length:payload in 2:6:6:2 bits.
    uint8_t     rx_byte;                   // Last byte read from UART.
    dtm_event_t result;                    // Result of a DTM operation.
#ifdef FEATURE_CFG_RTT_MODULE_CONTROL
    unsigned rtt_rd_size;
    bool rtt_remain_byte_flag = false;
    char rtt_rd_bufffer[4];   //check BUFFER_SIZE_DOWN
#endif
    uart_init();

    dtm_error_code = dtm_init();
    if (dtm_error_code != DTM_SUCCESS)
    {
        // If DTM cannot be correctly initialized, then we just return.
        return -1;
    }

#ifdef FEATURE_CFG_RTT_MODULE_CONTROL
    SEGGER_RTT_printf(0, "\n==TBC_OVER_RTT Testmode==\n");  //start marker is "==TBC_OVER_RTT Testmode==" don't change this
    memset(rtt_rd_bufffer, 0, sizeof(rtt_rd_bufffer));
#endif

    for (;;)
    {
        // Will return every timeout, 625 us.
        current_time = dtm_wait();

#ifdef FEATURE_CFG_RTT_MODULE_CONTROL
        if(rtt_remain_byte_flag)
        {
            rtt_rd_bufffer[0] = rtt_rd_bufffer[1];
            rtt_rd_bufffer[1] = 0;
            rtt_rd_size = 1;
            rtt_remain_byte_flag = false;
        }
        else
        {
            rtt_rd_size = SEGGER_RTT_Read(0, rtt_rd_bufffer, 2);  //check BUFFER_SIZE_DOWN
//            if(rtt_rd_size)SEGGER_RTT_Write(0,rtt_rd_bufffer,rtt_rd_size);  //loopback for test
        }
#endif
        if ((app_uart_get(&rx_byte) != NRF_SUCCESS)
#ifdef FEATURE_CFG_RTT_MODULE_CONTROL
            && rtt_rd_size == 0
#endif
        )
        {
            // Nothing read from the UART.
            continue;
        }

#ifdef FEATURE_CFG_RTT_MODULE_CONTROL
        if(rtt_rd_size == 0 && m_cfg_dtm_uart_disabled)
        {
            continue;
        }

        if(rtt_rd_size > 0)
        {
            m_cfg_dtm_uart_disabled = true;
        }
        
        if(rtt_rd_size == 1)
        {
            rx_byte = (uint8_t)(rtt_rd_bufffer[0]);
        }
        else if(rtt_rd_size == 2)
        {
            if(((rtt_rd_bufffer[0] == 'C') && ((rtt_rd_bufffer[1] == 'R') || (rtt_rd_bufffer[1] == 'F'))))
            {
                if((rtt_rd_bufffer[1] == 'R'))
                {
                    SEGGER_RTT_printf(0, "reset\n");
                    cfg_board_reset();  //reset work
                }
                else if(rtt_rd_bufffer[1] == 'F')
                {
                    SEGGER_RTT_printf(0, "factory reset\n");
                    cfg_nvm_factory_reset(true);  //factory work
                }
                continue;
            }
            else
            {                
                rtt_remain_byte_flag = true;
                rx_byte = rtt_rd_bufffer[0];
            }
        }
 #endif

        if (!is_msb_read)
        {
            // This is first byte of two-byte command.
            is_msb_read       = true;
            dtm_cmd_from_uart = ((dtm_cmd_t)rx_byte) << 8;
            msb_time          = current_time;

            // Go back and wait for 2nd byte of command word.
            continue;
        }

        // This is the second byte read; combine it with the first and process command
        if (current_time > (msb_time + MAX_ITERATIONS_NEEDED_FOR_NEXT_BYTE))
        {
            // More than ~5mS after msb: Drop old byte, take the new byte as MSB.
            // The variable is_msb_read will remains true.
            // Go back and wait for 2nd byte of the command word.
            dtm_cmd_from_uart = ((dtm_cmd_t)rx_byte) << 8;
            msb_time          = current_time;
            continue;
        }

        // 2-byte UART command received.
        is_msb_read        = false;
        dtm_cmd_from_uart |= (dtm_cmd_t)rx_byte;

        if(dtm_cmd_from_uart == 0x4352 /*"CR"*/)
        {
            cfg_board_reset();  //reset work
            continue;
        }
        else if(dtm_cmd_from_uart == 0x4346 /*"CF"*/)
        {
            cfg_nvm_factory_reset(true);  //factory work
            continue;
        }

        if (dtm_cmd_put(dtm_cmd_from_uart) != DTM_SUCCESS)
        {
            // Extended error handling may be put here.
            // Default behavior is to return the event on the UART (see below);
            // the event report will reflect any lack of success.
        }

        // Retrieve result of the operation. This implementation will busy-loop
        // for the duration of the byte transmissions on the UART.
        if (dtm_event_get(&result))
        {
            if(0)
            {
                ; //dummy if
            }
#ifdef FEATURE_CFG_RTT_MODULE_CONTROL
            else if(rtt_rd_size)  //rtt resp
            {
                char rtt_wr_bufffer[4];   //check BUFFER_SIZE_DOWN
                rtt_wr_bufffer[0] = (result >> 8) & 0xFF;
                rtt_wr_bufffer[1] = result & 0xFF;
                SEGGER_RTT_Write(0, rtt_wr_bufffer, 2);  //send result
            }
#endif
            else  //uart resp
            {
                // Report command status on the UART.
                // Transmit MSB of the result.
                while (app_uart_put((result >> 8) & 0xFF));
                // Transmit LSB of the result.
                while (app_uart_put(result & 0xFF));
            }
        }
    }
}

/// @}
