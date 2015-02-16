#!/usr/bin/python

from HTMLParser import HTMLParser
from argparse import ArgumentParser
from binascii import hexlify
from binascii import unhexlify
from hashlib import sha1
from http_parser.parser import HttpParser
import magic
import os
import redis
from slimit.parser import Parser as JSParser
import sys
from time import gmtime
from time import strftime
from traceback import print_exc
from urllib import FancyURLopener
from urllib import quote
from urllib import urlopen
from urlparse import urljoin
from urlparse import urlsplit
import warc

FancyURLopener.prompt_user_passwd = lambda *args, **kwargs: (None, None)

arg_parser = ArgumentParser()
arg_parser.add_argument('files', nargs = '+', type = str, metavar = 'FILE')
arg_parser.add_argument('--prefix', default = '.', type = str)
args = arg_parser.parse_args()

redis_db = redis.StrictRedis()

class ScriptExtractor(HTMLParser):
  def init(self, url, date):
    self.reset()
    self.url = url
    self.date = date
    self.pos = None
    self.last = dict()

  def handle_starttag(self, tag, attrs):
    if tag == 'script':
      self.pos = self.getpos()
      attrs = dict(attrs)
      src = attrs.get('src', '').strip()
      if src:
        url = urljoin(self.url, src.encode('latin_1', 'xmlcharrefreplace'))
        path = self._getpath(url)
        if self._addpath(path):
          try:
            urlres = urlopen(url)
            if urlres.getcode() != 200:
              raise IOError
            script = urlres.read().strip()
            if script.startswith('<!--') and script.endswith('-->'):
              script = script[4:-3].strip()
            if script:
              date = strftime('%a, %d %b %Y %H:%M:%S GMT', gmtime())
              self._addscript(path, date, url, script)
          except IOError:
            print >> sys.stderr, 'Cannot retrieve ' + url

  def handle_data(self, data):
    if self.pos:
      script = data.encode('latin_1').strip()
      if script.startswith('<!--') and script.endswith('-->'):
        script = script[4:-3].strip()
      if script:
        # Assume all paths are unique for inline scripts
        path = self._getpath(self.url, '%d,%d' % self.pos)
        self._addscript(path, self.date, self.url, script)

  def handle_endtag(self, tag):
    if tag == 'script':
      self.pos = None
      self.last = dict()

  def handle_error(self):
    p_checksum = self.last.get('path_checksum', None)
    if p_checksum:
      redis_db.srem('js_path_checksums', unhexlify(p_checksum))
    s_checksum = self.last.get('script_checksum', None)
    if s_checksum:
      redis_db.srem('js_script_checksums', unhexlify(s_checksum))
    print >> sys.stderr, self.pos, self.last

  def _getpath(self, url, inline = None):
    # Leave 63 bytes for prefix if the full path is restricted to 255 bytes.
    loc = urlsplit(url)
    loc = loc._replace(netloc = loc[1].lower(), fragment = inline)
    self.last['url'] = loc.geturl()
    path = (loc[1][3:] if loc[1].startswith('www') else loc[1])
    path = path.encode('ascii', 'ignore').translate(None, '.-').ljust(2, '_')
    path = path[0] + '/' + path[1] + '/' + quote(loc[1], '%')[:89]
    filename = quote(loc[2].strip('/') + ('#' + loc[4] if loc[4] else ''), '%')
    if not filename.endswith('.js'):
      filename += '.js'
    return path + '/' + filename[-92:]

  def _addpath(self, path):
    checksum = sha1(path).digest()
    if redis_db.sadd('js_path_checksums', checksum):
      self.last['path_checksum'] = hexlify(checksum)
      return True
    return False

  def _addscript(self, path, date, url, script):
    checksum = sha1(script).digest()
    if redis_db.sadd('js_script_checksums', checksum):
      self.last['script_checksum'] = hexlify(checksum)
      try:
        js_parser = JSParser()
        js_parser.parse(script)
      except Exception:
        # Keep unparsable scripts as 'bad' script if they don't look like HTML.
        mime_type = magic.from_buffer(script.replace('\v', ' '), mime = True)
        if mime_type == 'text/html' or mime_type == 'application/xml':
          return
        print >> sys.stderr, 'Cannot parse ' + path
        path = '.bad/' + path
      header = '// Retrieved %sfrom %s\n' % (date + ', ' if date else '', url)
      path = args.prefix + '/' + path
      try:
        os.makedirs(os.path.dirname(path))
      except OSError:
        pass
      jsfile = open(path, 'w')
      jsfile.write(header)
      jsfile.write(script)
      jsfile.close()

extractor = ScriptExtractor()

for filename in args.files:
  print >> sys.stderr, "Extracting JS files from %s..." % filename
  for record in warc.open(filename):
    if record.type == 'response':
      http_parser = HttpParser()
      http_parser.execute(record.payload, record.header.content_length)
      header = http_parser.get_headers()
      extractor.init(record.url, header.get('Date', None))
      try:
        extractor.feed(http_parser.recv_body().decode('latin_1'))
      except Exception:
        extractor.handle_error()
        print_exc()
      except:
        extractor.handle_error()
        raise
