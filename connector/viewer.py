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

import logging
import socket
import struct

from selectable import Selectable


class ViewerListener(Selectable):
    def __init__(self, selectable_list, addr):
        self.selectable_list = selectable_list
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(addr)
        self.sock.listen(5)

    def fileno(self):
        return self.sock.fileno()

    def on_read(self):
        sock, addr = self.sock.accept()
        ViewerClient(self.selectable_list, sock)

class ViewerClient(Selectable):
    def __init__(self, selectable_list, sock):
        self.selectable_list = selectable_list
        self.selectable_list.append(self)
        self.sock = sock
        self.rdbuf = ''
        self.wrbuf = ''
        self.other = None

        logging.info("viewer(%d) connected", self.fileno())

    def fileno(self):
        return self.sock.fileno()

    def can_write(self):
        return bool(self.wrbuf)

    def on_read(self):
        if not self.other:
            self.pair_up()
            return

        try:
            data = self.sock.recv(4096)
        except:
            data = ''

        if self.other:
            if self.rdbuf:
                self.other.wrbuf += self.rdbuf
                self.rdbuf = ''
            self.other.wrbuf += data
        else:
            self.rdbuf += data

        if not data:
            self.on_close()

    def on_write(self):
        data, self.wrbuf = self.wrbuf[:4096], self.wrbuf[4096:]
        try:
            self.sock.send(data)
        except:
            pass

    def on_close(self):
        logging.info("viewer(%d) disconnected", self.fileno())
        if self.other:
            self.other.other = None
            self.other.on_close()
        self.wrbuf = ''
        self.other = None
        self.selectable_list.remove(self)
        self.sock.close()

    def pair_up(self):
        try:
            data = self.sock.recv(4)
        except:
            data = ''

        if not data:
            self.on_close()
            return

        _id = struct.unpack('!I', data)[0]
        logging.debug("viewer(%d) requested id %09d", self.fileno(), _id)

        for selectable in self.selectable_list:
            if not selectable.__class__.__name__ == 'ServerClient':
                continue
            if selectable.other:
                continue
            if selectable._id == _id:
                selectable.other = self
                self.other = selectable

        result = 1 if self.other else 0
        self.wrbuf += struct.pack('!I', result)

        if self.other:
            self.other.wrbuf += self.rdbuf
            self.rdbuf = ''
            self.wrbuf += self.other.rdbuf
            self.other.rdbuf = ''
            logging.info("viewer(%d) paired with server(%d)", self.fileno(),
                         selectable.fileno())
