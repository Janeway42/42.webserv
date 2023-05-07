#!/usr/bin/python

######################################## nginx
# import threading
# sys.modules['_dummy_thread'] = threading
# sys.stderr.write(str(sys.path) + '\n')
# sys.path.append('/Users/jmurovec/.brew/lib/python3.11/site-packages')
# from flup.server.fcgi import WSGIServer
# added jaka: nginx is complaining about unsopported version of fastcgi
######################################## nginx

import sys
import os   # TO ACCESS execve ENV variables
import time
import sys	# to read from std input
import cgi, cgitb
# cgitb.enable()	# detailed errors msgs

# TO READ FROM STDIN, FROM PIPE
print ("<br><br><h3>THIS IS PYTHON SCRIPT:</h3>")

# time.sleep(11000)
# exit()


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
    #else:
    #    URL="DEFAULT=default"



# from urllib.parse import urlparse, parse_qs # why is this not good ??
# from urlparse import urlparse, parse_qs        # in python2
from urllib.parse import urlparse, parse_qs     # in python3

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

if 'city' in dict_result:
    city = dict_result['city'][0]
else:
    city = "unknown"

if 'street' in dict_result:
    street = dict_result['street'][0]
else:
    street = "unknown"

print ("<div style='width:fit-content; padding:5%; margin: 10% 0% 0% 10%; border:1px solid; background-color: linen; border-radius: 4px';><h1>Congratulations!</h1>")
print ("<h3>      Your city is: <span style='background-color:MediumSeaGreen; padding:1%;'>" + city + "</span></h3>")
print ("<h3>    Your street is: <span style='background-color:coral; padding:1%'>" + street + "</h3>")

print("")

form = cgi.FieldStorage()
if "street" not in form or "city" not in form:
    print("<H1>Error</H1>")
    print("Please fill in the street and city fields.")
# exit

print ("Python Form: ")
print (form)
