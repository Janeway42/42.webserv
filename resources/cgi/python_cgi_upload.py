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
       parent_dir = os.getcwd()
       with open(parent_dir + '/resources/uploads/' + fn, 'wb') as f:
           f.write(fileitem.file.read())
         #   message = 'The file ' + fn + ' was uploaded successfully'
   except IOError as e:
       message = 'Error uploading file: ' + str(e)
       sys.stderr.write('Error uploading file: ' + str(e))
       print (message)
       exit




# Define the path to the folder you want to list
folder_path = parent_dir + '/resources/uploads/'

# Define the HTML template
html_template = """
<!DOCTYPE html>
<html>
<head>
    <title>Folder Contents</title>
</head>
<body>
    <h1>Folder Contents:</h1>
    <ul>
        {}
    </ul>
</body>
</html>
"""


# Define the item HTML template
item_template = """
<li>
    {}
    <form method="delete" action="">
        <input type="hidden" name="delete" value="{}">
        <input type="submit" value="Delete">
    </form>
</li>
"""


# Check if a delete request has been made
form = cgi.FieldStorage()
if "delete" in form:
    os.remove(os.path.join(folder_path, form["delete"].value))
    
    
# Generate the item HTML
items_html = ""
for item in os.listdir(folder_path):
    item_path = os.path.join(folder_path, item)
    item_html = item_template.format(item, item_path)
    items_html += item_html
    
    
# Generate the full HTML page
html = html_template.format(items_html)

# Set the content type to HTML
print("Content-type:text/html\r\n\r\n")

# Output the HTML page
print(html)