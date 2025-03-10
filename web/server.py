import sys
import http.server
import socketserver

PORT = 8000

if len(sys.argv) > 1:
    PORT = int(sys.argv[1])

class MyHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
	def end_headers(self):
		self.send_header('Cross-Origin-Opener-Policy', 'same-origin')
		self.send_header('Cross-Origin-Embedder-Policy', 'require-corp')
		self.send_header('Access-Control-Allow-Origin', '*')
		super().end_headers()

Handler = MyHTTPRequestHandler

with socketserver.TCPServer(("", PORT), Handler) as httpd:
	print(f"Serving at port {PORT}")
	httpd.serve_forever()

