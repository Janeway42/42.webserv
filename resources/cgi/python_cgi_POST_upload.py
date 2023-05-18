#!/usr/bin/python

import time
import cgitb
import cgi
import os
import sys  		# to read from std input
import urllib.parse	# to encode spec. chars in filename
cgitb.enable()		# detailed errors msgs


environ = os.environ.copy()

for param in os.environ.keys():
	if param == 'UPLOAD_DIR':
		uploadDir_AbsPath = os.environ[param]
	if param == 'PATH_INFO':
	    uploadDirName = os.environ[param]

# sys.stderr.write('!!! From python uploads: ')
# sys.stderr.write(uploadDirName)
# sys.stderr.write('\n')


form = cgi.FieldStorage()
# sys.stderr.write('FORM IS [' + str(form) + ']\n')
# Get filename here
fileItem = form['filename']
# fileItem

# sys.stderr.write('!!! fileItem: [')
# sys.stderr.write(fileItem.filename)
# sys.stderr.write(']\n')

if 'filename' not in form or not form['filename'].value:
	print("Error: You need to choose a file to be uploaded")
	exit ()






# Test if the file was uploaded
if fileItem.filename:
	# strip leading path from file name to avoid
	# directory traversal attacks
	fn = os.path.basename(fileItem.filename)
	try:
		parent_dir = os.getcwd()
	#	with open(parent_dir + '/resources/uploads/' + fn, 'wb') as f:  # TODO this path has to come from the config file??? (look pdF)
		# with open('../uploads/' + fn, 'wb') as f:  # TODO this path has to come from the config file??? (look pdF)
		with open(uploadDir_AbsPath + "/" + fn, 'wb') as f:  # TODO this path has to come from the config file??? (look pdF)
			f.write(fileItem.file.read())
			message = 'The file ' + fn + ' was uploaded successfully'
	except IOError as e:
		message = 'Error uploading file: ' + str(e)
		sys.stderr.write('Error uploading file: ' + str(e))
		print(message)
		exit ()

# Define the path to the folder you want to list
# folder_path = parent_dir + '/resources/uploads/'  # TODO this path has to come from the config file??? (look pdF)
# folder_path = '../uploads/'  # TODO this path has to come from the config file??? (look pdF)

# Define the HTML template
html_template = """
<!DOCTYPE html>
<html>
<head>
	<title>Uploaded Files</title>
	<style>
		.button-container {{
			display: flex;
		}}
  		.button-container a {{
			color: black;
			text-decoration: none;
		text
		}}
	</style>
</head>
<body>
	<h4><a href='../cgi_index.html'> Main Cgi Page </a></h4>
	<h3>This is the file python_cgi_POST_upload.py</h3>
	<hr>
	<h2>Uploaded Files:</h2>
	<br>
	<fieldset style="background: #fff3f3; border: 2px solid #fbd58f;">
		<legend> DELETE FILE/IMAGE (method POST) </legend>
		<ul>
			{}
		</ul>
	</fieldset>
</body>
</html>
"""

# <input type="hidden" name="delete" value="{}">
# Define the item HTML template

# TODO The uploads folder must come from config file
item_template = """
<form action="python_cgi_POST_delete.py" method="post">
	<mark><b>{}</b></mark>
	<div class="button-container">
		<button> <a href="../{}/{}">  View </a> </button> 
		<button> <a href="../{}/{}" download> Download </a> </button>
		<input type="hidden" name="delete" value="{}">
		<input type="submit" value="Delete">
	</div>
</form>
<br>
"""

# # Check if a delete request has been made // (joyce comment for jaka) I have moved it to the python_cgi_POST_delete.py
# form = cgi.FieldStorage()
# if "delete" in form:
#     os.remove(os.path.join(folder_path, form["delete"].value))

# Generate the item HTML
items_html = ""
for item in os.listdir(uploadDir_AbsPath):
	# encodedItem = urllib.parse.quote(item)
	# sys.stderr.write('!!! encoded item: ')
	# sys.stderr.write(encodedItem)
	# sys.stderr.write('\n')
	item_path = os.path.join(uploadDir_AbsPath, item)
	item_html = item_template.format(item, uploadDirName, item, uploadDirName, item, item_path)
	# item_html = item_template.format(item, item, item, item_path)
	items_html += item_html

# Generate the full HTML page
html = html_template.format(items_html)

# Set the content type to HTML
# print("Content-type:text/html\r\n\r\n")

# Output the HTML page
print(html)

sys.stderr.write('From Python Upload: end of script\n')
# sys.stderr.write(html)




