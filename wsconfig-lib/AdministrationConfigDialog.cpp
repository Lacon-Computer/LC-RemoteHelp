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

#include "tvnserver/resource.h"
#include "AdministrationConfigDialog.h"
#include "CommonInputValidation.h"
#include "UIDataAccess.h"
#include "ConfigDialog.h"
#include "file-lib/File.h"
#include "server-config-lib/ServerConfig.h"
#include "server-config-lib/Configurator.h"
#include "util/CommonHeader.h"
#include "util/StringParser.h"
#include "wsconfig-lib/ChangePasswordDialog.h"
#include "util/StringTable.h"
#include "win-system/Process.h"
#include "tvnserver-app/NamingDefs.h"

AdministrationConfigDialog::AdministrationConfigDialog()
: BaseDialog(IDD_CONFIG_ADMINISTRATION_PAGE), m_parentDialog(NULL)
{
}

AdministrationConfigDialog::~AdministrationConfigDialog()
{
}

void AdministrationConfigDialog::setParentDialog(BaseDialog *dialog)
{
  m_parentDialog = dialog;
}

BOOL AdministrationConfigDialog::onInitDialog()
{
  m_config = Configurator::getInstance()->getServerConfig();

  initControls();
  updateUI();

  return TRUE;
}

BOOL AdministrationConfigDialog::onCommand(UINT controlID, UINT notificationID)
{
  if (notificationID == BN_CLICKED) {
    if (controlID == IDC_SHARED_RADIO1) {
      onShareRadioButtonClick(0);
    } else if (controlID == IDC_SHARED_RADIO2) {
      onShareRadioButtonClick(1);
    } else if (controlID == IDC_SHARED_RADIO3) {
      onShareRadioButtonClick(2);
    } else if (controlID == IDC_SHARED_RADIO4) {
      onShareRadioButtonClick(3);
    } else if (controlID == IDC_SHARED_RADIO5) {
      onShareRadioButtonClick(4);
    } else if (controlID == IDC_OPEN_LOG_FOLDER_BUTTON) {
      onOpenFolderButtonClick();
    } else if (controlID == IDC_LOG_FOR_ALL_USERS) {
      onLogForAllUsersClick();
    }

  } else if (notificationID == EN_UPDATE) {
    if (controlID == IDC_LOG_LEVEL) {
      onLogLevelUpdate();
    }
  }
  return TRUE;
}

bool AdministrationConfigDialog::validateInput()
{
  if (!CommonInputValidation::validateUINT(
    &m_logLevel,
    StringTable::getString(IDS_INVALID_LOG_LEVEL))) {
    return false;
  }

  unsigned int logLevel;

  UIDataAccess::queryValueAsUInt(&m_logLevel, &logLevel);

  if (logLevel > 9) {
    CommonInputValidation::notifyValidationError(
      &m_logLevel,
      StringTable::getString(IDS_INVALID_LOG_LEVEL));
    return false;
  }

  return true;
}

void AdministrationConfigDialog::updateUI()
{
  m_logLevel.setSignedInt(m_config->getLogLevel());

  ConfigDialog *configDialog = (ConfigDialog *)m_parentDialog;

  StringStorage logPath;

  m_config->getLogFileDir(&logPath);

  if (logPath.isEmpty()) {
    logPath.setString(StringTable::getString(IDS_LOGPATH_UNAVALIABLE));
    m_openLogPathButton.setEnabled(false);
    m_logPathTB.setEnabled(false);
  }

  m_logPathTB.setText(logPath.getString());

  StringStorage folder;
  getFolderName(logPath.getString(), &folder);

  File folderFile(folder.getString());

  if (folderFile.canRead()) {
    m_openLogPathButton.setEnabled(true);
  } else {
    m_openLogPathButton.setEnabled(false);
  }

  for (int i = 0; i < 5; i++) {
    m_shared[0].check(false);
  }

  if (m_config->isAlwaysShared() && !m_config->isNeverShared() && !m_config->isDisconnectingExistingClients()) {
    m_shared[0].check(true);
  }
  if (!m_config->isAlwaysShared() && m_config->isNeverShared() && !m_config->isDisconnectingExistingClients()) {
    m_shared[1].check(true);
  }
  if (!m_config->isAlwaysShared() && m_config->isNeverShared() && m_config->isDisconnectingExistingClients()) {
    m_shared[2].check(true);
  }
  if (!m_config->isAlwaysShared() && !m_config->isNeverShared() && !m_config->isDisconnectingExistingClients()) {
    m_shared[3].check(true);
  }
  if (!m_config->isAlwaysShared() && !m_config->isNeverShared() && m_config->isDisconnectingExistingClients()) {
    m_shared[4].check(true);
  }

  m_logForAllUsers.check(m_config->isSaveLogToAllUsersPathFlagEnabled());
}

