#!/usr/bin/python

import os   # TO ACCESS execve ENV variables
import sys	# to read from std input


# TO READ FROM STDIN, FROM PIPE
print ("<h4>THIS IS COMING FROM PYTHON SCRIPT:</h4>")


# for l in sys.stdin:
# 	print (l)




for param in os.environ.keys():
	# print ("<b>%30s</b>: %s</br>") % (param, os.environ[param])
	# print (param, os.environ[param])
	if param == 'QUERY_STRING':
		URL = os.environ[param]
		print("<p>YEEEEEEEEES<p>")
		print (URL)
	else:
		URL="DEFAULT=default"



# # from urllib.parse import urlparse, parse_qs # why is this not good ??
# from urlparse import urlparse, parse_qs
# parsed_result = urlparse(URL)
# parse_qs(parsed_result.query)
# # HOW TO USE PIPES FROM LINUX TO PYTHON AND VICE VERSA ???



# print ('''<p>THIS IS COMING FROM python_cgi.py''')
# print ("Parsed_result:")
# print (parsed_result)

# dict_result = parse_qs(parsed_result.path)
# print ("\nFOUND QUERY STRING:")
# print (dict_result)

# print ("<h3>Congratulations!</h3>\n ")
# print ("<p>      Your city is: " + dict_result['city'][0] + "</p>")
# print ("<p>    Your street is: " + dict_result['street'][0] + "</p>")

########## ######## ############## ########### ###########