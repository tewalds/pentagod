#!/usr/bin/python

from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer
from SocketServer import ThreadingMixIn
from os import curdir, sep
from gtp import GTPClient
import os, time, hashlib, urllib, urlparse

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
	protocol_version="HTTP/1.1"

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

		gtp = None
		try:
			gtp = GTPClient("./pentagod")
			if 'p' in params and params['p'] in ['mcts','pns','ab']:
				gtp.cmd(params['p'])
			if 't' in params and 0 < float(params['t']) <= 60:
				gtp.time(0, params['t'])
			gtp.cmd("playgame" + game)
			gtp.cmd("print")
			r = gtp.cmd("genmove")
			gtp.cmd("print")
		finally:
			if gtp:
				gtp.close()

		body = inv_xmap[r[1]] + inv_ymap[r[0]] + inv_rmap[r[2]]

		self.send_response(200)
		self.send_header('Content-type', mime_types["html"])
		self.send_header('Content-Length', len(body))
#		self.send_header('Connection', 'close')
		self.end_headers()
		self.wfile.write(body)

	def serve_static(self, uri, params):
		if uri == "/":
			uri = "/index.html"
		elif ".." in self.path or "/." in uri:
			self.send_error(403, 'Forbidden')
			return

		ending = uri.split('.')[-1]
		if ending in mime_types:
			try:
				filename = webroot + sep + uri
				f = open(filename, 'rb')
				st = os.fstat( f.fileno() )
				length = st.st_size
				data = f.read()
				md5 = hashlib.md5()
				md5.update(data)
				md5_key = self.headers.getheader('If-None-Match')
				if md5_key and md5_key[1:-1] == md5.hexdigest():
					self.send_response(304)
					self.send_header('ETag', '"{0}"'.format(md5.hexdigest()))
					self.send_header('Keep-Alive', 'timeout=5, max=100')
					self.end_headers()
					return
				self.send_response(200)
				self.send_header('Content-type', mime_types[ending])
				self.send_header('Content-Length', length )
				self.send_header('ETag', '"{0}"'.format(md5.hexdigest()))
				self.send_header('Accept-Ranges', 'bytes')
				self.send_header('Last-Modified', time.strftime("%a %d %b %Y %H:%M:%S GMT",time.localtime(os.path.getmtime(filename))))
				self.end_headers()
				self.wfile.write(data)
				f.close()
			except IOError:
				self.send_error(404,'File Not Found: %s' % self.path)
		else:
			self.send_error(404, "Unknown file type")




class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    pass

try:
	#Create a web server and define the handler to manage the incoming request
	server = ThreadedHTTPServer(('', port), PentagoHandler)
	print 'Started httpserver on port', port
	server.serve_forever()

except KeyboardInterrupt:
	print '^C received, shutting down the web server'
	server.socket.close()
