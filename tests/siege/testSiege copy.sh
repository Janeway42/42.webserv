#!/bin/bash

# COLORS
RED='\033[31m'
GRE='\033[32m'
GRY='\e[1;30m'
YEL='\033[33m'
BLU='\e[34m'
RES='\033[0m'

outputOut="./outputFiles/stdOut.txt"
outputErr="./outputFiles/stdErr.txt"

rm $outputOut
rm $outputErr

# Run WebServ
#$pathToWebserv >> "./webservOutput.txt"

cd ../../ ;  ./webserv
cd tests/siege/


# siege -c [concurrent users] -t [time in seconds] -f [path to URL list]
# siege   -c 50 -t 1s -p 8080 -f ./pathList.txt
# siege   -c 50 -t 1s -p 8080 http://localhost:8080/index.html -m GET

# siege -b -c 1 -t 1s -p 8080 http://localhost:8080 >> $outputOut 2>> $outputErr
# siege -b -c 1 -t 1s -p 8080 http://localhost:8080/ >> $outputOut 2>> $outputErr
siege -c 1 -t 1s -p 8080 http://localhost:8080/index.html # >> $outputOut 2>> $outputErr

# sleep 0.2
# curl -s -X GET http://localhost:8080/index.html
pkill -f webserv
printf "$GRE TEST SIEGE FINISHED, CHECK THE OUTPUT RESULTS IN 'siege/outputFiles/stdOut.txt'  $RES"