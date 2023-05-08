#!/usr/bin/python

import time
import cgitb
import cgi
import os
import sys  # to read from std input
# cgitb.enable()

environ = os.environ.copy()

for param in os.environ.keys():
    if param == 'UPLOAD_DIR':
        upload_dir = os.environ[param]



form = cgi.FieldStorage()
# sys.stderr.write('FORM IS [' + str(form) + ']\n')

if 'filename' not in form or not form['filename'].value:
	print("Error: You need to choose a file to be uploaded ")
	exit ()


# Get filename here
fileItem = form['filename']

# Test if the file was uploaded
if fileItem.filename:
	# strip leading path from file name to avoid
	# directory traversal attacks
	fn = os.path.basename(fileItem.filename)
	try:
		parent_dir = os.getcwd()
	#	with open(parent_dir + '/resources/uploads/' + fn, 'wb') as f:  # TODO this path has to come from the config file??? (look pdF)
		# with open('../uploads/' + fn, 'wb') as f:  # TODO this path has to come from the config file??? (look pdF)
		with open(upload_dir + "/" + fn, 'wb') as f:  # TODO this path has to come from the config file??? (look pdF)
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
</head>
<body>
	<h4><a href='../cgi_index.html'> Main Cgi Page </a></h4>
	<h3>This is the file python_cgi_POST_upload.py</h3>
	<p>(path  ./resources/cgi/python_cgi_POST_upload.py)</p>
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
	{}
	<button> <a href="../uploads/{}">  View </a> </button> 
	<button> <a href="../uploads/{}" download>  Download </a> </button>
	<br>
	<input type="hidden" name="delete" value="{}">
	<input type="submit" value="Delete">
</form>
<br>
"""

# # Check if a delete request has been made // (joyce comment for jaka) I have moved it to the python_cgi_POST_delete.py
# form = cgi.FieldStorage()
# if "delete" in form:
#     os.remove(os.path.join(folder_path, form["delete"].value))

# Generate the item HTML
items_html = ""
for item in os.listdir(upload_dir):
	item_path = os.path.join(upload_dir, item)
	item_html = item_template.format(item, item, item, item_path)
	items_html += item_html

# Generate the full HTML page
html = html_template.format(items_html)

# Set the content type to HTML
# print("Content-type:text/html\r\n\r\n")

# Output the HTML page
print(html)
