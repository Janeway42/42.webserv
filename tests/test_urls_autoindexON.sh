#!/bin/bash

# COLORS
RED='\033[31m'
GRE='\033[32m'
GRY='\033[1;30m'
YEL='\033[33m'
BLU='\033[34m'
RES='\033[0m'


# OVERVIEW #################################################################
# This tester compares the nginx outoput with the webserver output.
# First, Nginx is run with the command: sudo   /usr/local/opt/nginx/bin/nginx -g "daemon off;"
#		(The command 'brew sevices start nginx' has not been tried yet)
# Then, each url is called with Curl, ie: curl -s -X GET localhost:$PORT/index.html ...
# The result is stored in a separate folder 'curlNginxOutput'
# The same is repeated with ./webserv and Curl, stored into 'curlWebservOutput'
# Then, files in both folders are compared.

# You need to change the paths in this file and in nginx config file, according
# to your local paths, ie:  PathMyWebServer="/Users/jmurovec/Desktop/projects/webserv03may00/"


# IMPORTANT PATHS #################################################################
# nginx path:       /usr/local/etc/nginx
# path nginx www:   /usr/local/var/www/
# path error log:   cat /usr/local/var/log/nginx/error.log > /...path to/error.log
# path access log:	/Users/jmurovec/Desktop/projects/webserv03may00/tests/access.log
# path brew nginx:  /usr/local/Cellar/nginx/1.23.4

### Command to run nginx in foreground:
#       sudo   /usr/local/opt/nginx/bin/nginx -g "daemon off;"


# TODO #################################################################
# Our server does not output the last empty line at the end of html file, after </body>
# Our server outputs 2 more characters '\r\n' than nginx, after the header


### SETTINGS #################################################################
PASSWORD="..."													# local sudo password, to run /usr/local/opt/nginx/bin/nginx -g "daemon off;" (this method is bad, but for now brew is not working)
# PathMyWebServer="/Users/jmb/Desktop/projects/webserv03may00"		# path to ./webserv executable  HOME
PathMyWebServer="/Users/jmurovec/Desktop/projects/webserv08may00/"		# path to ./webserv executable  CODAM
PathMyWebServerPublicFolder="$PathMyWebServer/resources/"			# path to html content of the webserv 
# PathNginxPublicFolder="/usr/local/var/www/resources/"					# the above folder will be copied to this nginx folder  HOME
PathNginxPublicFolder="/Users/jmurovec/.brew/var/www/resources/"		# the above folder will be copied to this nginx folder	CODAM
# PathNginx="/usr/local/etc/nginx/"									# nginx executable	HOME
PathNginx="/Users/jmurovec/.brew/etc/nginx/"						# nginx executable	CODAM
localhost="http://localhost:$PORT/"
curlNginxOutput="$PathMyWebServer/tests/curlNginxOutput/"
curlWebservOutput="$PathMyWebServer/tests/curlWebservOutput/"
PORT=8081
CONFIG_FILE=standard_complete_forTester.conf


# Copy the modified nginx config file into nginx, to override the original nginx.config
# The modified version has to have correct content, to be able to display webserver html pages
mv "$PathNginx/nginx.conf" "$PathNginx/nginx.conf.ORIG"
cp "$PathMyWebServer/tests/nginx.conf" "$PathNginx/nginx.conf"

