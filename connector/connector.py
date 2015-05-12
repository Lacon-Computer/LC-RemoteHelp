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
import select
import sys
import time

from viewer import ViewerListener
from server import ServerListener


#LOG_LEVEL   = logging.DEBUG
LOG_LEVEL   = logging.WARNING
TICK_PERIOD = 5
VIEWER_PORT = 5899
SERVER_PORT = 5499


def main():
    logging.basicConfig(stream=sys.stderr, level=LOG_LEVEL)

    selectable_list = []

    viewer_addr = ('0.0.0.0', VIEWER_PORT)
    viewer_listener = ViewerListener(selectable_list, viewer_addr)
    selectable_list.append(viewer_listener)

    server_addr = ('0.0.0.0', SERVER_PORT)
    server_listener = ServerListener(selectable_list, server_addr)
    selectable_list.append(server_listener)

    last_tick = time.time()
    while True:
        rs, ws, es = [], [], []

        for selectable in selectable_list:
            if selectable.can_read():
                rs.append(selectable)
            if selectable.can_write():
                ws.append(selectable)
            es.append(selectable)

        rs, ws, es = select.select(rs, ws, es, TICK_PERIOD)

        now = time.time()
        if last_tick + TICK_PERIOD <= now:
            last_tick = now
            for selectable in selectable_list:
                selectable.on_tick(now)

        for r in rs:
            r.on_read()

        for w in ws:
            w.on_write()

        for e in es:
            e.on_except()


if __name__ == '__main__':
    main()
