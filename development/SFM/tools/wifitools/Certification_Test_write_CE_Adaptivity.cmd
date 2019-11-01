@echo off

set UART_PORT=COM64

esptool --port %UART_PORT% -b 460800 write_flash --flash_mode dout --flash_freq 80m --flash_size 1MB --verify 0x0000 WIFI_Certification_ESP_RF_test_tool_v2.0/bin/ESP8266_Adaptivity_v2.1_26m_20170726_1M.bin
if not "%ERRORLEVEL%" == "0" (
echo WIFI Write Error
pause
)
