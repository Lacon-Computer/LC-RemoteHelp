#!/usr/bin/env python
# -*- coding: utf-8 -*-
#############################################################################
# Copyright (c) 2015, Jeff Kent <jeff@jkent.net>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#############################################################################

import ctypes
import logging
from optparse import OptionParser
from random import randrange
import select
import socket
import struct
import sys
from threading import RLock, Thread


ORGANIZATION_NAME = u'Lacon Computer'
SERVER_PORT = 5499
VIEWER_PORT = 5899
MIN_SESSION_ID = 1
MAX_SESSION_ID = 999999999
READ_SIZE = 4096


def main():
    parser = OptionParser()
    parser.add_option('-l', '--log-level', default='error')
    options, args = parser.parse_args()

    logging.basicConfig(stream=sys.stderr, level=options.log_level.upper(),
                        format='%(threadName)s %(message)s')

    server_listener = Listener(ServerThread, ('0.0.0.0', SERVER_PORT))
    viewer_listener = Listener(ViewerThread, ('0.0.0.0', VIEWER_PORT))

    try:
        while True:
            rs = (server_listener, viewer_listener)
            rs, ws, es = select.select(rs, [], [], 1)
            for r in rs:
                r.on_read()
    except KeyboardInterrupt:
        pass

    server_listener.shutdown()
    viewer_listener.shutdown()

    with BaseThread.pool_lock:
        for thread in BaseThread.pool:
            thread.shutdown()


class Listener(object):
    def __init__(self, thread_type, addr):
        self.thread_type = thread_type
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(addr)
        self.sock.listen(5)

    def fileno(self):
        return self.sock.fileno()

    def shutdown(self):
        self.sock.close()

    def on_read(self):
        sock, addr = self.sock.accept()
        thread = self.thread_type(sock, addr)
        thread.start()


class SocketClosedException(Exception):
    pass


class BaseThread(Thread):
    pool = []
    pool_lock = RLock()

    @classmethod
    def add_pool(cls, thread):
        with cls.pool_lock:
            cls.pool.append(thread)

    @classmethod
    def remove_pool(cls, thread):
        with cls.pool_lock:
            cls.pool.remove(thread)

    def __init__(self, sock, addr):
        Thread.__init__(self)
        self.name = '%s(%s)' % (self.__class__.__name__[:-6], self.name[7:])
        self.sock = sock
        self.addr = addr
        self.pair = None

    def run(self):
        logging.info('connection from %s', self.addr[0])
        BaseThread.add_pool(self)
        try:
            self.run_safe()
        except Exception as e:
            self.shutdown()
            if type(e) != SocketClosedException:
                logging.warning(str(e))
        BaseThread.remove_pool(self)
        logging.info('closed')

    def shutdown(self):
        if not self.sock:
            return

        if self.pair:
            self.pair.pair = None
            self.pair.shutdown()
            self.pair = None

        sock = self.sock
        self.sock = None
        try:
            sock.shutdown(socket.SHUT_RDWR)
            sock.close()
        except:
            pass

    def read(self, count):
        data = self.sock.recv(count)
        if not data:
            raise SocketClosedException
        return data

    def readFully(self, toread):
        buf = bytearray(toread)
        view = memoryview(buf)
        while toread:
            nbytes = self.sock.recv_into(view, toread)
            if not nbytes:
                raise SocketClosedException
            view = view[nbytes:]
            toread -= nbytes
        return buf

    def readUInt8(self):
        data = self.readFully(1)
        return struct.unpack('!B', data)[0]

    def readUInt16(self):
        data = self.readFully(2)
        return struct.unpack('!H', data)[0]

    def readUInt32(self):
        data = self.readFully(4)
        return struct.unpack('!I', data)[0]

    def readUInt64(self):
        data = self.readFully(8)
        return struct.unpack('!Q', data)[0]

    def readInt8(self):
        data = self.readFully(1)
        return struct.unpack('!b', data)[0]

    def readInt16(self):
        data = self.readFully(2)
        return struct.unpack('!h', data)[0]

    def readInt32(self):
        data = self.readFully(4)
        return struct.unpack('!i', data)[0]

    def readInt64(self):
        data = self.readFully(8)
        return struct.unpack('!q', data)[0]

    def readUTF8(self):
        nbytes = self.readUInt32()
        if nbytes > 1024:
            raise Exception('utf-8 string too long')
        s = u''
        if nbytes > 0:
            data = self.readFully(nbytes)
            s = data.decode('utf-8', 'ignore')[:-1]
        return s

    def write(self, data):
        self.sock.send(data)

    def writeFully(self, data):
        self.sock.sendall(data)

    def writeUInt8(self, n):
        data = struct.pack('!B', ctypes.c_uint8(n).value)
        self.writeFully(data)

    def writeUInt16(self, n):
        data = struct.pack('!H', ctypes.c_uint16(n).value)
        self.writeFully(data)

    def writeUInt32(self, n):
        data = struct.pack('!I', ctypes.c_uint32(n).value)
        self.writeFully(data)

    def writeUInt64(self, n):
        data = struct.pack('!Q', ctypes.c_uint64(n).value)
        self.writeFully(data)

    def writeInt8(self, n):
        data = struct.pack('!b', ctypes.c_int8(n).value)
        self.writeFully(data)

    def writeInt16(self, n):
        data = struct.pack('!h', ctypes.c_int16(n).value)
        self.writeFully(data)

    def writeInt32(self, n):
        data = struct.pack('!i', ctypes.c_int32(n).value)
        self.writeFully(data)

    def writeInt64(self, n):
        data = struct.pack('!q', ctypes.c_int64(n).value)
        self.writeFully(data)

    def writeUTF8(self, s):
        data = s.encode('utf-8') + '\0'
        self.writeUInt32(len(data))
        self.writeFully(data)


