## Server

`curl` was used for testing because its verbose output is very helpful for debugging and error checking.

### Storing a key

```
curl -v -X PUT -d mykey=myvalue 127.0.0.1:8080/
*   Trying 127.0.0.1...
* TCP_NODELAY set
* Connected to 127.0.0.1 (127.0.0.1) port 8080 (#0)
> PUT / HTTP/1.1
> Host: 127.0.0.1:8080
> User-Agent: curl/7.58.0
> Accept: */*
> Content-Length: 13
> Content-Type: application/x-www-form-urlencoded
> 
* upload completely sent off: 13 out of 13 bytes
< HTTP/1.1 200 OK
< Connection: close
< Content-Length: 9
< 
* Closing connection 0
Key added
```

### Retrieving a key

```
curl -v -X GET -d mykey 127.0.0.1:8080
* Rebuilt URL to: 127.0.0.1:8080/
*   Trying 127.0.0.1...
* TCP_NODELAY set
* Connected to 127.0.0.1 (127.0.0.1) port 8080 (#0)
> GET / HTTP/1.1
> Host: 127.0.0.1:8080
> User-Agent: curl/7.58.0
> Accept: */*
> Content-Length: 5
> Content-Type: application/x-www-form-urlencoded
> 
* upload completely sent off: 5 out of 5 bytes
< HTTP/1.1 200 OK
< Connection: close
< Content-Length: 7
< 
* Closing connection 0
myvalue
```

### Modifying a key

```curl -v -X PUT -d mykey=myvalue 127.0.0.1:8080/
*   Trying 127.0.0.1...
* TCP_NODELAY set
* Connected to 127.0.0.1 (127.0.0.1) port 8080 (#0)
> PUT / HTTP/1.1
> Host: 127.0.0.1:8080
> User-Agent: curl/7.58.0
> Accept: */*
> Content-Length: 13
> Content-Type: application/x-www-form-urlencoded
> 
* upload completely sent off: 13 out of 13 bytes
< HTTP/1.1 200 OK
< Connection: close
< Content-Length: 12
< 
* Closing connection 0
Key modified
```

### Requesting a non-existing key

```
curl -v -X GET -d nonekey 127.0.0.1:8080
* Rebuilt URL to: 127.0.0.1:8080/
*   Trying 127.0.0.1...
* TCP_NODELAY set
* Connected to 127.0.0.1 (127.0.0.1) port 8080 (#0)
> GET / HTTP/1.1
> Host: 127.0.0.1:8080
> User-Agent: curl/7.58.0
> Accept: */*
> Content-Length: 7
> Content-Type: application/x-www-form-urlencoded
> 
* upload completely sent off: 7 out of 7 bytes
< HTTP/1.1 404 Not Found
< Connection: close
< Content-Length: 36
< 
* Closing connection 0
ERROR: Key doesn't exist in database
```

### Trying to store a key without value

```
curl -v -X PUT -d mykey= 127.0.0.1:8080/
*   Trying 127.0.0.1...
* TCP_NODELAY set
* Connected to 127.0.0.1 (127.0.0.1) port 8080 (#0)
> PUT / HTTP/1.1
> Host: 127.0.0.1:8080
> User-Agent: curl/7.58.0
> Accept: */*
> Content-Length: 6
> Content-Type: application/x-www-form-urlencoded
> 
* upload completely sent off: 6 out of 6 bytes
< HTTP/1.1 400 Bad Request
< Connection: close
< Content-Length: 34
< 
* Closing connection 0
ERROR: Specify a value for the key
```

### Sending non acceptable request type (anything than PUT or GET)

```
curl -v -X DELETE -d mykey 127.0.0.1:8080/
*   Trying 127.0.0.1...
* TCP_NODELAY set
* Connected to 127.0.0.1 (127.0.0.1) port 8080 (#0)
> DELETE / HTTP/1.1
> Host: 127.0.0.1:8080
> User-Agent: curl/7.58.0
> Accept: */*
> Content-Length: 5
> Content-Type: application/x-www-form-urlencoded
> 
* upload completely sent off: 5 out of 5 bytes
< HTTP/1.1 400 Bad Request
< Connection: close
< Content-Length: 42
< 
* Closing connection 0
ERROR: Call with PUT or GET requests only.
```

### Sending GET without key

```
curl -v -X GET 127.0.0.1:8080
Note: Unnecessary use of -X or --request, GET is already inferred.
* Rebuilt URL to: 127.0.0.1:8080/
*   Trying 127.0.0.1...
* TCP_NODELAY set
* Connected to 127.0.0.1 (127.0.0.1) port 8080 (#0)
> GET / HTTP/1.1
> Host: 127.0.0.1:8080
> User-Agent: curl/7.58.0
> Accept: */*
> 
< HTTP/1.1 400 Bad Request
< Connection: close
< Content-Length: 36
< 
* Closing connection 0
ERROR: Specify a key in request body
```

### Sending PUT without key

```
curl -v -X PUT 127.0.0.1:8080
* Rebuilt URL to: 127.0.0.1:8080/
*   Trying 127.0.0.1...
* TCP_NODELAY set
* Connected to 127.0.0.1 (127.0.0.1) port 8080 (#0)
> PUT / HTTP/1.1
> Host: 127.0.0.1:8080
> User-Agent: curl/7.58.0
> Accept: */*
> 
< HTTP/1.1 400 Bad Request
< Connection: close
< Content-Length: 36
< 
* Closing connection 0
ERROR: Specify a key in request body
```

### Sending invalid keyname

```
curl -v -X GET -d "mykey!*" 127.0.0.1:8080
curl -v -X GET -d "mykey-v -X PUT 127.0.0.1:8080" 127.0.0.1:8080
* Rebuilt URL to: 127.0.0.1:8080/
*   Trying 127.0.0.1...
* TCP_NODELAY set
* Connected to 127.0.0.1 (127.0.0.1) port 8080 (#0)
> GET / HTTP/1.1
> Host: 127.0.0.1:8080
> User-Agent: curl/7.58.0
> Accept: */*
> Content-Length: 29
> Content-Type: application/x-www-form-urlencoded
> 
* upload completely sent off: 29 out of 29 bytes
< HTTP/1.1 400 Bad Request
< Connection: close
< Content-Length: 59
< 
* Closing connection 0
ERROR: Only alphanumeric characters are allowed as keyname!
```

## Client

### Storing a key

```
python client.py -u 127.0.0.1 -p 8080 -t PUT -k mykey -v myvalue
HTTP 200
Key added
```

### Retrieving a key

```
python client.py -u 127.0.0.1 -p 8080 -t GET -k mykey 
HTTP 200
myvalue
```

### Modifying a key

```
python client.py -u 127.0.0.1 -p 8080 -t PUT -k mykey -v newvalue
HTTP 200
Key modified
```

### Sending invalid request type

```
python client.py -u 127.0.0.1 -p 8080 -t DELETE -k mykey 
Only GET or PUT request types allowed!
```

### Trying to store a key without value

```
python client.py -u 127.0.0.1 -p 8080 -t PUT -k mykey
Please specify a value for key with -v / --value !
```

### Request with server side error

```
python client.py -u 127.0.0.1 -p 8080 -t GET -k mykey22 
HTTP 404
ERROR: Key doesn't exist in database
```

### Storage keyfile not writable

```
chmod 400 serverdb/mykey
python client.py -u 127.0.0.1 -p 8080 -t PUT -k mykey -v newvalue
HTTP 500
ERROR: Error while writing key file on server
```
