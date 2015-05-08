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

#include "ControlClient.h"
#include "TvnServer.h"
#include "OutgoingRfbConnectionThread.h"

#include "tvncontrol-app/ControlProto.h"

#include "network/socket/SocketStream.h"

#include "server-config-lib/Configurator.h"

#include "util/VncPassCrypt.h"

#include "rfb/HostPath.h"

#include "win-system/WTS.h"

#include "tvnserver/resource.h"

#include <time.h>
#include "util/AnsiStringStorage.h"

ControlClient::ControlClient(Transport *transport,
                             RfbClientManager *rfbClientManager,
                             HANDLE pipeHandle, LogWriter *log)
: m_transport(transport), m_rfbClientManager(rfbClientManager),
  m_tcpDispId(0),
  m_pipeHandle(pipeHandle),
  m_log(log)
{
  m_stream = m_transport->getIOStream();

  m_gate = new ControlGate(m_stream);
}

ControlClient::~ControlClient()
{
  terminate();
  wait();

  delete m_gate;
  delete m_transport;
}

void ControlClient::execute()
{
  try {
    while (!isTerminating()) {
      UINT32 messageId = m_gate->readUInt32();
      UINT32 messageSize = m_gate->readUInt32();

      m_log->detail(_T("Recieved control message ID %u, size %u"),
                  (unsigned int)messageId, (unsigned int)messageSize);

      try {
        switch (messageId) {
        case ControlProto::RELOAD_CONFIG_MSG_ID:
          m_log->detail(_T("Command requested: Reload configuration"));
          reloadConfigMsgRcvd();
          break;
        case ControlProto::DISCONNECT_ALL_CLIENTS_MSG_ID:
          m_log->detail(_T("Command requested: Disconnect all clients command requested"));
          disconnectAllMsgRcvd();
          break;
        case ControlProto::SHUTDOWN_SERVER_MSG_ID:
          m_log->detail(_T("Command requested: Shutdown command requested"));
          shutdownMsgRcvd();
          break;
        case ControlProto::ADD_CLIENT_MSG_ID:
          m_log->detail(_T("Command requested: Attach listening viewer"));
          addClientMsgRcvd();
          break;
        case ControlProto::GET_SERVER_INFO_MSG_ID:
          m_log->detail(_T("Control client requests server info"));
          getServerInfoMsgRcvd();
          break;
        case ControlProto::GET_CLIENT_LIST_MSG_ID:
          m_log->detail(_T("Control client requests client list"));
          getClientsListMsgRcvd();
          break;
        case ControlProto::SET_CONFIG_MSG_ID:
          m_log->detail(_T("Control client sends new server config"));
          setServerConfigMsgRcvd();
          break;
        case ControlProto::GET_CONFIG_MSG_ID:
          m_log->detail(_T("Control client requests server config"));
          getServerConfigMsgRcvd();
          break;
        case ControlProto::GET_SHOW_TRAY_ICON_FLAG:
          m_log->detail(_T("Control client requests tray icon visibility flag"));
          getShowTrayIconFlagMsgRcvd();
          break;
        case ControlProto::UPDATE_TVNCONTROL_PROCESS_ID_MSG_ID:
          m_log->detail(_T("Control client sends process ID"));
          updateTvnControlProcessIdMsgRcvd();
          break;
        default:
          m_gate->skipBytes(messageSize);
          m_log->warning(_T("Received unsupported message from control client"));
          throw ControlException(_T("Unknown command"));
        } // switch (messageId).
      } catch (ControlException &controlEx) {
        m_log->error(_T("Exception while processing control client's request: \"%s\""),
                   controlEx.getMessage());

        sendError(controlEx.getMessage());
      }
    } // while
  } catch (Exception &ex) {
    m_log->error(_T("Exception in control client thread: \"%s\""), ex.getMessage());
  }
}

void ControlClient::onTerminate()
{
  try { m_transport->close(); } catch (...) { }
}

void ControlClient::sendError(const TCHAR *message)
{
  m_gate->writeUInt32(ControlProto::REPLY_ERROR);
  m_gate->writeUTF8(message);
}

void ControlClient::getClientsListMsgRcvd()
{
  UINT32 clientCount = 0;

  RfbClientInfoList clients;

  m_rfbClientManager->getClientsInfo(&clients);

  m_gate->writeUInt32(ControlProto::REPLY_OK);
  _ASSERT(clients.size() == (unsigned int)clients.size());
  m_gate->writeUInt32((unsigned int)clients.size());

  for (RfbClientInfoList::iterator it = clients.begin(); it != clients.end(); it++) {
    m_gate->writeUInt32((*it).m_id);
    m_gate->writeUTF8((*it).m_peerAddr.getString());
  }
}

