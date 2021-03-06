// Copyright (C) 2015 Lacon Computer
// All rights reserved.
//
//-------------------------------------------------------------------------
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

#ifndef _SESSION_PRESENTER_DIALOG_H_
#define _SESSION_PRESENTER_DIALOG_H_

#include "win-system/WindowsApplication.h"
#include "gui/BaseDialog.h"
#include "gui/Control.h"
#include "gui/Font.h"

#include "rfb-sconn/RfbInitializer.h"

class SessionPresenterDialog : public BaseDialog
{
public:
  SessionPresenterDialog(WindowsApplication *windowsApplication,
    RfbInitializer *rfbInitializer);
  virtual ~SessionPresenterDialog();

  int getResult();

protected:
  void onCancelButtonClick();
  void updateOrganizationLabel();
  void updateConnectorIdLabel();

protected:
  /**
   * Inherited from BaseDialog
   */
  virtual BOOL onInitDialog();
  virtual BOOL onNotify(UINT controlID, LPARAM data);
  virtual BOOL onCommand(UINT controlID, UINT notificationID);
  virtual BOOL onDestroy();

protected:
  WindowsApplication *m_windowsApplication;
  Control m_organizationLabel;
  Control m_sessionIdLabel;
  Font m_sessionIdFont;
  RfbInitializer *m_rfbInitializer;
  int m_result;
};

#endif
