#!/usr/bin/python
import time
import sys	# to read from std input
import cgi, os
import cgitb
# cgitb.enable()

environ = os.environ.copy()



form = cgi.FieldStorage()
#sys.stderr.write('FORM IS [' + str(form) + ']\n')

# Get filename here.
fileitem = form['filename']
#print('fileitem.file:', fileitem.file)
# Test if the file was uploaded
if fileitem.filename:
   # strip leading path from file name to avoid
   # directory traversal attacks
   fn = os.path.basename(fileitem.filename)
   try:
       with open('/Users/jmurovec/Desktop/projects/webservCJJ_28mar00/resources/uploads/' + fn, 'wb') as f:
           f.write(fileitem.file.read())
           message = 'The file ' + fn + ' was uploaded successfully'
   except IOError as e:
       message = 'Error uploading file: ' + str(e)
       sys.stderr.write('Error uploading file: ' + str(e))


# with open('/Users/jmurovec/Desktop/projects/webservCJJ_25mar01_newVarHasCGI/resources/uploads/' + fn, 'rb') as f:
#     file_data = f.read()
# print('file_data:', file_data)

# print(os.getcwd())
# print(os.listdir('/Users/jmurovec/Desktop/projects/webservCJJ_25mar01_newVarHasCGI/resources/uploads/'))


# form = _cgi.FieldStorage()
# sys.stderr.write('FORM IS [' + str(form) + ']\n')
# # Get filename here.
# fileitem = form['filename']
# # Test if the file was uploaded
# if fileitem.filename:
#    # strip leading path from file name to avoid
#    # directory traversal attacks
#    fn = os.path.basename(fileitem.filename)
#    # open('/tmp/' + fn, 'wb').write(fileitem.file.read())
#    open('/Users/jmurovec/Desktop/projects/webservCJJ_25mar01_newVarHasCGI/resources/uploads/' + fn, 'wb').write(fileitem.file.read())
#    message = 'The file ' + fn + ' was uploaded successfully'
# else:
#    message = 'No file was uploaded'






# print ("""\
# Content-Type: text/html\n
# <html>
# <body>
#    <p>%s</p>
# </body>
# </html>
# """ % (message,))
# time.sleep(1)
print(message)