void ControlClient::getServerInfoMsgRcvd()
{
  bool acceptFlag = false;

  StringStorage logPath;
  StringStorage statusText;

  TvnServerInfo info;

  TvnServer::getInstance()->getServerInfo(&info);

  StringStorage status;
  {
    AutoLock al(&m_tcpDispValuesMutex);
    if (m_tcpDispId != 0) {
      status.format(_T("ID = %u; Dispatcher Name = %s; %s"),
                    m_tcpDispId,
                    m_gotDispatcherName.getString(),
                    info.m_statusText.getString());
    } else {
      status.setString(info.m_statusText.getString());
    }
  }

  m_gate->writeUInt32(ControlProto::REPLY_OK);

  m_gate->writeUTF8(status.getString());
}

void ControlClient::reloadConfigMsgRcvd()
{
  m_gate->writeUInt32(ControlProto::REPLY_OK);

  Configurator::getInstance()->load();
}

void ControlClient::disconnectAllMsgRcvd()
{
  m_gate->writeUInt32(ControlProto::REPLY_OK);

  m_rfbClientManager->disconnectAllClients();
  m_connectingSocketThreadCollector.destroyAllThreads();
}

void ControlClient::shutdownMsgRcvd()
{
  m_gate->writeUInt32(ControlProto::REPLY_OK);

  TvnServer::getInstance()->generateExternalShutdownSignal();
}

void ControlClient::addClientMsgRcvd()
{
  m_gate->writeUInt32(ControlProto::REPLY_OK);

  //
  // Read parameters.
  //

  StringStorage connectString;

  m_gate->readUTF8(&connectString);

  bool viewOnly = m_gate->readUInt8() == 1;

  //
  // Parse host and port from connection string.
  //
  AnsiStringStorage connectStringAnsi(&connectString);
  HostPath hp(connectStringAnsi.getString(), 5499);

  if (!hp.isValid()) {
    return;
  }

  StringStorage host;
  AnsiStringStorage ansiHost(hp.getVncHost());
  ansiHost.toStringStorage(&host);

  //
  // Make outgoing connection in separate thread.
  //
  OutgoingRfbConnectionThread *newConnectionThread =
                               new OutgoingRfbConnectionThread(host.getString(),
                                                               hp.getVncPort(), viewOnly,
                                                               m_rfbClientManager, m_log);

  newConnectionThread->resume();

  ZombieKiller::getInstance()->addZombie(newConnectionThread);
}

void ControlClient::setServerConfigMsgRcvd()
{
  m_gate->writeUInt32(ControlProto::REPLY_OK);

  Configurator::getInstance()->getServerConfig()->deserialize(m_gate);
  Configurator::getInstance()->save();
  Configurator::getInstance()->load();
}

void ControlClient::getShowTrayIconFlagMsgRcvd()
{
  bool showIcon = Configurator::getInstance()->getServerConfig()->getShowTrayIconFlag();

  m_gate->writeUInt32(ControlProto::REPLY_OK);

  m_gate->writeUInt8(showIcon ? 1 : 0);
}

void ControlClient::updateTvnControlProcessIdMsgRcvd()
{
  m_gate->readUInt32();

  try {
    WTS::duplicatePipeClientToken(m_pipeHandle);
  } catch (Exception &e) {
    m_log->error(_T("Can't update the control client impersonation token: %s"),
               e.getMessage());
  }
  m_gate->writeUInt32(ControlProto::REPLY_OK);
}

void ControlClient::getServerConfigMsgRcvd()
{
  m_gate->writeUInt32(ControlProto::REPLY_OK);

  Configurator::getInstance()->getServerConfig()->serialize(m_gate);
}

void ControlClient::onGetId(unsigned int id,
                            const AnsiStringStorage *dispatcherName)
{
  AutoLock al(&m_tcpDispValuesMutex);
  m_tcpDispId = id;
  dispatcherName->toStringStorage(&m_gotDispatcherName);
}

void ControlClient::onClearId()
{
  AutoLock al(&m_tcpDispValuesMutex);
  m_tcpDispId = 0;
  m_gotDispatcherName.setString(_T(""));
}
