from __future__ import print_function

import os, sys, time
from itertools import islice

class ParallelListReader(object):
    def __init__(self, filename, chunksize=1, timeout_secs=None, retry_delay=5):
        self._filename = filename
        self._chunksize = chunksize
        self._cache = []
        self._retry_delay = retry_delay

        self._timeout = timeout_secs
        if self._timeout:
            self._tstart = time.time()

    def __iter__(self):
        return self

    def next(self):
        if not self._cache:
            self._load()
            if not self._cache:
                raise StopIteration
        return self._cache.pop(0).strip()

    def _load(self):
        if self._timeout:
            delta = time.time() - self._tstart
            if delta > self._timeout:
                print('Terminating due to specified timeout')
                raise StopIteration

        print('Grabbing input list lock')
        os.system('time lockfile -%d %s.lock' % (self._retry_delay, self._filename))
        with open(self._filename) as f:
            self._cache = list(islice(f, self._chunksize))
            rest = list(f)
        open(self._filename, 'w').writelines(rest)
        os.system('rm -f %s.lock' % self._filename)

class ParallelListWriter(object):
    def __init__(self, filename, chunksize=1, retry_delay=5):
        self._filename = filename
        self._chunksize = chunksize
        self._cache = []
        self._retry_delay = retry_delay

    def put(self, line):
        self._cache.append(line)
        if len(self._cache) == self._chunksize:
            self._flush()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        if self._cache:
            self._flush()

    def _flush(self):
        chunk = '\n'.join(self._cache) + '\n'
        os.system('time lockfile -%d %s.lock' % (self._retry_delay, self._filename))
        with open(self._filename, 'a') as f:
            f.write(chunk)
        os.system('rm -f %s.lock' % self._filename)
        self._cache = []

class DoneLogger(ParallelListWriter):
    def log(self, path):
        tstamp = time.strftime('%Y-%m-%dT%H:%M:%S')
        self.put(tstamp + ' ' + path)

def parse_path(path):
    fields = os.path.basename(path).split('.')
    runno, fileno, site = int(fields[2]), int(fields[6][1:]), int(fields[4][2])
    return runno, fileno, site

def sysload():
    print('BEGIN-SYSLOAD')
    os.system('date; top ibn1; free -g')
    print('END-SYSLOAD')

def unbuf_stdout():
    sys.stdout = os.fdopen(sys.stdout.fileno(), 'w', 0)
