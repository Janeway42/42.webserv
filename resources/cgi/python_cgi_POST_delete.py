#!/usr/bin/python

import time
import cgitb
import cgi
import os
import sys # to read from std input
# cgitb.enable()

# GET, POST, PUT and DELETE (there are others) are a part of the HTTP standard.
# HTML only defines the use of POST and GET for forms.

form = cgi.FieldStorage()

# Define the path to the folder you want to list
rootFolder = os.getcwd()
#pathToUploads = rootFolder + '/resources/uploads/'  # TODO this path has to come from the config file??? (look pdF)
pathToUploads = '../uploads/'  # TODO this path has to come from the config file??? (look pdF)

for param in os.environ.keys():
    # print("<b>%30s</b>: %s</br>") % (param, os.environ[param])
    # sys.stderr.write(param + ':  ')
    # sys.stderr.write(os.environ[param] + '\n')
    if param == 'REQUEST_METHOD':
        URL = os.environ[param]
        # If it is a POST request, we can get the form from the url
        if URL == "POST":
            # print("<p>FOUND METHOD: " + URL + "<br>")
            # If the form has a delete key on it, we can get the string after this key, which is the file to be deleted
            if form["delete"] is not None:
                formData = form["delete"].value
                os.remove(os.path.join(pathToUploads, form["delete"].value))

# Define the HTML template
html_template = """
<!DOCTYPE html>
<html>
<head>
    <h4><a href='../cgi_index.html'> Main Cgi Page </a></h4>
    <title>Folder Contents</title>
</head>
<body>
	<h3>This is the file python_cgi_POST_delete.py</h3>
    <p>(path ./resources/cgi/python_cgi_POST_delete.py)</p>
    <hr>
    <h2>File deleted:</h2>
    <br>
    <fieldset style="background: #fff3f3; border: 2px solid #ff0000;">
        <ul>
            <a href=''> {} </a>
        </ul>
    </fieldset>
</body>
</html>
"""

# # Define the HTML template
# html_template = """
# <!DOCTYPE html>
# <html>
# <head>
# 	<title>Folder Contents</title>
# </head>
# <body>
# 	<h4><a href='../index.html'> Main Page </a></h4>
# 	<h4><a href='cgi_index.html'> Back </a></h4>
# 	<h1>Upload Folder Content:</h1>
# 	<ul>
# 		{}
# 	</ul>
# </body>
# </html>
# """

# # Define the item HTML template
# item_template = """
# <li>
# 	{}
# 	<form method="post" action="python_cgi_delete.py">
# 		<input type="hidden" name="delete" value="{}">
# 		<input type="submit" value="Delete">
# 	</form>
# </li>
# """

# # Generate the item HTML
# items_html = ""
# for item in os.listdir(pathToUploads):
#     item_path = os.path.join(pathToUploads, item)
#     item_html = item_template.format(item, item_path)
#     items_html += item_html

# Generate the full HTML page
html = html_template.format(formData)
# html = html_template.format(items_html)

# Set the content type to HTML
print("Content-type:text/html\r\n\r\n")

# Output the HTML page
print(html)

# print("<br>Form data from url: <b>%30s</b>") % (formData)
