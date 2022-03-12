# NetworkQuality
It is a C implementation of draft 

## Build

To compile:
make

To run:
./client

## HTTP commands

GET /config HTTP/1.1
Host: monitor.uac.bj
Connection: Upgrade, HTTP2-Settings
Upgrade: h2c 
HTTP2-Settings: (SETTINGS payload)

HTTP1
-----

socat - OPENSSL:monitor.uac.bj:4449
GET /config HTTP/1.1
Host:monitor.uac.bj
valdez avec la touche "Entrer"

HTTP2
-----

socat - OPENSSL:monitor.uac.bj:4449
GET /config HTTP/1.1
Host: monitor.uac.bj
Connection: Upgrade, HTTP2-Settings
Upgrade: h2c 
HTTP2-Settings:
valdez avec la touche "Entrer"
