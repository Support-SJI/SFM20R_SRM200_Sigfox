@echo off

if not "%1"=="" (
    set UART_PORT=%1
    set PAUSE_TOOLS=
) else (
    set UART_PORT=COM5
    set PAUSE_TOOLS=pause
)

esptool --port %UART_PORT% -b 460800 write_flash --flash_mode dout --flash_freq 80m --flash_size 1MB --verify 0xfc000 bin_spi_at/esp_init_data_default_SRM200.bin 0xfe000 bin_spi_at/blank_multiboot_0x81000.bin 0x0000 bin_spi_at/boot_v1.6.bin 0x1000 bin_spi_at/user1.1024.new.2_20190820.bin 0x81000 bin_spi_at/ESP8266_SDK_INIT_PARAM_26M_20170421.bin
if not "%ERRORLEVEL%" == "0" (
echo WIFI Write Error
%PAUSE_TOOLS%
) else (
echo SRM WIFI OK
)