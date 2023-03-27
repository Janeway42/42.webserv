#!/usr/bin/python

import time
import os   # TO ACCESS execve ENV variables
import sys	# to read from std input

import cgi, cgitb
# cgitb.enable()	# detailed errors msgs
# cgitb.enable(display=0, logdir="/path/to/logdir")

print("<p>")
print(" ... The Start of python script ...")
sys.stderr.write('START PYTHON SCRIPT ( via stderr)\n')


### To get at submitted form data, use the FieldStorage class.
# If the form contains non-ASCII characters, use the encoding keyword parameter set to the value of the encoding defined for the document (Content-Type header).
# FieldStorage class reads the form contents from the standard input OR the environment (depending on the value of various environment variables set according to the CGI standard). Since it may consume ??? standard input, it should be instantiated only once.


# WITHOUT SLEEP, NOTHING COMES YET TO THE FieldStoraeg() 
# AND IT THEREFORE GIVES PYTHON ERROR: [Errno 35] Resource temporarily unavailable
# time.sleep(2)


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
sys.stderr.write('FORM IS [' + str(form) + ']\n')


# iterate over the fields in the form
for field_name in form.keys():
    field = form[field_name]
    sys.stderr.write('   Field is [' + str(field) + ']\n')
    print(field)
    

    # process the field data here







# from werkzeug.formparser import FileStorage
# # create a FileStorage object from stdin
# file_storage = FileStorage(sys.stdin.buffer)
# # read from the FileStorage object until the end of the data is reached
# while True:
#     data = file_storage.read(1024)
#     if not data:
#         break
# # do something with the data
# # close the FileStorage object
# file_storage.close()


# if "street" not in form or "city" not in form:
# 	print("<H1>Error</H1>")
# 	print("Please fill in the street and city fields.")
# 	exit

# print("<p>street:", form["street"].value)
# print("<p>city:", form["city"].value)
# print("<p>fruit:", form["fruit"].value)
# print("<p>vegetable:", form["vegetable"].value)
# print(form)
# print("Print form field fruit:")
# for fruit in form["fruit"]:
#     print(fruit)



sys.stderr.write('END PYTHON SCRIPT (via stderr)\n')
# sys.stderr.write(form_data)
print(" ... The End of Python Script ... ")
print("</p>")

