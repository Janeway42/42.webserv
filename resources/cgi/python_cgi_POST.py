#!/usr/bin/python
print("Start python script")

import time
import os   # TO ACCESS execve ENV variables
import sys	# to read from std input

import cgi, cgitb
# cgitb.enable()	# detailed errors msgs
# cgitb.enable(display=0, logdir="/path/to/logdir")



### To get at submitted form data, use the FieldStorage class.
# If the form contains non-ASCII characters, use the encoding keyword parameter set to the value of the encoding defined for the document (Content-Type header).
# FieldStorage class reads the form contents from the standard input OR the environment (depending on the value of various environment variables set according to the CGI standard). Since it may consume ??? standard input, it should be instantiated only once.


# WITHOUT SLEEP, NOTHING COMES YET TO THE FieldStoraeg() 
# AND IT THEREFORE GIVES PYTHON ERROR: [Errno 35] Resource temporarily unavailable
time.sleep(2)
# try:
form = cgi.FieldStorage()
form_data = ''.join([str(field.value) for field in form.list])
# except Exception as e:
#     # Handle the exception here
#     print("An error occurred: {}".format(e))
#     sys.stderr.write("Exception from python")
    

# if "street" not in form or "city" not in form:
# 	print("<H1>Error</H1>")
# 	print("Please fill in the street and city fields.")
# 	exit

# print("<p>street:", form["street"].value)
# print("<p>city:", form["city"].value)
# print("<p>fruit:", form["fruit"].value)
# print("<p>vegetable:", form["vegetable"].value)
# print(form)
print("Print form field fruit:")
# for fruit in form["fruit"]:
#     print(fruit)



sys.stderr.write('IN PYTHON SCRIPT, via stderr\n')
sys.stderr.write(form_data)
sys.stderr.write('\nIN PYTHON SCRIPT, via stderr\n')

print("End python script, to PIPEOUT")
