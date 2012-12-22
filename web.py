#!/usr/bin/python

from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer
from os import curdir, sep
from gtp import GTPClient
import urllib, urlparse

webroot = curdir + sep + "pentagoo"
port = 8080

mime_types = {
	"html": 'text/html',
	"jpeg": 'image/jpeg',
	"jpg" : 'image/jpg',
	"png" : 'image/png',
	"gif" : 'image/gif',
	"ico" : 'image/ico',
	"js"  : 'application/javascript',
	"css" : 'text/css',
}
#convert from pentagoo's format
ymap = {'0':'a', '1':'b', '2':'c', '3':'d', '4':'e', '5':'f'}
xmap = {'0':'1', '1':'2', '2':'3', '3':'4', '4':'5', '5':'6'}
rmap = {'1r':'t', '1l':'s', '2r':'v', '2l':'u', '3r':'z', '3l':'y', '4r':'x', '4l':'w'}
#and their inverse to convert back
inv_ymap = {}
for k,v in ymap.iteritems(): inv_ymap[v] = k
inv_xmap = {}
for k,v in xmap.iteritems(): inv_xmap[v] = k
inv_rmap = {}
for k,v in rmap.iteritems(): inv_rmap[v] = k


class PentagoHandler(BaseHTTPRequestHandler):
	#Handler for the GET requests
	def do_GET(self):
		if '?' in self.path:
			uri, qs = self.path.split('?', 1)
			params = {}
			for k,v in urlparse.parse_qsl(qs, True): params[k] = urllib.unquote(v)
		else:
			uri, params = self.path, {}

		if uri == "/ai":
			self.call_ai(uri, params)
		else:
			self.serve_static(uri, params)

	def call_ai(self, uri, params):
		game = ""
		hist = filter(None, params['hist'].split(" "))
		for m in hist:
			game += " " + ymap[m[1]] + xmap[m[0]] + rmap[m[2:4]]

		try:
			gtp = GTPClient("./pentagod")
			if 'p' in params and params['p'] in ['mcts','pns','ab']:
				gtp.cmd(params['p'])
			if 't' in params and 0 < int(params['t']) <= 60:
				gtp.time(0, params['t'])
			gtp.cmd("playgame" + game)
			gtp.cmd("print")
			r = gtp.cmd("genmove")
			gtp.cmd("print")
		finally:
			gtp.close()

		self.send_response(200)
		self.send_header('Content-type', mime_types["html"])
		self.end_headers()
		self.wfile.write(inv_xmap[r[1]] + inv_ymap[r[0]] + inv_rmap[r[2]])

	def serve_static(self, uri, params):
		if uri == "/":
			uri = "/index.html"
		elif ".." in self.path or "/." in uri:
			self.send_error(403, 'Forbidden')
			return

		ending = uri.split('.')[-1]
		if ending in mime_types:
			try:
				f = open(webroot + sep + uri)
				self.send_response(200)
				self.send_header('Content-type', mime_types[ending])
				self.end_headers()
				self.wfile.write(f.read())
				f.close()
			except IOError:
				self.send_error(404,'File Not Found: %s' % self.path)
		else:
			self.send_error(404, "Unknown file type")

try:
	#Create a web server and define the handler to manage the incoming request
	server = HTTPServer(('', port), PentagoHandler)
	print 'Started httpserver on port', port
	server.serve_forever()

except KeyboardInterrupt:
	print '^C received, shutting down the web server'
	server.socket.close()

