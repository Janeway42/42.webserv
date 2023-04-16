#!/usr/bin/python
import time
import cgitb
import cgi
import os
import sys  # to read from std input

# cgitb.enable()

environ = os.environ.copy()

form = cgi.FieldStorage()
# sys.stderr.write('FORM IS [' + str(form) + ']\n')

# Get filename here.
fileItem = form['filename']
# print('fileItem.file:', fileItem.file)

# Test if the file was uploaded
if fileItem.filename:
    # strip leading path from file name to avoid
    # directory traversal attacks
    fn = os.path.basename(fileItem.filename)
    try:
        parent_dir = os.getcwd()
        with open(parent_dir + '/resources/uploads/' + fn, 'wb') as f:
            f.write(fileItem.file.read())
        #   message = 'The file ' + fn + ' was uploaded successfully'
    except IOError as e:
        message = 'Error uploading file: ' + str(e)
        sys.stderr.write('Error uploading file: ' + str(e))
        # print(message)
        exit

# Define the path to the folder you want to list
folder_path = parent_dir + '/resources/uploads/'

# Define the HTML template
html_template = """
<!DOCTYPE html>
<html>
<head>
    <h4><a href='cgi_index.html'> Main Cgi Page </a></h4>
    <title>Uploaded Files</title>
</head>
<body>
    <h3>This is the file python_cgi_upload.py</h3>
    <p>(path  ./resources/cgi/python_cgi_upload.py)</p>
    <hr>
    <h2>Uploaded Files:</h2>
    <br>
    <fieldset style="background: #fff3f3; border: 2px solid #fbd58f;">
        <legend> DELETE FILE/IMAGE (method POST) </legend>
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
<form action="python_cgi_DELETE.py" method="post">
    {}
    <br>
    <input type="hidden" name="delete" value="{}">
    <input type="submit" value="Delete">
</form>
<br>
"""

# # Check if a delete request has been made
# form = cgi.FieldStorage()
# if "delete" in form:
#     os.remove(os.path.join(folder_path, form["delete"].value))

# Generate the item HTML
items_html = ""
for item in os.listdir(folder_path):
    item_path = os.path.join(folder_path, item)
    item_html = item_template.format(item, item_path)
    items_html += item_html

# Generate the full HTML page
html = html_template.format(items_html)

# Set the content type to HTML
# print("Content-type:text/html\r\n\r\n")

# Output the HTML page
print(html)
