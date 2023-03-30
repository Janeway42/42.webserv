RED='\033[31m'
GRE='\033[32m'
YEL='\033[33m'
RES='\033[0m'

cd ../
rm -rf tests/CurlOut

./webserv standard_complete.conf > tests/log.txt & 
sleep 1

cd ./tests
mkdir -p CurlOut

folderCurlOut="CurlOut"
folderCurlCompare="CurlCompare"


function testURLpath {
    testPath=$1
    testName=$2
    curl -i -s -X GET $testPath > $folderCurlOut/$testName &
    sleep 0.1
    if diff $folderCurlOut/$testName $folderCurlCompare/$testName >/dev/null ; then
        echo -en "${GRE}[OK]${RES}"
        printf "%-50s" " $testPath"
        printf "%-50s\n" "The files are identical"
    else
        echo -en "${RED}[OK]${RES}"
        printf "%-50s" " $testPath"
        printf "%-50s\n" "The files differ"
    fi
}

#               URL-PATH                             OUTPUT FILENAME
testURLpath "localhost:8080"                            "localhost:8080"
testURLpath "localhost:8080/index.html"                 "localhost:index.html"
testURLpath "localhost:8080/index_just_text.html"       "localhost:index_just_text.html"
testURLpath "localhost:8080/index_just_text_bible.html" "localhost:index_just_text_bible.html"
testURLpath "localhost:8080/cgi/cgi_index.html"         "localhost:cgi:cgi_index.html"

### images
testURLpath "localhost:8080/img_36kb.jpg"               "localhost:img_36kb.jpg"


pkill webserv