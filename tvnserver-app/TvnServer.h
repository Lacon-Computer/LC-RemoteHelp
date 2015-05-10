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

#ifndef _TVN_SERVER_H_
#define _TVN_SERVER_H_

#include "util/CommonHeader.h"

#include "desktop/ApplicationDesktopFactory.h"
#include "RfbClientManager.h"
#include "ControlServer.h"
#include "TvnServerListener.h"

#include "thread/ZombieKiller.h"
#include "thread/LocalMutex.h"
#include "log-writer/LogWriter.h"
#include "util/Singleton.h"
#include "util/ListenerContainer.h"

#include "server-config-lib/Configurator.h"

#include "tvncontrol-app/TvnServerInfo.h"
#include "LogInitListener.h"

/**
 * TightVNC server singleton that includes serveral components:
 *   1) Zombie killer singleton.
 *   2) Configurator singleton.
 *   3) Log singleton.
 *   4) Control server.
 *   5) Other features:
     1) Do action when last client disconnects.
 */
class TvnServer : public Singleton<TvnServer>,
                  public ListenerContainer<TvnServerListener *>,
                  public ConfigReloadListener,
                  public RfbClientManagerEventListener
{
public:
  /**
   * Creates and starts TightVNC server execution (in separate thread).
   *
   * Makes sereval steps:
   *  1) Instanizes zombie killer.
   *  2) Instanizes configurator and load configuration.
   *  3) Instanizes log.
   *  4) Starts all servers.
   *
   * @remark doesn't block calling thread execution cause all servers runs in it's own threads.
   * To know when need to shutdown TightVNC server you need to use addListener method.
   */
  TvnServer(LogInitListener *logInitListener,
            Logger *logger);
  /**
   * Stops and destroys TightVNC server.
   * @remark don't generate shutdown signal(like shutdown() method does) for listeners.
   */
  virtual ~TvnServer();

  /**
   * Fills structure with information of current state of TvnServer.
   * @param info [out] output parameter that will contain TightVNC server information
   * after call of this method.
   * @fixme place extended information to server info.
   */
  void getServerInfo(TvnServerInfo *info);

  /**
   * Inherited from ConfigReloadListener interface to catch configuration reload event.
   *
   * Make several things:
   *  1) Changes log level.
   */
  virtual void onConfigReload(ServerConfig *serverConfig);

  /**
   * Only generates shutdown signal (event) for TvnServer listeners.
   *
   * @remark used by ControlClient, when it recieves command to shutdown TightVNC.
   * @remark doesn't stop TightVNC server.
   * @fixme rename this method.
   */
  void generateExternalShutdownSignal();

  /**
   * Implemented from RfbClientManagerEventListener.
   *
   * Does nothing.
   */
  virtual void afterFirstClientConnect();

  /**
   * Implemented from RfbClientManagerEventListener.
   *
   * Does specifed in configuration action when last client disconnects from
   * rfb server.
   */
  virtual void afterLastClientDisconnect();

protected:
  void restartControlServer();

  void stopControlServer();

  // Calls a callback function to change update log properties.
  void changeLogProps();

protected:
  LogWriter m_log;
  ZombieKiller m_zombieKiller;

  Configurator m_config;
  /**
   * Shortcut to global server configuration.
   */
  ServerConfig *m_srvConfig;

  /**
   * Mutex for protecting servers.
   */
  LocalMutex m_mutex;

  ApplicationDesktopFactory m_applicationDesktopFactory;
  /**
   * Rfb client manager (for all rfb servers), used by rfb servers
   * rfb clients, control server and control clients.
   */
  RfbClientManager *m_rfbClientManager;
  /**
   * Control server.
   */
  ControlServer *m_controlServer;

  LogInitListener *m_logInitListener;
};

#endif
