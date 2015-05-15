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

#include "ControlTrayIcon.h"
#include "ControlApplication.h"

#include "UpdateRemoteConfigCommand.h"
#include "ShutdownCommand.h"
#include "ControlCommand.h"
#include "UpdateLocalConfigCommand.h"

#include "util/ResourceLoader.h"
#include "util/StringTable.h"
#include "util/MacroCommand.h"

#include "tvnserver/resource.h"

#include <crtdbg.h>

UINT ControlTrayIcon::WM_USER_TASKBAR;

ControlTrayIcon::ControlTrayIcon(ControlProxy *serverControl,
                                 Notificator *notificator,
                                 ControlApplication *appControl,
                                 bool showAfterCreation)
: NotifyIcon(showAfterCreation),
  m_serverControl(serverControl), m_notificator(notificator),
  m_appControl(appControl),
  m_inWindowProc(false),
  m_termination(false)
{
  ResourceLoader *resLoader = ResourceLoader::getInstance();

  m_iconWorking = new Icon(resLoader->loadIcon(MAKEINTRESOURCE(IDI_CONNECTED)));
  m_iconIdle = new Icon(resLoader->loadIcon(MAKEINTRESOURCE(IDI_IDLE)));
  m_iconDisabled = new Icon(resLoader->loadIcon(MAKEINTRESOURCE(IDI_DISABLED)));

  setWindowProcHolder(this);

  // Default icon state.
  setNotConnectedState();

  // Update status.
  syncStatusWithServer();

  WM_USER_TASKBAR = RegisterWindowMessage(_T("TaskbarCreated"));
}

ControlTrayIcon::~ControlTrayIcon()
{
  delete m_iconDisabled;
  delete m_iconIdle;
  delete m_iconWorking;
}

LRESULT ControlTrayIcon::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                    bool *useDefWindowProc)
{
  if (m_inWindowProc) {
    // This call is recursive, do not do any real work.
    *useDefWindowProc = true;
    return 0;
  }
  if (m_termination) {
    m_endEvent.notify();
    *useDefWindowProc = true;
    return 0;
  }

  // Make sure to reset it back to false before leaving this function for any
  // reason (check all return statements, exceptions should not happen here).
  m_inWindowProc = true;

  switch (uMsg) {
  case WM_USER + 1:
    switch (lParam) {
    case WM_RBUTTONUP:
      onRightButtonUp();
      break;
    case WM_LBUTTONDOWN:
      onLeftButtonDown();
      break;
    } // switch (lParam)
    break;
  default:
    if (uMsg == WM_USER_TASKBAR) {
      hide();
    }
    *useDefWindowProc = true;
  }

  m_inWindowProc = false;
  return 0;
}

void ControlTrayIcon::onRightButtonUp()
{
  HMENU hRoot = LoadMenu(GetModuleHandle(0), MAKEINTRESOURCE(IDR_TRAYMENU));
  HMENU hMenu = GetSubMenu(hRoot, 0);

  SetMenuDefaultItem(hMenu, ID_ABOUT_TIGHTVNC_MENUITEM, FALSE);

  if (m_appControl->m_slaveModeEnabled) {
    RemoveMenu(hMenu, ID_CLOSE_CONTROL_INTERFACE, MF_BYCOMMAND);
  }

  POINT pos;

  if (!GetCursorPos(&pos)) {
    pos.x = pos.y = 0;
  }

  SetForegroundWindow(getWindow());

  int action = TrackPopupMenu(hMenu,
                              TPM_NONOTIFY | TPM_RETURNCMD | TPM_RIGHTBUTTON,
                              pos.x, pos.y, 0, getWindow(), NULL);

  switch (action) {
  case ID_SHUTDOWN_SERVICE:
    onShutdownServerMenuItemClick();
    break;
  case ID_ABOUT_TIGHTVNC_MENUITEM:
    onAboutMenuItemClick();
    break;
  }
}

void ControlTrayIcon::onLeftButtonDown()
{
  onAboutMenuItemClick();
}

void ControlTrayIcon::onShutdownServerMenuItemClick()
{
  // Shutdown TightVNC server.

  ShutdownCommand unsafeCommand(m_serverControl);

  ControlCommand safeCommand(&unsafeCommand, m_notificator);

  safeCommand.execute();
}

void ControlTrayIcon::onAboutMenuItemClick()
{
  m_aboutDialog.show();

  ControlApplication::addModelessDialog(m_aboutDialog.getControl()->getWindow());
}

void ControlTrayIcon::syncStatusWithServer()
{
  try {
     // Get TightVNC server info.
    TvnServerInfo info = m_serverControl->getServerInfo();
    std::list<RfbClientInfo *> clients;
    m_serverControl->getClientsList(&clients);

    // Change icon status.
    if (clients.size() > 0) {
      setIcon(m_iconWorking);
    } else {
      setIcon(m_iconIdle);
    }

    setText(info.m_statusText.getString());

    if (clients.size() > m_lastKnownNumClients) {
      StringStorage message;
      if (clients.back()->m_contactName.isEmpty()) {
        message.format(StringTable::getString(IDS_NEW_CLIENT_BALLOON_MESSAGE),
          StringTable::getString(IDS_NEW_CLIENT_UNKNOWN_CONTACT));
      } else {
        message.format(StringTable::getString(IDS_NEW_CLIENT_BALLOON_MESSAGE),
          clients.back()->m_contactName.getString());
      }
      showBalloon(message.getString(),
        StringTable::getString(IDS_NEW_CLIENT_BALLOON_CAPTION), 5000);
    }

    m_lastKnownNumClients = clients.size();

    // Cleanup.
    for (std::list<RfbClientInfo *>::iterator it = clients.begin(); it != clients.end(); it++) {
      delete *it;
    }

    {
      AutoLock l(&m_serverInfoMutex);

      m_lastKnownServerInfo = info;
    }
  } catch (IOException &) {
    setNotConnectedState();
  } catch (Exception &) {
    _ASSERT(FALSE);
  } // try / catch.
}

void ControlTrayIcon::setNotConnectedState()
{
  setIcon(m_iconDisabled);
  setText(StringTable::getString(IDS_CONTROL_CLIENT_NOT_CONNECTED));
}

void ControlTrayIcon::terminate()
{
  m_termination = true;
  // Forcing window message
  PostMessage(getWindow(), WM_USER + 1, 0, 0);
}

void ControlTrayIcon::waitForTermination()
{
  m_endEvent.waitForEvent();
}
