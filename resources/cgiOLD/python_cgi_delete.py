#!/usr/bin/python
import time
import sys	# to read from std input
import cgi, os
import cgitb
# cgitb.enable()


form = cgi.FieldStorage()

# Define the path to the folder you want to list
rootFolder = os.getcwd()
pathToUploads = rootFolder + '/resources/uploads/'

for param in os.environ.keys():
	# print ("<b>%30s</b>: %s</br>") % (param, os.environ[param])
	# print (param, os.environ[param])
	# sys.stderr.write(param + ':  ')
	# sys.stderr.write(os.environ[param] + '\n')
 
	if param == 'REQUEST_METHOD':
		URL = os.environ[param]
		# print("<p>FOUND METHOD:")
		# print ("<span style='background-color:plum; padding:1%;'>" + URL + "</span><p>")
		#if URL == "delete":


os.remove(os.path.join(pathToUploads, form["delete"].value))






# Define the HTML template
html_template = """
<!DOCTYPE html>
<html>
<head>
	<title>Folder Contents</title>
</head>
<body>
	<h4><a href='cgi_index.html'> Main Page </a></h4>
	<h1>Upload Folder Content:</h1>
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
	<form method="post" action="python_cgi_delete.py">
		<input type="hidden" name="delete" value="{}">
		<input type="submit" value="Delete">
	</form>
</li>
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
print("Content-type:text/html\r\n\r\n")

# Output the HTML page
print(html)