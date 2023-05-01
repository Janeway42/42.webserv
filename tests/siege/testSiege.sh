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
TIME="-t1s"

rm $outputOut
rm $outputErr

echo -e  "$GRE\n ~ ~ ~ TEST SIEGE ~ ~ ~ $RES \n" 1>&2
printf "$GRE Start the Webserv  $RES \n" 1>&2

# Run WebServ
rm -f ./tests/siege/outputFiles/webservOutput.txt
cd ../../ ;  ./webserv >> ./tests/siege/outputFiles/webservOutput.txt & 
# cd ../../ ;  ./webserv >> /dev/null & 
cd tests/siege/
sleep 2     # Wait that the Webserver starts


# !!! It does not work with -p flag: -p 8080 

printf "$GRE Testing Siege: $GRY http://localhost:$portNr/index.html $RES \n" 1>&2

echo -e "$BLU   TEST: 1 user, $TIME seconds $RES" 1>&2
siege -b -c1 $TIME http://localhost:$portNr/index.html >> $outputOut 2>> $outputErr

echo -e "$BLU   TEST: 20 users, $TIME seconds $RES" 1>&2
siege -b -c20 $TIME http://localhost:$portNr/index.html >> $outputOut 2>> $outputErr

echo -e "$BLU   TEST: 50 users, $TIME seconds $RES" 1>&2
siege -b -c50 $TIME http://localhost:$portNr/index.html >> $outputOut 2>> $outputErr

echo -e "$BLU   TEST: 100 users, $TIME seconds $RES" 1>&2
siege -b -c100 $TIME http://localhost:$portNr/index.html >> $outputOut 2>> $outputErr

echo -e "$BLU   TEST: 150 users, $TIME seconds $RES" 1>&2
siege -b -c150 $TIME http://localhost:$portNr/index.html >> $outputOut 2>> $outputErr

echo -e "$BLU   TEST: 200 users, $TIME seconds $RES" 1>&2
siege -b -c200 $TIME http://localhost:$portNr/index.html >> $outputOut 2>> $outputErr

echo -e "$BLU   TEST: 250 users, $TIME seconds $RES" 1>&2
siege -b -c200 $TIME http://localhost:$portNr/index.html >> $outputOut 2>> $outputErr

pkill -f webserv
cat ./outputFiles/stdErr.txt | grep Availability 1>&2
rm -f ./outputFiles/webservOutput.txt

echo -e "\n$GRE TEST SIEGE FINISHED, CHECK THE OUTPUT RESULTS IN 'tests/siege/outputFiles/stdOut.txt' $RES\n" 1>&2