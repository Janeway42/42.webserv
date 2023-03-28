#!/usr/bin/python

import os   # TO ACCESS execve ENV variables
import sys	# to read from std input


# TO READ FROM STDIN, FROM PIPE
print ("<br><br><h3>THIS IS PYTHON SCRIPT:</h3>")

content_lenght = int(os.environ.get('CONTENT_LENGTH', 0))   ### is this int ok if length is size_t ??
print ("<p>BODY CONTENT_LENGTH: <span style='background-color:lavender; padding:1%;'>")
print (content_lenght)
print ("</span></p>")
print("")

# for line in sys.stdin:
# 	print ("Print python line [" + line + "]")
# 	print ("    len(line): ", len(line))
# 	# print (len(line))
# 	# if len(line) == content_lenght:
# 	# if len(line) == 22:
# 	# 	print ("YES THE SAME ------------")
# 	# 	print (line)
# 	# 	break
# 	# print ("]")

# sys.stdin.close()

# request_body = sys.stdin.read(content_lenght)

# print ("Print python request_body [")
# print (request_body)
# print ("]")




##################################################
# data = ""
# while True:
#     chunk = sys.stdin.read(20)
#     if not chunk:
#         break
#     data += chunk
##################################################





##################################################
# Set the buffer size to read from the pipe
# BUFFER_SIZE = 50
BUFFER_SIZE = 4096

data = ""

print ("Python while loop: ")
while True:
	# Read data from the pipe's reading end
	chunk = os.read(sys.stdin.fileno(), BUFFER_SIZE)
	
	# If there's no data, break out of the loop
	if not chunk:
		sys.stdin.close()
		break

	data += chunk
	print ("Print python line [[[" + chunk + "]]]")

	
	# Print the data to stdout
	# sys.stdout.write(data.decode('utf-8'))
	# sys.stdout.flush()
#sys.stdin.close()

##################################################

print ("End python")



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