
# Parse the query string
OIFS=$IFS
IFS='=&'
params=($QUERY_STRING)
IFS=$OIFS

# Get data from fields
name=${params[1]}
age=${params[3]}

echo "<h1>Hello $name!</h1>"
echo "<p>You are $age years old.</p>"