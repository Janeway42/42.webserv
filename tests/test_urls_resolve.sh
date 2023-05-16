#!/bin/bash

# COLORS
RED='\033[31m'
GRE='\033[32m'
GRY='\033[1;30m'
YEL='\033[33m'
BLU='\033[34m'
RES='\033[0m'





### SETTINGS #################################################################
PASSWORD="..."													# local sudo password, to run /usr/local/opt/nginx/bin/nginx -g "daemon off;" (this method is bad, but for now brew is not working)
# PathMyWebServer="/Users/jmb/Desktop/projects/webserv03may00"		# path to ./webserv executable  HOME
PathMyWebServer="/Users/jmurovec/Desktop/projects/webserv08may00/"		# path to ./webserv executable  CODAM
PathMyWebServerPublicFolder="$PathMyWebServer/resources/"			# path to html content of the webserv 
PathMyWebServerPublicFolder2="$PathMyWebServer/resources4243/"			# path to html content of the webserv 
# PathNginxPublicFolder="/usr/local/var/www/resources/"					# the above folder will be copied to this nginx folder  HOME
PathNginxPublicFolder="/Users/jmurovec/.brew/var/www/resources/"		# the above folder will be copied to this nginx folder	CODAM
PathNginxPublicFolder2="/Users/jmurovec/.brew/var/www/resources4243/"		# the above folder will be copied to this nginx folder	CODAM
# PathNginx="/usr/local/etc/nginx/"									# nginx executable	HOME
PathNginx="/Users/jmurovec/.brew/etc/nginx/"						# nginx executable	CODAM
PORT=8080
localhost="http://localhost:$PORT/"
curlNginxOutput="$PathMyWebServer/tests/curlNginxOutput/"
curlWebservOutput="$PathMyWebServer/tests/curlWebservOutput/"
CONFIG_FILE=standard_complete_forTester.conf


# Copy the modified nginx config file into nginx, to override the original nginx.config
# The modified version has to have correct content, to be able to display webserver html pages
mv "$PathNginx/nginx.conf" "$PathNginx/nginx.conf.ORIG"
cp "$PathMyWebServer/tests/nginx.conf" "$PathNginx/nginx.conf"

# COPY THE WHOLE PUBLIC HTML FOLDER FROM WEBSERV TO NGINX
# BEFORE COPYING, DELETE PREVIOUS HTML FOLDER FROM NGINX, IF IT EXISTS
rm      -rf $PathNginxPublicFolder
mkdir   -p  $PathNginxPublicFolder $PathNginxPublicFolder2
cp      -r  $PathMyWebServerPublicFolder/*   $PathNginxPublicFolder
cp      -r  $PathMyWebServerPublicFolder2/*   $PathNginxPublicFolder2
# echo "web $PathMyWebServerPublicFolder"
# echo "nginx  $PathNginxPublicFolder"

# CREATE 2 FOLDERS FOR WEBSERV AND NGINX OUTPUT FILES
rm -rf $curlNginxOutput $curlWebservOutput
mkdir -p $curlNginxOutput $curlWebservOutput
# echo "nkdir name: " $curlNginxOutput

echo -e  "$GRE\n ~ ~ ~ WEBSERV - TEST, PORT:$PORT ~ ~ ~ ~ ~ ~ $RES " 1>&2
echo -e  "$GRE ~ ~ ~ TESTS: ${YEL}CURL --RESOLVE${GRE} ~ ~ ~ $RES \n" 1>&2
echo -e  "$BLU ~ ~ ~ ( same port, different server name, etc ... ) ~ ~ ~ $RES \n" 1>&2



##################################################################################################################
### START NGINX ##################################################################################################

# echo $PASSWORD | sudo -S nginx -g "daemon off;" &
nginx -g "daemon off;" &
sleep 1		# needs some time to start running 



# Open all html pages with nginx and store the result into CurlNginxOutput
# The header is called separately with Curl. Only the first line is stored (ie: HTTP/1.1 200 OK)
function testURLpath {
	arg1=$1
    arg2=$2
	testName=$3
	# echo "name: " $curlNginxOutput/$testName
	# echo "arg1: $arg1, arg2: $arg2, name: $arg3"
	curl -i -s  --resolve $arg1 $arg2 | head -n 1 > $curlNginxOutput/$testName
	sleep 0.1
	echo -e "\r" >> $curlNginxOutput/$testName
	curl -s  --resolve $arg1 $arg2 >> $curlNginxOutput/$testName
	sleep 0.1
}

# function testURLpath_justFirstHeaderLine {
# 	arg1=$1
#     arg2=$2
# 	testName=$3
# 	curl -s -L -i --resolve $arg1 $arg2 | head -n 1 > $curlNginxOutput/$testName ;	sleep 0.1
# 	echo -e "\r" >> $curlNginxOutput/$testName ;	sleep 0.1
# }

testURLpath "localhost:8080:127.0.0.1"      "http://localhost:8080"       "localhost:8080"
testURLpath "server4243.com:8080:127.0.0.1" "http://server4243.com:8080"  "server4243.com:8080"


sleep 1
cat /Users/jmurovec/.brew/Cellar/nginx/logs/error.log > ./error.log		# CODAM

# STOP NGINX
nginx -s stop
# sudo nginx -s stop
# echo "EXIT tests"
# exit



##################################################################################################################
### RUN WEBSERV ##################################################################################################
sleep 0.5
# exit

# Remove any old Curl Output
rm -rf $curlWebservOutput


cd ../
./webserv $CONFIG_FILE > log.txt & 
sleep 1

mkdir -p $curlWebservOutput

printf "${BLU} - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -${RES}\n"
printf "${BLU}%s%-50s%-30s%-30s${RES}\n" "     " "Tested URL"  "Response Code"
function testURLpath {
	arg1=$1
	arg2=$2
	testName=$3
	responseCode=$(curl -i -s  --resolve $arg1 $arg2 | head -n 1)
	echo "$responseCode" > $curlWebservOutput/$testName
	responseCode=$(echo "$responseCode" | tr -d '\r\n')
	# echo "RESPONSECODE: " "$responseCode"
	#echo -e "/n" >> $testURL >> $curlWebservOutput/$testName
	curl -s --resolve $arg1 $arg2 >> $curlWebservOutput/$testName
	sleep 0.2
	if diff $curlWebservOutput/$testName $curlNginxOutput/$testName >/dev/null ; then
		echo -en "${GRE}[OK] ${RES}"
		printf "${GRY}%-50s${RES}" "$arg1"
		printf "%-20s" "$responseCode"
		printf "${GRE}%-30s${RES}\n" "Responses are identical"
	else
		echo -en "${RED}[KO] ${RES}"
		printf "${GRY}%-50s${RES}" "$arg1"
		printf "%-20s" "$responseCode"
		printf "${RED}%-30s ${RES}\n" "Responses different"
	fi
}

testURLpath "localhost:8080:127.0.0.1"      "http://localhost:8080"       "localhost:8080"
testURLpath "server4243.com:8080:127.0.0.1" "http://server4243.com:8080"  "server4243.com:8080"







#####################################################################################################################################
# # CGI UPLOAD
# curl -i -F 'filename=@/Users/jmurovec/Desktop/text_small.txt' http://localhost:8080/cgi/python_cgi_POST_upload.py	# -i prints the response header
# curl  -s  -F 'filename=@/Users/jmurovec/Desktop/text_small.txt' http://localhost:8080/cgi/python_cgi_POST_upload.py > ./curlWebservOutput/cgi_POST_upload ;	sleep 0.1

