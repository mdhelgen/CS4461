# os/filesystem modules
import sys
import os
import string

# http server modules
import SocketServer
import BaseHTTPServer
import urllib

# xml parse module
import et.ElementTree as ET

#mp3 tag modules
from mutagen.mp3 import MP3
from mutagen.easyid3 import EasyID3


PORT = 29392 
SERVE_DIR = './mp3'


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

	# the filesystem_path will begin at our served directory
	filesystem_path = '/' 

	path_array = urllib.unquote(path_string).split('/')
	# there's probably a better way to do this
	# 
        # the first entry in our URI path is the resource, so we skip that
        #  add the rest of the path to the filesystem_path and return it
	for i in range(2, len(path_array)):
		# avoid having paths such as .//
		if path_array[i] != '':
			filesystem_path += path_array[i]
			filesystem_path += '/'

	# ugh, very ugly here as well
	# unless the path is simply "/", remove the trailing '/' from the return value
	# this is an issue because  ./dir and ./dir/ are both seen as directories, but
        #    ./file.mp3 and ./file.mp3/ will not both be seen as existing files
	if len(filesystem_path) == 1: 
		return filesystem_path
	else: 	
		return filesystem_path[:-1]


# build_dir_xml(parent_node, path)
#
#   recursively builds an xml tree. 
#   parent_node is the xml node of the parent directory
#   path is the current directory path we are exploring
#
#   all of the files are added first, then the directories are recursively added
#
def build_dir_xml(parent_node, path):

	path = string.replace(path, '//','/')

	# list the regular files in the current directory
	for entry in os.listdir(SERVE_DIR + path):
		if not os.path.isdir(SERVE_DIR + path +"/"+ entry):
			print SERVE_DIR + path + entry
			file = ET.SubElement(parent_node, 'file')
			file.set('path', path)
			file.text = entry

	# list directories. make a new directory tag and then recursively add the files (and directories) under it
	for entry in os.listdir(SERVE_DIR + path):
		if os.path.isdir(SERVE_DIR + path + "/"+entry):
			dir = ET.SubElement(parent_node, 'directory')
			dir.set('path', path)
			dir.set('name', entry)
			build_dir_xml(dir, path +'/'+ entry + '/')

	return

# list_GET_handler(self)
#
#  this function is called when the request is of type 'GET /list/...'
def list_GET_handler(self):

	# get the path from the URI	
	fs_path = file_system_path(self.path)

	# if the path doesn't exist, then return a 404 error	
	if not os.path.exists(SERVE_DIR + fs_path):
		send_404_response(self)	
		return

	# create the xml tree
	root = ET.Element('list')
	
	# the root path is the server directory
	root.set('path', SERVE_DIR + fs_path)

	# build_dir_xml recursively adds files/directories into the XML tree
	build_dir_xml(root, fs_path)	
	
	# make an ElementTree with the root node
	tree = ET.ElementTree(root)

	self.send_response(200)
	self.send_header('Content-type','text/xml')
	self.end_headers()
	
	# write the tree to the response body 
	tree.write(self.wfile)

	return

# tagdata_GET_handler(self)
#
#  this function is called when the request is of type 'GET /tagdata/...'
def tagdata_GET_handler(self):

	fs_path = file_system_path(self.path)

	if not os.path.exists(SERVE_DIR + fs_path):
		send_404_response(self)
		return

	root = ET.Element("tagdata")
	
	root.set('path', fs_path)

	# special case if the path points directly at an mp3
	if not os.path.isdir(SERVE_DIR + fs_path):
		node = ET.SubElement(root, 'mp3')
		node.set('path', fs_path)

		audio = EasyID3(SERVE_DIR + fs_path)
		audiomp3 = MP3(SERVE_DIR + fs_path)

		title = ET.SubElement(node, 'title')
		title.text = audio["title"][0]

		artist = ET.SubElement(node, 'artist')
		artist.text = audio['artist'][0]

		album = ET.SubElement(node, 'album')
		album.text = audio['album'][0]
		
		length = ET.SubElement(node, 'length')
		length.text = audiomp3.info.length

		bitrate = ET.SubElement(node, 'bitrate')
		bitrate.text = audiomp3.info.bitrate
	# 
	else:			
		for entry in os.listdir(SERVE_DIR + fs_path):
			if os.path.splitext(entry)[1] == '.mp3':
				node = ET.SubElement(root, 'mp3')
				node.set('filename', entry)

				audio = EasyID3(SERVE_DIR + fs_path + entry)
				audiomp3 = MP3(SERVE_DIR + fs_path + entry)
				
				title = ET.SubElement(node, 'title')
				title.text = audio["title"][0]
				artist = ET.SubElement(node, 'artist')
				artist.text = audio['artist'][0]
				album = ET.SubElement(node, 'album')
				album.text = audio['album'][0]

				length = ET.SubElement(node, 'length')
				length.text = str(audiomp3.info.length)

				bitrate = ET.SubElement(node, 'bitrate')
				bitrate.text = str(audiomp3.info.bitrate)
			
	
	tree = ET.ElementTree(root)

	self.send_response(200)
	self.send_header('Content-type','text/xml')
	self.end_headers()
	tree.write(self.wfile)
	
	return

def list_POST_handler(self):
	send_404_response(self)
	return

def tagdata_POST_handler(self):
	send_404_response(self)
	return

def upload_POST_handler(self):
	send_404_response(self)
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

class CustomHandler(BaseHTTPServer.BaseHTTPRequestHandler):
	def do_POST(self):
		length = int(self.headers['Content-Length'])
		print self.rfile.read(length)
		if top_level_resource(self.path) == 'list':
			list_POST_handler(self)
			return
		if top_level_resource(self.path) == 'tagdata':
			tagdata_POST_handler(self)
			return
		if top_level_resource(self.path) == 'upload':
			upload_POST_handler(self)
			return
		send_404_response(self)
		return

	# custom function for GET requests
	def do_GET(self):

		# decide what action should take place based on the URI
		if top_level_resource(self.path) == 'list':
			list_GET_handler(self)
			return

		if top_level_resource(self.path) == 'tagdata':
			tagdata_GET_handler(self)
			return
		if top_level_resource(self.path) == 'download':
			fs_path = file_system_path(self.path)
			self.path = SERVE_DIR + fs_path
			BaseHTTPServer.BaseHTTPRequestHandler.do_GET(self)
			return

		else:
			send_404_response(self)
			# default SimpleHTTPServer GET handler serves files from
			#  the current working directory
			#SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

httpd = SocketServer.ThreadingTCPServer(('localhost', PORT),CustomHandler)
print "serving at port", PORT

#httpd.handle_request()
httpd.serve_forever()

