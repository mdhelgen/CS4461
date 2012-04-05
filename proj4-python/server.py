import SocketServer
import SimpleHTTPServer

PORT = 29392 

def move():
	""" sample function to be called via a URL"""
	return 'hi'


# top_level_resource(path_string)
#
# 	returns the top level resource specified by the URI path
def top_level_resource(path_string):
	path_array = path_string.split('/')
	return path_array[1];	


# file_system_path(path_string)
# 
#   returns the file system path specified after the top level resource in the URI
def file_system_path(path_string):

	filesystem_path = ''
	path_array = path_string.split('/')
	for i in range(2, len(path_array)):
		filesystem_path += '/'
		filesystem_path += path_array[i]

	return filesystem_path


# list_GET_handler(self)
#
#  this function is called when the request is of type 'GET /list/...'
def list_GET_handler(self):
	
	self.send_response(200)
	self.send_header('Content-type','text')
	self.end_headers()

	print 'filesystem path is: '
	print file_system_path(self.path)
	print '\n'

	self.wfile.write('list:\n')
	self.wfile.write(self.path)
	self.wfile.write('\n')

	return

# tagdata_GET_handler(self)
#
#  this function is called when the request is of type 'GET /tagdata/...'
def tagdata_GET_handler(self):
	self.send_response(200)
	self.send_header('Content-type','text')
	self.end_headers()
	self.path.split('/')
	self.wfile.write('tagdata:\n')
	self.wfile.write(self.path)
	self.wfile.write('\n')
	
	return

# send_404_response(self)
#
#  Send a generic 404 response indicating the URI was not matched by anything
def send_404_response(self):
	self.send_response(404)
	self.send_header('Content-type','text')
	self.end_headers()
	self.wfile.write('%s %s %s\n' % (self.command, self.path, self.request_version))
	self.wfile.write('Error: the specified URI was not found\n')
	return

# handles incoming HTTP requests and calls the appropriate handler method

class CustomHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
	# custom function for GET requests
	def do_GET(self):

		# decide what action should take place based on the URI
		if top_level_resource(self.path) == 'list':
			list_GET_handler(self)
			return

		if top_level_resource(self.path) == 'tagdata':
			tagdata_GET_handler(self)
			return

		else:
			send_404_response(self)
			# default SimpleHTTPServer GET handler serves files from
			#  the current working directory
			#SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

httpd = SocketServer.ThreadingTCPServer(('localhost', PORT),CustomHandler)
print "serving at port", PORT

httpd.handle_request()
#httpd.serve_forever()

