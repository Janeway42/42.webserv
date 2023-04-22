#!/usr/bin/python

# GET, POST, PUT and DELETE (there are others) are a part of the HTTP standard.
# HTML only defines the use of POST and GET for forms.

import time
import cgitb
import cgi
import os
import sys  # to read from std input

# cgitb.enable()

form = cgi.FieldStorage()

# Define the path to the folder you want to list
rootFolder = os.getcwd()
pathToUploads = rootFolder + '/resources/uploads/'  # TODO this path has to come from the config file??? (look pdF)

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
    <h4><a href='cgi_index.html'> Main Cgi Page </a></h4>
    <title>Folder Contents</title>
</head>
<body>
	<h3>This is the file python_cgi_DELETE.py</h3>
    <p>(path ./resources/_cgi/python_cgi_DELETE.py)</p>
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

# Generate the full HTML page
html = html_template.format(formData)

# Set the content type to HTML
# print("Content-type:text/html\r\n\r\n")

# Output the HTML page
print(html)

# print("<br>Form data from url: <b>%30s</b>") % (formData)
