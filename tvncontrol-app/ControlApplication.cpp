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

#include "ControlApplication.h"
#include "ControlTrayIcon.h"
#include "ControlTrayIcon.h"
#include "ControlPipeName.h"
#include "ControlCommand.h"
#include "ReloadConfigCommand.h"
#include "ShutdownCommand.h"

#include "util/winhdr.h"
#include "util/StringTable.h"
#include "tvnserver-app/NamingDefs.h"

#include "tvncontrol-app/ControlCommandLine.h"
#include "tvnserver-app/TvnServerHelp.h"

#include "win-system/Environment.h"
#include "win-system/Shell.h"
#include "win-system/Process.h"
#include "win-system/WinCommandLineArgs.h"

#include "thread/ZombieKiller.h"
#include "thread/GlobalMutex.h"

#include "gui/CommonControlsEx.h"

#include "network/socket/WindowsSocket.h"

#include "tvnserver/resource.h"

#include "util/AnsiStringStorage.h"
#include "tvnserver-app/NamingDefs.h"

ControlApplication::ControlApplication(HINSTANCE hinst,
                                       const TCHAR *windowClassName,
                                       const TCHAR *commandLine)
 : WindowsApplication(hinst, windowClassName),
   m_serverControl(0),
   m_gate(0),
   m_transport(0),
   m_trayIcon(0),
   m_slaveModeEnabled(false),
   m_configurator(),
   m_log(0)
{
  m_commandLine.setString(commandLine);

  CommonControlsEx::init();

  WindowsSocket::startup(2, 1);
}

ControlApplication::~ControlApplication()
{
  try {
    WindowsSocket::cleanup();
  } catch (...) { }

  if (m_serverControl != 0) {
    delete m_serverControl;
  }
  if (m_gate != 0) {
    delete m_gate;
  }
  if (m_transport != 0) {
    delete m_transport;
  }
}

int ControlApplication::run()
{
  ControlCommandLine cmdLineParser;

  // Check command line for valid.
  try {
    WinCommandLineArgs cmdArgs(m_commandLine.getString());
    cmdLineParser.parse(&cmdArgs);
  } catch (CommandLineFormatException &) {
    TvnServerHelp::showUsage();
    return 1;
  }

  int retCode = 0;

  ZombieKiller zombieKiller;

  // Connect to server.
  try {
    connect(cmdLineParser.isSlave());
  } catch (Exception &) {
    if (!cmdLineParser.isSlave()) {
      const TCHAR *msg = StringTable::getString(IDS_FAILED_TO_CONNECT_TO_CONTROL_SERVER);
      const TCHAR *caption = StringTable::getString(IDS_MBC_TVNCONTROL);
      MessageBox(0, msg, caption, MB_OK | MB_ICONERROR);
    }
    return 1;
  }

  // Execute command (if specified) and exit.
  if (cmdLineParser.isCommandSpecified()) {
    Command *command = 0;

    if (cmdLineParser.hasReloadFlag()) {
      command = new ReloadConfigCommand(m_serverControl);
    } else if (cmdLineParser.hasShutdownFlag()) {
      command = new ShutdownCommand(m_serverControl);
    }

    retCode = runControlCommand(command);

    if (command != 0) {
      delete command;
    }
  } else {
    bool showIcon = true;

    if (cmdLineParser.isSlave()) {
      m_slaveModeEnabled = true;
      try {
        try {
          showIcon = m_serverControl->getShowTrayIconFlag();
        } catch (RemoteException &remEx) {
          notifyServerSideException(remEx.getMessage());
        }
        try {
          m_serverControl->updateTvnControlProcessId(GetCurrentProcessId());
        } catch (RemoteException &remEx) {
          notifyServerSideException(remEx.getMessage());
        }
      } catch (IOException &) {
        notifyConnectionLost();
        return 1;
      } catch (Exception &) {
        _ASSERT(FALSE);
      }
    }

    retCode = runControlInterface(showIcon);
  }

  return retCode;
}

void ControlApplication::connect(bool slave)
{
  // Determine the name of pipe to connect to.
  StringStorage pipeName;
  ControlPipeName::createPipeName(&pipeName, &m_log);

  int numTriesRemaining = slave ? 10 : 1;
  int msDelayBetweenTries = 2000;

  while (numTriesRemaining-- > 0) {
    try {
      m_transport = TransportFactory::createPipeClientTransport(pipeName.getString());
      break;
    } catch (Exception &) {
      if (numTriesRemaining <= 0) {
        throw;
      }
    }
    Sleep(msDelayBetweenTries);
  }

  // We can get here only on successful connection.
  m_gate = new ControlGate(m_transport->getIOStream());
  m_serverControl = new ControlProxy(m_gate);
}

void ControlApplication::notifyServerSideException(const TCHAR *reason)
{
  StringStorage message;

  message.format(StringTable::getString(IDS_CONTROL_SERVER_RAISE_EXCEPTION), reason);

  MessageBox(0, message.getString(), StringTable::getString(IDS_MBC_TVNSERVER), MB_OK | MB_ICONERROR);
}

void ControlApplication::notifyConnectionLost()
{
  MessageBox(0,
             StringTable::getString(IDS_CONTROL_CONNECTION_LOST),
             StringTable::getString(IDS_MBC_TVNCONTROL),
             MB_OK | MB_ICONEXCLAMATION);
}

void ControlApplication::execute()
{
  try {
    while (!isTerminating()) {
      Thread::sleep(500);
      // If we need to show or hide icon.
      bool showIcon = m_serverControl->getShowTrayIconFlag() || !m_slaveModeEnabled;

      // Check if we need to show icon.
      if (showIcon && !m_trayIcon->isVisible()) {
        m_trayIcon->show();
      }
      // Check if we need to hide icon.
      if (m_trayIcon != 0 && !showIcon) {
        m_trayIcon->hide();
      }
      // Update tray icon status if icon is set.
      if (m_trayIcon->isVisible()) {
        m_trayIcon->syncStatusWithServer();
      }
    }
  } catch (...) {
    m_trayIcon->terminate();
    m_trayIcon->waitForTermination();
    shutdown();
  }
}

int ControlApplication::runControlInterface(bool showIcon)
{
  m_trayIcon = new ControlTrayIcon(m_serverControl, this, this, showIcon);

  resume();

  int ret = WindowsApplication::run();

  terminate();
  wait();

  delete m_trayIcon;

  return ret;
}

int ControlApplication::runControlCommand(Command *command)
{
  ControlCommand ctrlCmd(command);

  ctrlCmd.execute();

  int errorCode = ctrlCmd.executionResultOk() ? 0 : 1;
  return errorCode;
}
