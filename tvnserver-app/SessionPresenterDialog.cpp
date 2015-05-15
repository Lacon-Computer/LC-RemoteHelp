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

#include "SessionPresenterDialog.h"

#include "tvnserver/resource.h"

SessionPresenterDialog::SessionPresenterDialog(WindowsApplication *windowsApplication,
  RfbInitializer *rfbInitializer)
  : BaseDialog(IDD_SESSION_PRESENTER),
  m_windowsApplication(windowsApplication),
  m_rfbInitializer(rfbInitializer),
  m_result(0)
{
}

SessionPresenterDialog::~SessionPresenterDialog()
{
}

int SessionPresenterDialog::getResult()
{
  return m_result;
}

void SessionPresenterDialog::onCancelButtonClick()
{
  m_result = 1;
  kill(IDCANCEL);
}

void SessionPresenterDialog::updateOrganizationLabel()
{
  StringStorage labelText;
  labelText.format(_T("Connected to %s."), m_rfbInitializer->getOrganization().getString());

  m_organizationLabel.setText(labelText.getString());
}

void SessionPresenterDialog::updateConnectorIdLabel()
{
  unsigned int sessionId = m_rfbInitializer->getSessionId();
  unsigned int n[3];

  n[0] = (sessionId / 1000000) % 1000;
  n[1] = (sessionId / 1000) % 1000;
  n[2] = (sessionId / 1) % 1000;

  StringStorage labelText;
  labelText.format(_T("%03d %03d %03d"), n[0], n[1], n[2]);

  m_sessionIdLabel.setText(labelText.getString());
}

BOOL SessionPresenterDialog::onInitDialog()
{
  m_organizationLabel.setWindow(GetDlgItem(m_ctrlThis.getWindow(), IDC_ORGANIZATION));

  m_sessionIdLabel.setWindow(GetDlgItem(m_ctrlThis.getWindow(), IDC_SESSION_ID));
  m_sessionIdFont.setFont(_T("Arial"), 32, true);
  m_sessionIdLabel.setFont(&m_sessionIdFont);

  updateOrganizationLabel();
  updateConnectorIdLabel();

  return FALSE;
}

BOOL SessionPresenterDialog::onNotify(UINT controlId, LPARAM data)
{
  return FALSE;
}

BOOL SessionPresenterDialog::onCommand(UINT controlID, UINT notificationID)
{
  switch (controlID) {
  case IDCANCEL:
    onCancelButtonClick();
    break;
  }
  return TRUE;
}

BOOL SessionPresenterDialog::onDestroy()
{
  m_windowsApplication->shutdown();
  return FALSE;
}