void AdministrationConfigDialog::apply()
{
  StringStorage logLevelStringStorage;
  m_logLevel.getText(&logLevelStringStorage);

  int logLevel = 0;

  StringParser::parseInt(logLevelStringStorage.getString(), &logLevel);

  m_config->setLogLevel(logLevel);

  bool alwaysShared = false;
  bool neverShared = false;
  bool disconnectClients = false;

  if (m_shared[0].isChecked()) {
    alwaysShared = true;
    neverShared = false;
    disconnectClients = false;
  } else if (m_shared[1].isChecked()) {
    alwaysShared = false;
    neverShared = true;
    disconnectClients = false;
  } else if (m_shared[2].isChecked()) {
    alwaysShared = false;
    neverShared = true;
    disconnectClients = true;
  } else if (m_shared[3].isChecked()) {
    alwaysShared = false;
    neverShared = false;
    disconnectClients = false;
  } else if (m_shared[4].isChecked()) {
    alwaysShared = false;
    neverShared = false;
    disconnectClients = true;
  }

  m_config->setAlwaysShared(alwaysShared);
  m_config->setNeverShared(neverShared);
  m_config->disconnectExistingClients(disconnectClients);
  m_config->saveLogToAllUsersPath(m_logForAllUsers.isChecked());
}

void AdministrationConfigDialog::initControls()
{
  HWND hwnd = m_ctrlThis.getWindow();
  m_logLevel.setWindow(GetDlgItem(hwnd, IDC_LOG_LEVEL));
  m_logPathTB.setWindow(GetDlgItem(hwnd, IDC_LOG_FILEPATH_EDIT));

  m_openLogPathButton.setWindow(GetDlgItem(hwnd, IDC_OPEN_LOG_FOLDER_BUTTON));

  m_shared[0].setWindow(GetDlgItem(hwnd, IDC_SHARED_RADIO1));
  m_shared[1].setWindow(GetDlgItem(hwnd, IDC_SHARED_RADIO2));
  m_shared[2].setWindow(GetDlgItem(hwnd, IDC_SHARED_RADIO3));
  m_shared[3].setWindow(GetDlgItem(hwnd, IDC_SHARED_RADIO4));
  m_shared[4].setWindow(GetDlgItem(hwnd, IDC_SHARED_RADIO5));

  m_logForAllUsers.setWindow(GetDlgItem(hwnd, IDC_LOG_FOR_ALL_USERS));

  m_logSpin.setWindow(GetDlgItem(hwnd, IDC_LOG_LEVEL_SPIN));

  m_logSpin.setBuddy(&m_logLevel);
  m_logSpin.setRange(0, 9);
  m_logSpin.setAccel(0, 1);
}

void AdministrationConfigDialog::onShareRadioButtonClick(int number)
{
  if (!m_shared[number].isChecked()) {
    m_shared[number].check(true);
    for (int i = 0; i < 5; i++) {
      if (i != number) {
        m_shared[i].check(false);
      } // if
    } // for
    ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
  } // if
}

void AdministrationConfigDialog::onOpenFolderButtonClick()
{
  StringStorage logDir;

  m_config->getLogFileDir(&logDir);

  StringStorage command;

  command.format(_T("explorer /select,%s\\%s.log"), logDir.getString(),
                 LogNames::SERVER_LOG_FILE_STUB_NAME);

  Process explorer(command.getString());

  try {
    explorer.start();
  } catch (...) {
    // TODO: Place error notification here.
  }
}

void AdministrationConfigDialog::onLogForAllUsersClick()
{
  ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
}

void AdministrationConfigDialog::onLogLevelUpdate()
{
  ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
}

//
// FIXME: Dublicating code (see RegistrySettingsManager::getFolderName method)
//

void AdministrationConfigDialog::getFolderName(const TCHAR *key, StringStorage *folder)
{
  std::vector<TCHAR> folderString(_tcslen(key) + 1);
  memcpy(&folderString.front(), key, folderString.size() * sizeof(TCHAR));
  TCHAR *token = _tcsrchr(&folderString.front(), _T('\\'));
  if (token != NULL) {
    *token = _T('\0');
    folder->setString(&folderString.front());
  } else {
    folder->setString(_T(""));
  }
}
