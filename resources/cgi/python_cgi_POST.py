#!/usr/bin/python

import os   # TO ACCESS execve ENV variables
import sys	# to read from std input


# TO READ FROM STDIN, FROM PIPE
print ("<br><br><h3>THIS IS COMING FROM PYTHON SCRIPT:</h3>")

content_lenght = int(os.environ.get('CONTENT_LENGTH', 0))   ### is this int ok if length is size_t ??
print ("<p>BODY CONTENT_LENGTH: <span style='background-color:lavender; padding:1%;'>")
print (content_lenght)
print ("</span></p>")

request_body = sys.stdin.read(content_lenght)




# for l in sys.stdin:
# 	print (l)




# for param in os.environ.keys():
# 	# print ("<b>%30s</b>: %s</br>") % (param, os.environ[param])
# 	# print (param, os.environ[param])
# 	if param == 'REQUEST_METHOD':
# 		URL = os.environ[param]
# 		print("<p>FOUND METHOD:")
# 		print ("<span style='background-color:plum; padding:1%;'>" + URL + "</span><p>")
# 	if param == 'QUERY_STRING':
# 		URL = os.environ[param]
# 		print("<p>FOUND QUERY STRING:")
# 		print ("<span style='background-color:cornsilk; padding:1%;'>" + URL + "</span><p>")
# 	else:
# 		URL="DEFAULT=default"



# # from urllib.parse import urlparse, parse_qs # why is this not good ??
# from urlparse import urlparse, parse_qs
# parsed_result = urlparse(URL)
# parse_qs(parsed_result.query)


# print ("<p>PARSED RESULT: <span style='background-color:lavender; padding:1%;'>")
# print (parsed_result)
# print ("</span></p>")


# dict_result = parse_qs(parsed_result.path)
# print ("<p>EXTRACTED QUERY STRING:")
# print ("<span style='background-color:darkkhaki; padding:1%;'>")
# print (dict_result)
# print ("</span></p>")


# print ("<div style='width:fit-content; padding:5%; margin: 10% 0% 0% 10%; border:1px solid; background-color: linen; border-radius: 4px';><h1>Congratulations!</h1>")

### THESE ARE CAUSING ERROR WITH POST METHOD, BECAUSE THEY DON'T EXIST IN THE QUERY STRING
# print ("<h3>      Your fruit is: <span style='background-color:bisque; padding:1%;'>" + dict_result['fruit'][0] + "</span></h3>")
#print ("<h3>    Your vegetable is: <span style='background-color:coral; padding:1%'>" + dict_result['vegetable'][0] + "</h3>")

########## ######## ############## ########### ###########