import http.server
import socketserver
import os

# class HttpRequestHandler(http.server.SimpleHTTPRequestHandler):
#     def do_GET(self):
#         if self.path == '/':
#             self.path = 'index.html'
#         return http.server.SimpleHTTPRequestHandler.do_GET(self)
#

path = os.path.join(os.path.dirname(__file__), 'doc/html')
os.chdir(path)

handler = http.server.SimpleHTTPRequestHandler

PORT=8000

with socketserver.TCPServer(("", PORT), handler) as httpd:
    print("Server started at localhost: " + str(PORT))
    httpd.serve_forever()
