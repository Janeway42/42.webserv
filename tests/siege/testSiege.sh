#!/bin/bash

# COLORS
RED='\033[31m'
GRE='\033[32m'
GRY='\033[1;30m'
YEL='\033[33m'
BLU='\033[34m'
RES='\033[0m'

outputOut="./outputFiles/stdOut.txt"
outputErr="./outputFiles/stdErr.txt"
portNr="8080"
TIME="$TIME"

rm $outputOut
rm $outputErr

echo -e  "$GRE\n ~ ~ ~ TEST SIEGE ~ ~ ~ $RES \n" 1>&2
printf "$GRE Start the Webserv  $RES \n" 1>&2

# Run WebServ
cd ../../ ;  ./webserv >> ./tests/siege/outputFiles/webservOutput.txt & 
cd tests/siege/
sleep 2     # Wait that the Webserver starts


# !!! It does not work with -p flag: -p 8080 

printf "$GRE Testing Siege: $GRY http://localhost:$portNr/index.html $RES \n" 1>&2

echo -e "$BLU   TEST: 1 user $RES" 1>&2
siege -c1 $TIME http://localhost:$portNr/index.html >> $outputOut 2>> $outputErr

echo -e "$BLU   TEST: 20 users $RES" 1>&2
siege -c20 $TIME http://localhost:$portNr/index.html >> $outputOut 2>> $outputErr

echo -e "$BLU   TEST: 50 users $RES" 1>&2
siege -c50 $TIME http://localhost:$portNr/index.html >> $outputOut 2>> $outputErr

echo -e "$BLU   TEST: 100 users $RES" 1>&2
siege -c100 $TIME http://localhost:$portNr/index.html >> $outputOut 2>> $outputErr

echo -e "$BLU   TEST: 200 users $RES" 1>&2
siege -c200 $TIME http://localhost:$portNr/index.html >> $outputOut 2>> $outputErr

echo -e "$BLU   TEST: 250 users $RES" 1>&2
siege -c200 $TIME http://localhost:$portNr/index.html >> $outputOut 2>> $outputErr

pkill -f webserv
cat ./outputFiles/stdErr.txt | grep Availability 1>&2
echo -e "\n$GRE TEST SIEGE FINISHED, CHECK THE OUTPUT RESULTS IN 'tests/siege/outputFiles/stdOut.txt' $RES\n" 1>&2