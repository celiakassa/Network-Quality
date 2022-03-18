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

:method: GET
:scheme: https
:host: monitor.uac.bj
:path: /small

https://hpbn.co/http2/

https://sookocheff.com/post/networking/how-does-http-2-work/


:method: GET
:path: /
:scheme: https
:authority: monitor.uac.bj
accept: */*
accept-encoding: gzip, deflate
user-agent: nghttp2/1.43.0


https://github.com/gerardbos/h2clientserver

https://github.com/mwesenjak/apns-http2-client/blob/master/main.c

