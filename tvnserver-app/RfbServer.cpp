// Copyright (C) 2009,2010,2011,2012 GlavSoft LLC.
// All rights reserved.
//
//-------------------------------------------------------------------------
// This file is part of the TightVNC software.  Please visit our Web site:
//
//                       http://www.tightvnc.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//-------------------------------------------------------------------------
//

#include "RfbServer.h"
#include "server-config-lib/Configurator.h"

RfbServer::RfbServer(const TCHAR *bindHost, unsigned short bindPort,
                     RfbClientManager *clientManager,
                     bool lockAddr,
                     LogWriter *log,
                     const Rect *viewPort)
: TcpServer(bindHost, bindPort, false, lockAddr),
  m_clientManager(clientManager),
  m_log(log)
{
  if (viewPort != 0) {
    m_viewPort.setArbitraryRect(viewPort);
  }

  TcpServer::start();

  if (viewPort == 0) {
    m_log->message(_T("Rfb server started at %s:%d"), bindHost, (int)bindPort);
  } else {
    m_log->message(_T("Rfb server started at %s:%d with [%d %d %d %d] view port specified"),
                 bindHost, (int)bindPort,
                 viewPort->left, viewPort->right, viewPort->top, viewPort->bottom);
  }
}

RfbServer::~RfbServer()
{
  m_log->message(_T("Rfb server at %s:%d stopped"), getBindHost(), (int)getBindPort());
}

void RfbServer::onAcceptConnection(SocketIPv4 *socket)
{
  try {
    // Get incoming connection address and convert it to string.
    SocketAddressIPv4 peerAddr;
    socket->getPeerAddr(&peerAddr);
    StringStorage peerIpString;
    peerAddr.toString(&peerIpString);

    m_log->message(_T("Incoming rfb connection from %s"), peerIpString.getString());

    socket->enableNaggleAlgorithm(false);

    m_clientManager->addNewConnection(socket, &m_viewPort, false, false);

  } catch (Exception &ex) {
    m_log->error(_T("Failed to process incoming rfb connection with following reason: \"%s\""), ex.getMessage());
  }
}
