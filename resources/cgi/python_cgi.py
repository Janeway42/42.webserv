#!/usr/bin/python

import os   # TO ACCESS execve ENV variables


# TO READ FROM STDIN, FROM PIPE


# OPEN FILE WITH INCOMING BODY CONTENT AND PRINT IT
fileWithBodyContent = "resources/cgi/dummy_file"
with open(fileWithBodyContent, "r", 0) as f:
    print ("THIS IS PRINTED FROM CGI PYTHON (the file with the stored incoming body)")
    for l in f:
        print (l)


# SEARCHING IF A VARIABLE 'QUERY_STRING' EXISTS IN THE ENV
# IF YES, STORING IT INTO 'URL'
for param in os.environ.keys():
    # print ("<b>%30s</b>: %s</br>") % (param, os.environ[param])
    #  print (param, os.environ[param])
    if param == 'QUERY_STRING':
        URL = os.environ[param]
        #print("YEEEEEEEEES")
        #print (URL)
    else:
        URL="DEFAULT=default"


# PARSING URL PATH AND STORING THE KEY:VALUE PAIRS
# from urllib.parse import urlparse, parse_qs       # why is this not good ??
from urlparse import urlparse, parse_qs
parsed_result = urlparse(URL)
parse_qs(parsed_result.query)


# HOW TO USE PIPES FROM LINUX TO PYTHON AND VICE VERSA ???


# PRINTING THE KEY:VALUE PAIRS FROM PYTHON SCRIPT
print ("\nTHIS IS PRINTED FROM CGI PYTHON (the ENV variables)")
print ("Parsed_result:")
print (parsed_result)

dict_result = parse_qs(parsed_result.path)
print ("\nDict_result:")
print (dict_result)

print ("<h3>   Congratulations!</h3>")
print ("<p>    Your city is:   " + dict_result['city'][0] + "</p>")
print ("<p>    Your street is: " + dict_result['street'][0] + "</p>")

########## ######## ############## ########### ###########