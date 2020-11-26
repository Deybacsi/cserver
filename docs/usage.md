## Server

### Starting the server

Simply run the `server` binary to start server.

Alternatively, you can use the `compile.sh` script to compile, and automatically run it.

You can use it as a daemon when running it with `nohup` or `screen`.

### Stopping server

Press CTRL-C to terminate the server, or `kill` it when running in the background.

This exit method can cause problems, when a client is connected, so a more failsafe method shuld be implemented in the future for graceful exit.

### Communication

You can use the provided client to communicate with the server, or you can use `curl`.

Store a key:
```
> curl -X PUT -d mykey=myvalue 127.0.0.1:8080
Key added
```
Read a key:
```
> curl -X GET -d mykey 127.0.0.1:8080
myvalue
```

If an error occurs, the server responds back with a HTTP 400/404/500 header, and with the description of the problem.

```
> curl -X GET -d mykey2 127.0.0.1:8080
ERROR: Key doesn't exist in database
```

## Client

When running with `-h` or `--help` command line arguments, the client prints out it's usage:

```
> python client.py --help

usage: client.py [-h] -u URL -p PORT -t TYPE -k KEY [-v VALUE]

Makes a GET/PUT request to specified URL:port with a given key=value pair.

optional arguments:
  -h, --help            show this help message and exit
  -u URL, --url URL     Target URL
  -p PORT, --port PORT  Target port
  -t TYPE, --type TYPE  Request type: GET | PUT
  -k KEY, --key KEY     Key
  -v VALUE, --value VALUE
                        Value
```

To store a key:

```
> python client.py -u 127.0.0.1 -p 8080 -t PUT -k mykey -v myvalue
HTTP 200
Key added
```

To retrieve a key:

```
> python client.py -u 127.0.0.1 -p 8080 -t GET -k mykey
HTTP 200
myvalue
```

The client has basic error checking for some cases:
```
> python client.py -u 127.0.0.1 -p 8080 -t PUT -k mykey
Please specify a value for key with -v / --value !
```

