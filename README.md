# NetworkQuality
It is a C implementation of draft 

## Build

To compile:
make

To run:
./networkqualityC https://monitor.uac.bj:4449 small

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

https://nghttp2.org/documentation/tutorial-client.html#libevent-client-c

https://github.com/nghttp2/nghttp2/blob/master/examples/client.c


:method: GET

:path: /small

:scheme: https

:authority: monitor.uac.bj

accept: */*

accept-encoding: gzip, deflate

user-agent: nghttp2/1.43.0



https://github.com/gerardbos/h2clientserver

https://github.com/mwesenjak/apns-http2-client/blob/master/main.c


HHTP2 COMMANDS TEST
-------------------

ncat --ssl-alpn h1,h2 monitor.uac.bj 4449

openssl s_client -alpn h2 -connect monitor.uac.bj:4449
