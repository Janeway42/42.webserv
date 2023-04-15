#!/usr/bin/python
import time
import cgitb
import cgi
import os
import sys  # to read from std input

# cgitb.enable()

form = cgi.FieldStorage()

# Get filename here.
# fileitem = form['filename']
# print('fileitem.file:', fileitem.file)

# Define the path to the folder you want to list
rootFolder = os.getcwd()
pathToUploads = rootFolder + '/resources/uploads/'# TODO this path has to come from the config file??? (look pdF)

for param in os.environ.keys():
    print("<b>%30s</b>: %s</br>") % (param, os.environ[param])
    # sys.stderr.write(param + ':  ')
    # sys.stderr.write(os.environ[param] + '\n')
    if param == 'REQUEST_METHOD':
        URL = os.environ[param]
        # print(URL)
    # if URL == "delete":
    #     print("<p>FOUND METHOD: " + URL)
    #     print(form["delete"].value)
    #     os.remove(os.path.join(pathToUploads, form["delete"].value))

# Test if the file was deleted
# if fileitem.filename:
#     # strip leading path from file name to avoid
#     # directory traversal attacks
#     fn = os.path.basename(fileitem.filename)
#     try:
#         parent_dir = os.getcwd()
#         with open(parent_dir + '/resources/uploads/' + fn, 'wb') as f:
#             f.write(fileitem.file.read())
#         #   message = 'The file ' + fn + ' was uploaded successfully'
#     except IOError as e:
#         message = 'Error uploading file: ' + str(e)
#         sys.stderr.write('Error uploading file: ' + str(e))
#         print(message)
#         exit

# Define the HTML template
html_template = """
<!DOCTYPE html>
<html>
<head>
    <h4><a href='cgi_index.html'> Main Cgi Page </a></h4>
    <title>Folder Contents</title>
</head>
<body>
	<h3>This is the file python_cgi_DELETE.py</h3>
    <p>(path  ./resources/cgi/python_cgi_upload.py)</p>
    <br>
    <h2>Uploaded Files:</h2>
    <fieldset style="background: #fff3f3; border: 2px solid #fbd58f;">
        <legend> DELETE FILE/IMAGE (method DELETE) </legend>
        <ul>
            <a href=''> {} </a>
        </ul>
    </fieldset>
</body>
</html>
"""

# <input type="hidden" name="delete" value="{}">
# Define the item HTML template
item_template = """
<form method="post" action="python_cgi_DELETE.py">
	{}
	<br>
	<input type="submit" value="Delete">
</form>
<br>
"""

# Generate the item HTML
items_html = ""
for item in os.listdir(pathToUploads):
    item_path = os.path.join(pathToUploads, item)
    item_html = item_template.format(item, item_path)
    items_html += item_html

# Generate the full HTML page
html = html_template.format(items_html)

# Set the content type to HTML
# print("Content-type:text/html\r\n\r\n")

# Output the HTML page
print(html)
