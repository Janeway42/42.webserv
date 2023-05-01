#!/usr/bin/python

import cgi
import cgitb
import time
import sys  # to read from std input
import os  # to access execve ENV variables
# cgitb.enable()	# detailed errors msgs
# cgitb.enable(display=0, logdir="/path/to/logdir")

print("<p> ... This is Python script for POST method ...</p>")
sys.stderr.write('START PYTHON SCRIPT ( via stderr)\n')

# To get at submitted form data, use the FieldStorage class.
# If the form contains non-ASCII characters, use the encoding keyword parameter set to the value of the encoding
# defined for the document (Content-Type header).
# FieldStorage class reads the form contents from the standard input OR the environment (depending on the value of
# various environment variables set according to the CGI standard).
# Since it may consume ??? standard input, it should be instantiated only once.

class StdinStream(object):
    def __init__(self, bufsize=8192):
        self.bufsize = bufsize

    def read(self, size=-1):
        if size == -1:
            # read until EOF
            return sys.stdin.read()
        else:
            # read in chunks until size bytes are read or EOF is reached
            chunks = []
            remaining = size
            while remaining > 0:
                chunk = sys.stdin.read(min(self.bufsize, remaining))
                if not chunk:
                    break
                chunks.append(chunk)
                remaining -= len(chunk)
            return ''.join(chunks)


form = cgi.FieldStorage(fp=StdinStream(), environ={'REQUEST_METHOD': 'POST'})
# print(form)
# sys.stderr.write('FORM IS [' + str(form) + ']\n')

# iterate over the fields in the form
# for field_name in form.keys():
#     field = form[field_name]
#     sys.stderr.write('   Field is [' + str(field) + ']\n')
#     print("<p>")
#     print(field)
#     print("</p>")

print("<div style='background-color:lavender; padding:1%; margin: 5% 0% 0% 5%; width:30%'>")

if "name" in form:
    storedValue = form["name"].value
    print("<h1>")
    print("Hello ")
    print(storedValue)
    print(",</h1>")

if "description" in form:
    storedValue = form["description"].value
    wordList = storedValue.split()
    wordCount = len(wordList)
    print("<h2>")
    print("Your text has")
    print(wordCount)
    print("words")
    print("</h2>")

print("</div>")
