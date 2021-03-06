#!/usr/bin/env python3

import sys, time, os, re, random
import subprocess, shutil, glob, tempfile
import inspect # import currentframe, getframeinfo
import logging
import contextlib
import requests, socket, json
from pprint import pprint

HOME = os.environ.get('HOME')

def _query(numb, qf, url, quiet=0, timeout=15, headers={}, **kvargs):
    if not quiet:
        print('[{}] GET'.format(numb), url, 'T', timeout)
        for k,v in headers.items():
            print(' '*4,k,': ',v, sep='')
        print('[{}]'.format(numb), end=' ')
    try:
        resp = qf(url, timeout=int(timeout), headers=headers, **kvargs)
        if not quiet:
            print(resp.status_code)
            for k,v in resp.headers.items():
                print(' '*4,k,': ',v, sep='')
        return resp
    except (requests.exceptions.ReadTimeout,requests.packages.urllib3.exceptions.ReadTimeoutError,socket.timeout):
        pass
    return None

class Main(object):
    count_get = count_post = 0
    session = requests.Session()

    def _GET(self, url, **kvargs):
        numb = self.count_get
        self.count_get += 1
        return _query(numb, self.session.get, url, **kvargs)
    #@contextlib.contextmanager
    def _POST(self, url, **kvargs):
        numb = self.count_post
        self.count_post += 1
        return _query(numb, self.session.post, url, **kvargs)

    def __init__(self, *args, **kvargs):
        self.json_file = os.path.join(HOME,'Sync/Main/ip.json')
        if not os.path.exists(self.json_file):
            with open(self.json_file,'w') as f:
                f.write('{}')
        #self.cookies = requests.cookies.RequestsCookieJar()

    ### https://segmentfault.com/a/1190000000302955
    # #ifconfig.me #httpbin.org/ip
    #   curl -s http://ns1.dnspod.net:6666
    #   curl -s -I http://www.alibaba.com | grep ali_apache_id
    def httpbin_org(self, *args, hostname='-', **kvargs):
        rsp = self._GET('http://httpbin.org/ip')
        for _,x in rsp.json().items():
            self._add_ip(x, hostname)

        #times = int(times)
        #while times > 0 and not getattr(module,'STOP',None):
        #    times -= 1
    ### echo -e "GET / HTTP/1.0\r\n\r\n" |nc ns1.dnspod.net 6666 2>/dev/null
    def ns1_dnspod_net_6666(self, *args, hostname='-', **kvargs):
        sk = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sk.connect(('ns1.dnspod.net', 6666))
        sk.send(b'GET / HTTP/1.0\r\n\r\n')
        data = sk.recv(512)
        sk.close()
        self._add_ip(data.decode(), hostname)

    def _add_ip(self,ip,hostname):
        print(ip)
        dic = json.load( open(self.json_file) )
        ips = set( dic.setdefault(hostname,[]) )
        ips.add( ip )
        dic[hostname] = list(ips)
        json.dump(dic, open(self.json_file,'w'))

def help(*args, **kvargs):
    print('Usages:'
            '\t{0} X=<XValue> Y=<YValue> a1 yarg\n'
            '\t{0} X=<XValue> Y=<YValue> b1 xarg yarg\n'
            .format(sys.argv[0]))

#print('{0.filename}@{0.lineno}:'.format(inspect.getframeinfo(inspect.currentframe())))
def _main_():
    def _fn_lis_dic(args):
        fn, lis, dic = '', [], {} # defaultdict(list)
        for a in args:
            if a.startswith('-'):
                assert ( '=' in a )
                a = a.strip('-')
            if '=' in a:
                k,v = a.split('=',1)
                v0 = dic.setdefault(k,v)
                if v0 is not v:
                    if type(v0) == list:
                        v0.append(v)
                    else:
                        dic[k] = [v0, v]
            else:
                if not fn:
                    fn = a
                else:
                    lis.append(a)
        return fn, lis, dic
    def _fn(fn):
        f = getattr(module, fn, None)
        if not f:
            cls = getattr(module, 'Main', lambda *x,**y: None)
            f = getattr(cls(*args, **kvargs), fn, None)
        if not f:
            f = getattr(module, 'help', None)
        if not f:
            raise RuntimeError(fn, 'not found')
        return f
    t0 = time.time()
    fn, args, kvargs = _fn_lis_dic(sys.argv[1:])
    _fn(fn)(*args, **kvargs)
    sys.stdout.flush()
    print('time({}): {}m{}s'.format(fn, *map(int, divmod(time.time() - t0, 60))), file=sys.stderr)

if __name__ == '__main__':
    module = sys.modules[__name__]
    def _sig_handler(signal, frame):
        setattr(module, 'STOP', 1)
    try:
        import signal
        signal.signal(signal.SIGINT, _sig_handler)
        _main_()
    except Exception as e:
        help() #print(e, file=sys.stderr)
        raise

#>>> requests.get('http://httpbin.org/')
#<Response [200]>
#
#>>> debug_log_on()
#>>> requests.get('http://httpbin.org/')
#INFO:requests.packages.urllib3.connectionpool:Starting new HTTP connection (1): httpbin.org
#DEBUG:requests.packages.urllib3.connectionpool:"GET / HTTP/1.1" 200 12150
#send: 'GET / HTTP/1.1\r\nHost: httpbin.org\r\nConnection: keep-alive\r\nAccept-
#Encoding: gzip, deflate\r\nAccept: */*\r\nUser-Agent: python-requests/2.11.1\r\n\r\n'
#reply: 'HTTP/1.1 200 OK\r\n'
#header: Server: nginx
#...
#<Response [200]>
#
#>>> debug_requests_off()
#>>> requests.get('http://httpbin.org/')
#<Response [200]>
#
#>>> with debug_requests():
#...     requests.get('http://httpbin.org/')
#INFO:requests.packages.urllib3.connectionpool:Starting new HTTP connection (1): httpbin.org
#...
#<Response [200]>
#
#userdata = {"firstname": "John", "lastname": "Doe", "password": "jdoe123"}
#resp = requests.post('http://www.mywebsite.com/user', data=userdata)
#resp.json()

### http://stackoverflow.com/questions/2018026/what-are-the-differences-between-the-urllib-urllib2-and-requests-module?rq=1I

