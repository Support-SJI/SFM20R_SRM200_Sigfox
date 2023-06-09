
/* This file was automatically generated by nrfutil on 2019-02-14 (YY-MM-DD) at 14:06:19 */

#include "sdk_config.h"
#include "stdint.h"
#include "compiler_abstraction.h"
#ifdef FEATURE_WISOL_DEVICE
#include "cfg_config_defines.h"
#endif

#if NRF_CRYPTO_BACKEND_OBERON_ENABLED
/* Oberon backend is changing endianness thus public key must be kept in RAM. */
#define _PK_CONST
#else
#define _PK_CONST const
#endif


/* This file was generated with a throwaway private key, that is only inteded for a debug version of the DFU project.
  Please see https://github.com/NordicSemiconductor/pc-nrfutil/blob/master/README.md to generate a valid public key. */

#if defined(FEATURE_WISOL_DEVICE) && defined(FEATURE_WISOL_BOOTLOADER)

#if (CDEV_BOARD_TYPE == CDEV_BOARD_EVB)
//key file : EVB_pk.txt(gen by keys/EVB_private.pem)
/** @brief Public key used to verify DFU images */
__ALIGN(4) const uint8_t pk[64] =
{
    0x7c, 0x57, 0x2c, 0x85, 0xbf, 0xf6, 0x5e, 0x81, 0x93, 0x37, 0x46, 0x81, 0x67, 0x70, 0x94, 0x50, 0x7a, 0x9f, 0x51, 0xd5, 0x1a, 0x62, 0xbb, 0xf5, 0x54, 0x41, 0x22, 0x61, 0x62, 0x3b, 0x47, 0x47, 
    0xba, 0xfd, 0x75, 0x7b, 0x33, 0x48, 0x38, 0xd2, 0x5b, 0xc9, 0x10, 0x74, 0x11, 0x0f, 0x28, 0x87, 0x55, 0x30, 0xfd, 0x05, 0x12, 0xc5, 0x8d, 0x78, 0x32, 0xf9, 0x72, 0x92, 0xc0, 0xd4, 0xd5, 0xed
};

#elif (CDEV_BOARD_TYPE == CDEV_BOARD_IHERE)
//key file : ihere_pk.txt(gen by keys/ihere_private.pem)
/** @brief Public key used to verify DFU images */
__ALIGN(4) const uint8_t pk[64] =
{
    0xc2, 0x77, 0x90, 0x6b, 0xcc, 0x41, 0xfd, 0x57, 0xb1, 0x21, 0x64, 0x5b, 0x39, 0xc8, 0x15, 0xcb, 0x8f, 0x4a, 0x78, 0x7b, 0x11, 0xd4, 0x00, 0x27, 0x94, 0xa0, 0x13, 0x22, 0xbf, 0x12, 0x64, 0x08, 
    0x7a, 0x0f, 0xc1, 0xa1, 0xa4, 0x28, 0x4f, 0xea, 0xb0, 0x8d, 0xa5, 0xa1, 0xc0, 0x13, 0x82, 0xe2, 0x70, 0x76, 0x85, 0x1c, 0x1b, 0x97, 0x60, 0xbd, 0xc1, 0xc2, 0x66, 0xc4, 0xf8, 0x07, 0x30, 0xdb
};
#else
//key file : Default_pk.txt(gen by keys/Default_private.pem)
/** @brief Public key used to verify DFU images */
__ALIGN(4) const uint8_t pk[64] =
{
    0x7c, 0x57, 0x2c, 0x85, 0xbf, 0xf6, 0x5e, 0x81, 0x93, 0x37, 0x46, 0x81, 0x67, 0x70, 0x94, 0x50, 0x7a, 0x9f, 0x51, 0xd5, 0x1a, 0x62, 0xbb, 0xf5, 0x54, 0x41, 0x22, 0x61, 0x62, 0x3b, 0x47, 0x47, 
    0xba, 0xfd, 0x75, 0x7b, 0x33, 0x48, 0x38, 0xd2, 0x5b, 0xc9, 0x10, 0x74, 0x11, 0x0f, 0x28, 0x87, 0x55, 0x30, 0xfd, 0x05, 0x12, 0xc5, 0x8d, 0x78, 0x32, 0xf9, 0x72, 0x92, 0xc0, 0xd4, 0xd5, 0xed
};
#endif

#else
#ifdef NRF_DFU_DEBUG_VERSION 

/** @brief Public key used to verify DFU images */
__ALIGN(4) _PK_CONST uint8_t pk[64] =
{
    0x0e, 0xdf, 0x9f, 0xc8, 0xe6, 0x7b, 0x4e, 0x1a, 0xce, 0xe8, 0x6f, 0x86, 0xe2, 0x91, 0xc2, 0x98, 0x42, 0xc0, 0x23, 0x72, 0xfc, 0x76, 0xe6, 0xd2, 0xf7, 0x74, 0xfc, 0x09, 0x9f, 0xbf, 0x88, 0x5a, 
    0x79, 0x78, 0xd4, 0x2f, 0xcb, 0x27, 0x84, 0xb1, 0xd1, 0x82, 0x70, 0xf9, 0xe0, 0x14, 0xf9, 0x49, 0xa4, 0x47, 0xe9, 0x33, 0xaf, 0x7d, 0x07, 0x1d, 0xd0, 0x93, 0x4b, 0xd2, 0x93, 0x8b, 0xe6, 0x20
};

#else
#error "Debug public key not valid for production. Please see https://github.com/NordicSemiconductor/pc-nrfutil/blob/master/README.md to generate it"
#endif
#endif
