#!/usr/bin/python

import os   # TO ACCESS execve ENV variables
import sys	# to read from std input

import cgi, cgitb
# cgitb.enable()	# detailed errors msgs

# TO READ FROM STDIN, FROM PIPE
print ("<br><br><h3>THIS IS PYTHON SCRIPT:</h3>")



exit()


# for l in sys.stdin:
# 	print (l)




for param in os.environ.keys():
	# print ("<b>%30s</b>: %s</br>") % (param, os.environ[param])
	# print (param, os.environ[param])
	if param == 'REQUEST_METHOD':
		URL = os.environ[param]
		print("<p>FOUND METHOD:")
		print ("<span style='background-color:plum; padding:1%;'>" + URL + "</span><p>")
	if param == 'QUERY_STRING':
		URL = os.environ[param]
		print("<p>FOUND QUERY STRING:")
		print ("<span style='background-color:cornsilk; padding:1%;'>" + URL + "</span><p>")
	else:
		URL="DEFAULT=default"



# from urllib.parse import urlparse, parse_qs # why is this not good ??
from urlparse import urlparse, parse_qs
parsed_result = urlparse(URL)
parse_qs(parsed_result.query)


print ("<p>PARSED RESULT: <span style='background-color:lavender; padding:1%;'>")
print (parsed_result)
print ("</span></p>")


dict_result = parse_qs(parsed_result.path)
print ("<p>EXTRACTED QUERY STRING:")
print ("<span style='background-color:darkkhaki; padding:1%;'>")
print (dict_result)
print ("</span></p>")


print ("<div style='width:fit-content; padding:5%; margin: 10% 0% 0% 10%; border:1px solid; background-color: linen; border-radius: 4px';><h1>Congratulations!</h1>")
print ("<h3>      Your city is: <span style='background-color:MediumSeaGreen; padding:1%;'>" + dict_result['city'][0] + "</span></h3>")
print ("<h3>    Your street is: <span style='background-color:coral; padding:1%'>" + dict_result['street'][0] + "</h3>")

print("")






form = cgi.FieldStorage()
if "street" not in form or "city" not in form:
	print("<H1>Error</H1>")
	print("Please fill in the street and city fields.")
	# exit

print ("Python Form: ")
print (form)


########## ######## ############## ########### ###########