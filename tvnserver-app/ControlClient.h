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

#ifndef _CONTROL_CLIENT_H_
#define _CONTROL_CLIENT_H_

#include "RfbClientManager.h"

#include "tvncontrol-app/ControlGate.h"
#include "tvncontrol-app/Transport.h"
#include "TcpDispatcherConnectionListener.h"
#include "thread/ThreadCollector.h"
#include "log-writer/LogWriter.h"

/**
 * ControlClient exception sclass.
 *
 * Solves problem with catching errors that occured when processing
 * control client message (not IO errors).
 */
class ControlException : public Exception
{
public:
  ControlException(const Exception *parent) : Exception(parent->getMessage()) { }
  ControlException(const TCHAR *message) : Exception(message) { }
  virtual ~ControlException() { };
};

/**
 * Handler of control client connections.
 *
 * @remark runs in it's own thread.
 *
 * @remark class uses TvnServer singleton and actual TvnServer instance must exist
 * while any control client is runnin. If this rule is broken, it can cause application crash.
 */
class ControlClient : public Thread,
                      private TcpDispatcherConnectionListener
{
public:
  /**
   * Creates new control client handler thread and starts it.
   * @param transport transport of incoming control connection (non-authorized).
   * @param rfbClientManager active TightVNC rfb client manager.
   * @remark control client takes ownership over client transport.
   */
  ControlClient(Transport *transport,
                RfbClientManager *rfbClientManager,
                HANDLE pipeHandle,
                LogWriter *log);
  /**
   * Stops client thread and deletes control client.
   */
  virtual ~ControlClient();

protected:
  /**
   * Inherited from Thread class.
   * Processed control client connection, consists of following phases:
   *   Auth phase (send auth type and try auth client).
   *   Control message processing loop.
   */
  virtual void execute();

  /**
   * Inherited from Thread class.
   *
   * Closes transport.
   */
  virtual void onTerminate();

private:
  virtual void onGetId(unsigned int id,
                       const AnsiStringStorage *dispatcherName);
  virtual void onClearId();

  /**
   * Sends error (server exception message) to client side.
   * @param message description of error.
   * @throws IOException on io error.
   */
  void sendError(const TCHAR *message) throw(IOException);

  /**
   * Handlers of control proto messages.
   */

  /**
   * Called when get client list message recieved.
   * @throws IOException on io error.
   */
  void getClientsListMsgRcvd() throw(IOException);
  /**
   * Called when get server info message reciveved.
   * @throws IOException on io error.
   */
  void getServerInfoMsgRcvd() throw(IOException);
  /**
   * Called when reload configuration message recieved.
   * @throws IOException on io error.
   * @deprecated.
   */
  void reloadConfigMsgRcvd() throw(IOException);
  /**
   * Called when disconnect all clients message recieved.
   * @throws IOException on io error.
   */
  void shutdownMsgRcvd() throw(IOException);
  /**
   * Called when add new client message recieved.
   * @throws IOException on io error.
   */
  void setServerConfigMsgRcvd() throw(IOException);
  /**
   * Called when get server config message recieved.
   * @throws IOException on io error.
   */
  void getServerConfigMsgRcvd() throw(IOException);
  /**
   * Called when "get show tray icon flag" message recieved.
   * @throws IOException on io error.
   */
  void getShowTrayIconFlagMsgRcvd() throw(IOException);
  /**
   * Called when "update tvncontrol process id" message recieved.
   * @throws IOException on io error.
   */
  void updateTvnControlProcessIdMsgRcvd() throw(IOException);

private:
  /**
   * Client transport.
   */
  Transport *m_transport;
  /**
   * Low-level transport for writting and recieving bytes.
   */
  Channel *m_stream;

  /**
   * High-level transport for writting and reading control proto messages.
   */
  ControlGate *m_gate;
  HANDLE m_pipeHandle;

  /**
   * Active TightVNC rfb client manager.
   */
  RfbClientManager *m_rfbClientManager;

  // A connection identifier will be used by a viewer to connect to
  // the server across a tcp dispatcher.
  unsigned int m_tcpDispId;
  StringStorage m_gotDispatcherName;
  LocalMutex m_tcpDispValuesMutex;

  ThreadCollector m_connectingSocketThreadCollector;

  LogWriter *m_log;
};

#endif