# COPY THE WHOLE PUBLIC HTML FOLDER FROM WEBSERV TO NGINX
# BEFORE COPYING, DELETE PREVIOUS HTML FOLDER FROM NGINX, IF IT EXISTS
rm      -rf $PathNginxPublicFolder
mkdir   -p  $PathNginxPublicFolder
cp      -r  $PathMyWebServerPublicFolder/*   $PathNginxPublicFolder
# echo "web $PathMyWebServerPublicFolder"
# echo "nginx  $PathNginxPublicFolder"

# CREATE 2 FOLDERS FOR WEBSERV AND NGINX OUTPUT FILES
rm -rf $curlNginxOutput $curlWebservOutput
mkdir -p $curlNginxOutput $curlWebservOutput
# echo "nkdir name: " $curlNginxOutput



echo -e  "$GRE\n ~ ~ ~ WEBSERV - TEST, PORT:$PORT ~ ~ ~ ~ ~ ~ $RES " 1>&2
echo -e  "$GRE ~ ~ ~ BASIC TESTS: ${GRY}AUTOINDEX ON${GRE} ~ ~ ~ $RES \n" 1>&2



##################################################################################################################
### START NGINX ##################################################################################################

# echo $PASSWORD | sudo -S nginx -g "daemon off;" &
nginx -g "daemon off;" &
sleep 1		# needs some time to start running 


# Open all html pages with nginx and store the result into CurlNginxOutput
# The header is called separately with Curl. Only the first line is stored (ie: HTTP/1.1 200 OK)
function testURLpath {
	testURL=$1
	testName=$2
	# echo "name: " $curlNginxOutput/$testName
	# echo "URL to test by Nginx:" $testURL
	curl -i -s  -X GET $testURL | head -n 1 > $curlNginxOutput/$testName ;	sleep 0.1
	echo -e "\r" >> $curlNginxOutput/$testName
	curl -s -X GET $testURL >> $curlNginxOutput/$testName ; sleep 0.1
}

function testURLpath_justFirstHeaderLine {
	testURL=$1
	testName=$2
	curl -i -s  -X GET $testURL | head -n 1 > $curlNginxOutput/$testName ;	sleep 0.1
	echo -e "\r" >> $curlNginxOutput/$testName ;	sleep 0.1
}

### TEST URLs ############################################################################
# #               URL-PATH EXISTING                        OUTPUT FILENAME
testURLpath "localhost:$PORT"                            "localhost:$PORT"
testURLpath "localhost:$PORT/"                           "localhost:$PORT"
testURLpath "localhost:$PORT/index.html"                 "localhost:index.html"
testURLpath "localhost:$PORT/texts/index_texts.html"     "localhost:texts:index_texts.html"
testURLpath "localhost:$PORT/texts/one_sentence.html"    "localhost:texts:one_sentence.html"
testURLpath "localhost:$PORT/texts/one_page.html"        "localhost:texts:one_page.html"
testURLpath "localhost:$PORT/texts/bible.html"           "localhost:texts:bible.html"

# #               URL-PATH NOT EXISTING                  OUTPUT FILENAME
testURLpath_justFirstHeaderLine "localhost:$PORT/XXX.html"                 "localhost:XXX.html"
testURLpath_justFirstHeaderLine "localhost:$PORT/XXX/index_texts.html"     "localhost:XXX:index_texts.html"

# #               URL-PATH EXISTS< BUT NO index.html     OUTPUT FILENAME
testURLpath_justFirstHeaderLine "localhost:$PORT/_folderA"                 "localhost:_folderA"

# # # # IMAGES
# testURLpath "localhost:$PORT/images/index_images.html"   "localhost:images:index_images.html"
# # testURLpath "localhost:$PORT/images/img_36kb.jpg"       	"localhost:images:img_36kb.jpg"
# # # CGI
# testURLpath "localhost:$PORT/cgi/cgi_index.html"         "localhost:cgi:cgi_index.html"
# testURLpath 'localhost:$PORT/cgi/python_cgi_GET.py?street=Singel&city=Tokio'         "localhost:cgi:python_get.py?street=Singel&city=Tokio"
sleep 1
# cat /usr/local/var/log/nginx/error.log > ./error.log					# HOME
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
./webserv standard_complete_forTester.conf > log.txt & 
sleep 1

mkdir -p $curlWebservOutput


# wget --server-response --recursive --level=1 --no-parent --no-cookies  http://localhost:$PORT/images/index_images.html
printf "${BLU} - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -${RES}\n"
printf "${BLU}%s%-50s%-30s%-30s${RES}\n" "     " "Tested URL"  "Response Code"
function testURLpath {
	testURL=$1
	testName=$2
	responseCode=$(curl -i -s  -X GET $testURL | head -n 1)
	echo "$responseCode" > $curlWebservOutput/$testName
	responseCode=$(echo "$responseCode" | tr -d '\r\n')
	# echo "RESPONSECODE: " "$responseCode"
	#echo -e "/n" >> $testURL >> $curlWebservOutput/$testName
	curl -s -X GET $testURL >> $curlWebservOutput/$testName
	sleep 0.2
	# if diff $curlWebservOutput/$testName $curlNginxOutput/$testName >/dev/null ; then
	if diff <(head -n 1 $curlWebservOutput/$testName) <(head -n 1 $curlNginxOutput/$testName) >/dev/null ; then
		echo -en "${GRE}[OK] ${RES}"
		printf "${GRY}%-50s${RES}" "$testURL"
		printf "%-20s" "$responseCode"
		printf "${GRE}%-30s${RES}\n" "Responses are identical"
	else
		echo -en "${RED}[KO] ${RES}"
		printf "${GRY}%-50s${RES}" "$testURL"
		printf "%-20s" "$responseCode"
		printf "${RED}%-30s ${RES}\n" "Responses different"
	fi
}


function testURLpath_justFirstHeaderLine {
	testURL=$1
	testName=$2
	responseCode=$(curl -i -s  -X GET $testURL | head -n 1)
	echo "$responseCode" > $curlWebservOutput/$testName
	responseCode=$(echo "$responseCode" | tr -d '\r\n')
	# echo "RESPONSECODE: " "$responseCode"
	#echo -e "/n" >> $testURL >> $curlWebservOutput/$testName
	sleep 0.2
	# if diff $curlWebservOutput/$testName $curlNginxOutput/$testName >/dev/null ; then
	if diff <(head -n 1 $curlWebservOutput/$testName) <(head -n 1 $curlNginxOutput/$testName) >/dev/null ; then

		echo -en "${GRE}[OK] ${RES}"
		printf "${GRY}%-50s${RES}" "$testURL"
		printf "%-20s" "$responseCode"
		printf "${GRE}%-30s${RES}\n" "Responses are identical"
	else
		echo -en "${RED}[KO] ${RES}"
		printf "${GRY}%-50s${RES}" "$testURL"
		printf "%-20s" "$responseCode"
		printf "${RED}%-30s ${RES}\n" "Responses different"
	fi
}


### TEST URLs ############################################################################
# #               URL-PATH EXISTING                        OUTPUT FILENAME
echo -en "${BLU}\nPATH IS EXISTING:\n${RES}"

testURLpath "localhost:$PORT"                            "localhost:$PORT"
testURLpath "localhost:$PORT/"                           "localhost:$PORT"
testURLpath "localhost:$PORT/index.html"                 "localhost:index.html"
testURLpath "localhost:$PORT/texts/index_texts.html"     "localhost:texts:index_texts.html"
testURLpath "localhost:$PORT/texts/one_sentence.html"    "localhost:texts:one_sentence.html"
testURLpath "localhost:$PORT/texts/one_page.html"        "localhost:texts:one_page.html"
testURLpath "localhost:$PORT/texts/bible.html"           "localhost:texts:bible.html"

# #               URL-PATH NOT EXISTING                  OUTPUT FILENAME
echo -en "${BLU}\nPATH NOT EXISTING:\n${RES}"
testURLpath_justFirstHeaderLine "localhost:$PORT/XXX.html"                 "localhost:XXX.html"
testURLpath_justFirstHeaderLine "localhost:$PORT/XXX/index_texts.html"     "localhost:XXX:index_texts.html"

# #               URL-PATH EXISTS< BUT NO index.html     OUTPUT FILENAME
echo -en "${BLU}\nPATH NOT EXISTING:\n${RES}"
testURLpath_justFirstHeaderLine "localhost:$PORT/_folderA"                 "localhost:_folderA"

# # # # IMAGES
# testURLpath "localhost:$PORT/images/index_images.html"   "localhost:images:index_images.html"
# # testURLpath "localhost:$PORT/images/img_36kb.jpg"       	"localhost:images:img_36kb.jpg"
# # # CGI
# testURLpath "localhost:$PORT/cgi/cgi_index.html"         "localhost:cgi:cgi_index.html"
# testURLpath "localhost:$PORT/cgi/python_cgi_GET.py?street=Singel&city=Tokio"         "localhost:cgi:python_get.py?street=Singel&city=Tokio"

# diff $curlWebservOutput/$testName $curlNginxOutput/$testName

# cat -e $curlNginxOutput/$testName
# echo "------------------------------------------------------"
# cat -e $curlWebservOutput/$testName

pkill -f webserv

# sleep 1

# Start Siege Test

# cd ./tests/siege
# bash testSiege.sh