class ServerThread(BaseThread):
    sessions = {}
    sessions_lock = RLock()

    def __init__(self, *args, **kwargs):
        super(ServerThread, self).__init__(*args, **kwargs)
        self.session_id = 0
        self.readbuf = ''
        self.readbuf_lock = RLock()

    def run_safe(self):
        self.session_phase()
        self.wait_for_paring()
        self.paired_phase()

    def shutdown(self):
        super(ServerThread, self).shutdown()
        with ServerThread.sessions_lock:
            if ServerThread.sessions.has_key(self.session_id):
                del ServerThread.sessions[self.session_id]

    def session_phase(self):
        self.writeUTF8(ORGANIZATION_NAME)

        session_id = self.readUInt32()
        logging.debug('requested session id %09d', session_id)

        with ServerThread.sessions_lock:
            while session_id < MIN_SESSION_ID or \
                  session_id > MAX_SESSION_ID or \
                  ServerThread.sessions.has_key(session_id):
                session_id = randrange(MIN_SESSION_ID, MAX_SESSION_ID + 1)
            ServerThread.sessions[session_id] = self

        self.session_id = session_id
        self.writeUInt32(session_id)
        logging.info('assigned session id %09d', session_id)

    def wait_for_paring(self):
        while not self.pair:
            data = self.read(READ_SIZE)
            with self.readbuf_lock:
                self.readbuf += data
                if len(self.readbuf) > 1024:
                    raise Exception('readbuf overflow')

    def paired_phase(self):
        with self.readbuf_lock:
            self.pair.writeFully(self.readbuf)
            self.readbuf = ''

        while self.pair:
            data = self.read(READ_SIZE)
            self.pair.writeFully(data)


class ViewerThread(BaseThread):
    def run_safe(self):
        self.session_phase()
        self.paired_phase()

    def session_phase(self):
        contact_name = self.readUTF8()
        logging.debug('got contact name "%s"', contact_name)

        while not self.pair:
            requested_session_id = self.readUInt32()
            logging.debug('requested session id %09d', requested_session_id)
            with ServerThread.sessions_lock:
                for session_id, thread in ServerThread.sessions.iteritems():
                    if not thread.pair and session_id == requested_session_id:
                        self.pair = thread
                        thread.pair = self
                        break
            self.writeUInt8(1 if self.pair else 0)

        self.pair.writeUTF8(contact_name)
        logging.info('paired with %s', self.pair.name)

    def paired_phase(self):
        with self.pair.readbuf_lock:
            self.writeFully(self.pair.readbuf)
            self.pair.readbuf = ''

        while True:
            data = self.read(READ_SIZE)
            self.pair.writeFully(data)


if __name__ == '__main__':
    main()
