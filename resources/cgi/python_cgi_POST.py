#!/usr/bin/python
print("Start python script")

import os   # TO ACCESS execve ENV variables
import sys	# to read from std input

import cgi, cgitb
cgitb.enable()	# detailed errors msgs
# cgitb.enable(display=0, logdir="/path/to/logdir")



### To get at submitted form data, use the FieldStorage class.
# If the form contains non-ASCII characters, use the encoding keyword parameter set to the value of the encoding defined for the document (Content-Type header).
# FieldStorage class reads the form contents from the standard input OR the environment (depending on the value of various environment variables set according to the CGI standard). Since it may consume ??? standard input, it should be instantiated only once.


form = cgi.FieldStorage()
# if "street" not in form or "city" not in form:
# 	print("<H1>Error</H1>")
# 	print("Please fill in the street and city fields.")
# 	exit


# print("<p>street:", form["street"].value)
# print("<p>city:", form["city"].value)


print("End python script")
