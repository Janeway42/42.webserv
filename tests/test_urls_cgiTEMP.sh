#!/bin/bash

# COLORS
RED='\033[31m'
GRE='\033[32m'
GRY='\033[1;30m'
YEL='\033[33m'
BLU='\033[34m'
RES='\033[0m'


# curlWebservOutput="$PathMyWebServer/tests/curlWebservOutput/"


# # CGI UPLOAD
# curl -i -F 'filename=@/Users/jmurovec/Desktop/text_small.txt' http://localhost:8080/cgi/python_cgi_POST_upload.py	# -i prints the response header
curl  -s  -F 'filename=@/Users/jmurovec/Desktop/text_small.txt' http://localhost:8080/cgi/python_cgi_POST_upload.py > ./curlWebservOutput/cgi_POST_upload ;	sleep 0.1

