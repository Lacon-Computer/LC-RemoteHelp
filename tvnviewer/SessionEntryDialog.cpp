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

#include "SessionEntryDialog.h"

SessionEntryDialog::SessionEntryDialog()
  : BaseDialog(IDD_SESSION_ENTRY),
  m_sessionId(0)
{
}

BOOL SessionEntryDialog::onInitDialog()
{
  setControlById(m_sessionIdEdit, IDC_SESSION_ID);
  if (m_sessionIdStr.getLength() > 0) {
    m_sessionIdEdit.setText(m_sessionIdStr.getString());
  }
  m_sessionIdEdit.setFocus();

  return FALSE;
}

BOOL SessionEntryDialog::onCommand(UINT controlID, UINT notificationID)
{
  if (controlID == IDOK) {
    m_sessionIdEdit.getText(&m_sessionIdStr);
    m_sessionId = _tcstoul(m_sessionIdStr.getString(), NULL, 10);
    kill(1);
    return TRUE;
  }
  if (controlID == IDCANCEL) {
    kill(0);
    return TRUE;
  }
  return FALSE;
}
