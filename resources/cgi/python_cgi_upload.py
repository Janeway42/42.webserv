#!/usr/bin/python
import time
import sys	# to read from std input
import cgi, os
import cgitb
# cgitb.enable()

environ = os.environ.copy()

form = cgi.FieldStorage()
sys.stderr.write('FORM IS [' + str(form) + ']\n')

# Get filename here.
fileitem = form['filename']
# Test if the file was uploaded
if fileitem.filename:
   # strip leading path from file name to avoid
   # directory traversal attacks
   fn = os.path.basename(fileitem.filename)
   # open('/tmp/' + fn, 'wb').write(fileitem.file.read())
   open('/Users/jmurovec/Desktop/projects/webservCJJ_25mar01_newVarHasCGI/resources/uploads/' + fn, 'wb').write(fileitem.file.read())
   message = 'The file "' + fn + '" was uploaded successfully'
 
else:
   message = 'No file was uploaded'



# print ("""\
# Content-Type: text/html\n
# <html>
# <body>
#    <p>%s</p>
# </body>
# </html>
# """ % (message,))
time.sleep(1)
print("The file was uploaded successfully")



# FORM IS [
#    FieldStorage(None, None, [MiniFieldStorage(' name', '"filename"'), 
#                              MiniFieldStorage(' filename', '"sometext.txt"\r\n
#                               Content-Type: text/plain\r\n\r\n
#                               This is a file sometext.txt\n\r\n
#                               -----------------------------131863955734463801023167929879\r\n
#                               Content-Disposition: form-data'), 
#                              MiniFieldStorage(' name', '"submit"\r\n\r\n
#                               Upload\r\n
#                               -----------------------------131863955734463801023167929879--\r\n'),MiniFieldStorage(' name', '"filename"'),
#                              MiniFieldStorage(' filename', '"sometext.txt"\r\n
#                               Content-Type: text/plain\r\n\r\n
#                               This is a file sometext.txt\n\r\n
#                               -----------------------------131863955734463801023167929879\r\n
#                               Content-Disposition: form-data'),
#                              MiniFieldStorage(' name', '"submit"\r\n\r\n
#                               Upload\r\n
#                               -----------------------------131863955734463801023167929879--\r\n')])]