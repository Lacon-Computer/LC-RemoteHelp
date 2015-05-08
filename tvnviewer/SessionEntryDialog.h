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

#ifndef _SESSION_ENTRY_DIALOG_H_
#define _SESSION_ENTRY_DIALOG_H_

#include "gui/BaseDialog.h"
#include "gui/TextBox.h"
#include "resource.h"

class SessionEntryDialog : public BaseDialog
{
public:
  SessionEntryDialog();

  // this function returns the connector id entered by user
  unsigned int getSessionId() const { return m_sessionId; }

protected:
  BOOL onCommand(UINT controlID, UINT notificationID);
  BOOL onInitDialog();

  TextBox m_sessionIdEdit;
  StringStorage m_sessionIdStr;
  unsigned int m_sessionId;
};

#endif
