import SocketServer
import SimpleHTTPServer

PORT = 29392 

def move():
	""" sample function to be called via a URL"""
	return 'hi'

def top_level_resource(path_string):
	path_array = path_string.split('/')
	return path_array[1];	

class CustomHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
	# custom function for GET requests
	def do_GET(self):

		# decide what action should take place based on the URI
		if top_level_resource(self.path) == 'list':
			self.send_response(200)
			self.send_header('Content-type','text')
			self.end_headers()
			self.wfile.write('list:\n')
			self.wfile.write(self.path)
			self.wfile.write('\n')
			return

		if top_level_resource(self.path) == 'tagdata':
			self.send_response(200)
			self.send_header('Content-type','text')
			self.end_headers()
			self.wfile.write('tagdata:\n')
			self.wfile.write(self.path)
			self.wfile.write('\n')
			return

		if self.path=='/move':
			self.send_response(200)
			self.send_header('Content-type','text/html')
			self.end_headers()
			self.wfile.write(move()) #call sample function here
			return
		else:

			# default SimpleHTTPServer GET handler serves files from
			#  the current working directory
			SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

httpd = SocketServer.ThreadingTCPServer(('localhost', PORT),CustomHandler)
print "serving at port", PORT
httpd.serve_forever()

