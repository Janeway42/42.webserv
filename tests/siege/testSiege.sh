#!/bin/bash

# COLORS
RED='\033[31m'
GRE='\033[32m'
GRY='\e[1;30m'
YEL='\033[33m'
BLU='\e[34m'
RES='\033[0m'

# siege -c [concurrent users] -t [time in seconds] -f [path to URL list]
# siege   -c 50 -t 1s -p 8080 -f ./pathList.txt
# siege   -c 50 -t 1s -p 8080 http://localhost:8080/index.html -m GET
siege   -c 5 -t 1s -p 8080 -m GET http://localhost:8080/index.html

sleep 0.2
curl -s -X GET http://localhost:8080/index.html
