
import cgi
import cgitb

cgitb.enable()  # for troubleshooting

print("Content-type: text/html\n")

# Create instance of FieldStorage
form = cgi.FieldStorage()

# Get data from fields
name = form.getvalue('name')
age  = form.getvalue('age')

print(f"<h1>Hello {name}!</h1>")
print(f"<p>You are {age} years old.</p>")