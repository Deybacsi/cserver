import argparse
import requests

# setup command line arguments
parser = argparse.ArgumentParser(description='Makes a GET/PUT request to specified URL:port with a given key=value pair.')
parser.add_argument('-u', '--url',  type=str, required = True,  help='Target URL' )
parser.add_argument('-p', '--port', type=str, required = True,  help='Target port')
parser.add_argument('-t', '--type', type=str, required = True,  help='Request type: GET | PUT ')
parser.add_argument('-k', '--key',  type=str, required = True,  help='Key')
parser.add_argument('-v', '--value',type=str, required = False, help='Value')

clargs = parser.parse_args()
connectionUrl='http://' + clargs.url +':'+ clargs.port

if clargs.type != 'PUT' and clargs.type != 'GET':                                   # if request type is invalid
        print "Only GET or PUT request types allowed!"
        quit(1)

if clargs.type=='PUT':                                                              # PUT
    if clargs.value == None:
        print "Please specify a value for key with -v / --value !"
        quit(2)
    response=requests.put(connectionUrl, data = {clargs.key : clargs.value} )

if clargs.type=='GET':                                                              # GET
    response=requests.get(connectionUrl, data = clargs.key)

print 'HTTP' , response.status_code

print ('<null>') if response.text == '' else response.text                          # print return value or <null>
