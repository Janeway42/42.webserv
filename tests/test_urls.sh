

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
        echo "The files are identical"
    else
        echo "The files differ"
    fi
}

#               URL-PATH                             OUTPUT FILENAME
testURLpath "localhost:8080"                        "localhost:8080"
testURLpath "localhost:8080/index_just_text.html"   "localhost:index_just_text.html"
testURLpath "localhost:8080/index.html"             "localhost:index.html"