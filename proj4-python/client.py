import httplib
import urllib
import et.ElementTree as ET
from optparse import OptionParser

def file_listing(path,root):
	for node in root.findall('file'):
		print path + node.text
	
	for node in root.findall('directory'):
		file_listing(path+node.attrib['name']+"/", node)
	
	return
		


parser = OptionParser()
parser.add_option('--get',action="store_true", default=False)
parser.add_option('--post',action="store_true", default=False)

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

parser = ET.XMLTreeBuilder()
parser.feed(data)

tree = ET.ElementTree(parser.close())

root = tree.getroot()
#print root.text
ET.dump(tree)

if options.action == '/list' and method == "GET":
	print "File listing: "
	file_listing("/", root)
