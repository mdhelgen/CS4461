import httplib
import sys
import urllib
import et.ElementTree as ET
from optparse import OptionParser
import math

def file_listing(path,root):
	for node in root.findall('file'):
		print path + node.text
	
	for node in root.findall('directory'):
		file_listing(path+node.attrib['name']+"/", node)
	
	return

def tag_data(path,root,options):
	for node in root.findall('mp3'):
		if options.title:
			print 'title:   ' + node.find('title').text
		if options.artist:
			print 'artist:  ' + node.find('artist').text
		if options.album:
			print 'album:   ' + node.find('album').text
		if options.bitrate:
			print 'bitrate: ' + str(int(node.find('bitrate').text)/1000)+'kbps'
		if options.length:
			length = float(node.find('length').text)
			print 'length:  ' + str(int(math.floor(length/60)))+'m '+str(int(math.floor(length - 60*math.floor(length/60))))+'s'
		print '----------------------------'
		


parser = OptionParser()
parser.add_option('--get',action="store_true", default=False)
parser.add_option('--post',action="store_true", default=False)

parser.add_option('--title', action="store_true", default=False)
parser.add_option('--album', action="store_true",  default=False)
parser.add_option('--artist', action="store_true", default=False)
parser.add_option('--bitrate', action="store_true", default=False)
parser.add_option('--length', action="store_true", default=False)


parser.add_option('-a','--action', dest="action")
parser.add_option('-p','--path', dest="path")

(options, args) = parser.parse_args()

if options.get:
	method="GET"

if options.post:
	method="POST"

if options.action[0] != '/':
	options.action = '/'+options.action

if options.path[0] != '/':
	options.path = '/'+options.path


conn = httplib.HTTPConnection("localhost:29392")
conn.request(method,options.action+options.path)

resp = conn.getresponse()

data = resp.read()

if options.action == '/download':
	print sys.getsizeof(data)
	exit()

parser = ET.XMLTreeBuilder()
parser.feed(data)

tree = ET.ElementTree(parser.close())

root = tree.getroot()
#print root.text
ET.dump(tree)

if options.action == '/list' and method == "GET":
	print "File listing: "
	file_listing(options.path, root)

if options.action == '/tagdata' and method == "GET":
	print "Tag Data: "
	tag_data("/", root, options)
