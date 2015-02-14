#!/usr/bin/python

from binascii import unhexlify
import redis
import sys
from urllib import quote
from urlparse import urlsplit

redis_db = redis.StrictRedis()
redis_db.srem('js_path_checksums', unhexlify(sys.argv[1]))
redis_db.srem('js_script_checksums', unhexlify(sys.argv[2]))
