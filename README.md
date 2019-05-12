# ESP8266 Alexa / OTA update for WiFi Dimmer 

This provides a additional firmware for an additional ESP8266 (this does *not* replace the existing firmare)  for this marvelous product https://ex-store.de/2-Kanal-RS232-WiFi-WLan-Dimmer-Modul-V4-fuer-Unterputzmontage-230V-3A

# Required Libaries

Te following libaries are needed:

- https://bitbucket.org/xoseperez/fauxmoesp (use the dev branch)
- https://github.com/me-no-dev/ESPAsyncTCP
- https://github.com/Links2004/arduinoWebSockets

# OTA Update

From the command line `curl -F "image=@/path/to/firmware.bin" http://192.168.0.123:8080/update`.

Via Webbrowser ` http://192.168.0.123:8080/update`.

# Versions UDP / Websockets

While UDP is simpler and the resulting firmware is smaller, the Websocket approach works much better for me. Probably because i did something wrong with the UDP implementation.


