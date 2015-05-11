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
#include "ServerConfigDialog.h"
#include "ConfigDialog.h"
#include "ChangePasswordDialog.h"
#include "server-config-lib/Configurator.h"
#include "util/StringParser.h"
#include "CommonInputValidation.h"
#include "UIDataAccess.h"
#include <limits.h>

ServerConfigDialog::ServerConfigDialog()
: BaseDialog(IDD_CONFIG_SERVER_PAGE), m_parentDialog(NULL)
{
}

ServerConfigDialog::~ServerConfigDialog()
{
}

void ServerConfigDialog::setParentDialog(BaseDialog *dialog)
{
  m_parentDialog = dialog;
}

BOOL ServerConfigDialog::onInitDialog()
{
  m_config = Configurator::getInstance()->getServerConfig();
  initControls();
  updateUI();

  return TRUE;
}

BOOL ServerConfigDialog::onNotify(UINT controlID, LPARAM data)
{
  if (controlID == IDC_POLLING_INTERVAL_SPIN) {
    LPNMUPDOWN message = (LPNMUPDOWN)data;
    if (message->hdr.code == UDN_DELTAPOS) {
      onPollingIntervalSpinChangePos(message);
    }
  }
  return TRUE;
}

BOOL ServerConfigDialog::onCommand(UINT controlID, UINT notificationID)
{
  if (notificationID == BN_CLICKED) {
    switch (controlID) {
    case IDC_USE_AUTHENTICATION:
      onAuthenticationClick();
      break;
    case IDC_PRIMARY_PASSWORD:
      onPrimaryPasswordChange();
      break;
    case IDC_UNSET_PRIMARY_PASSWORD_BUTTON:
      onUnsetPrimaryPasswordClick();
      break;
    case IDC_ENABLE_FILE_TRANSFERS:
      onFileTransferCheckBoxClick();
      break;
    case IDC_REMOVE_WALLPAPER:
      onRemoveWallpaperCheckBoxClick();
      break;
    case IDC_BLOCK_LOCAL_INPUT:
      onBlockLocalInputChanged();
      break;
    case IDC_BLOCK_REMOTE_INPUT:
      onBlockLocalInputChanged();
      break;
    case IDC_LOCAL_INPUT_PRIORITY:
      onLocalInputPriorityChanged();
      break;
    case IDC_GRAB_TRANSPARENT:
      onGrabTransparentWindowsChanged();
      break;
    case IDC_USE_MIRROR_DRIVER:
      // FIXME: For high quality code is needed to use a function.
      ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
      break;
    case IDC_SHOW_TVNCONTROL_ICON_CHECKBOX:
      onShowTrayIconCheckBoxClick();
      break;
    }
  } else if (notificationID == EN_UPDATE) {
    switch (controlID) {
    case IDC_POLLING_INTERVAL:
      onPollingIntervalUpdate();
      break;
    case IDC_LOCAL_INPUT_PRIORITY_TIMEOUT:
      onInactivityTimeoutUpdate();
      break;
    }
  }
  return TRUE;
}

bool ServerConfigDialog::validateInput()
{
  bool commonValidationOk =
    CommonInputValidation::validateUINT(
      &m_pollingInterval,
      StringTable::getString(IDS_INVALID_POLLING_INTERVAL)) &&
    CommonInputValidation::validateUINT(
      &m_localInputPriorityTimeout,
      StringTable::getString(IDS_INVALID_INACTIVITY_TIMEOUT));

  if (!commonValidationOk) {
    return false;
  }

  unsigned int pollingInterval;

  UIDataAccess::queryValueAsUInt(&m_pollingInterval, &pollingInterval);

  if (pollingInterval < ServerConfig::MINIMAL_POLLING_INTERVAL) {
    CommonInputValidation::notifyValidationError(
      &m_pollingInterval,
      StringTable::getString(IDS_POLL_INTERVAL_TOO_SMALL));
    return false;
  }

  unsigned int inactivityTimeout;

  UIDataAccess::queryValueAsUInt(&m_localInputPriorityTimeout, &inactivityTimeout);

  if (inactivityTimeout < ServerConfig::MINIMAL_LOCAL_INPUT_PRIORITY_TIMEOUT) {
    CommonInputValidation::notifyValidationError(
      &m_localInputPriorityTimeout,
      StringTable::getString(IDS_INACTIVITY_TIMEOUT_TOO_SMALL));
    return false;
  }

  if (m_useAuthentication.isChecked() &&
      !m_ppControl->hasPassword()) {
    MessageBox(m_ctrlThis.getWindow(),
               StringTable::getString(IDS_SET_PASSWORD_NOTIFICATION),
               StringTable::getString(IDS_CAPTION_BAD_INPUT),
               MB_ICONSTOP | MB_OK);
    return false;
  }

  return true;
}

void ServerConfigDialog::updateUI()
{
  m_pollingInterval.setUnsignedInt(m_config->getPollingInterval());

  m_enableFileTransfers.check(m_config->isFileTransfersEnabled());
  m_removeWallpaper.check(m_config->isRemovingDesktopWallpaperEnabled());

  if (m_config->hasPrimaryPassword()) {
    UINT8 ppCrypted[8];
    m_config->getPrimaryPassword(ppCrypted);
    m_ppControl->setCryptedPassword((const char *)ppCrypted);
  }

  m_useAuthentication.check(m_config->isUsingAuthentication());

  m_blockLocalInput.check(m_config->isBlockingLocalInput());
  m_blockRemoteInput.check(m_config->isBlockingRemoteInput());
  m_localInputPriority.check(m_config->isLocalInputPriorityEnabled());
  if (m_config->isLocalInputPriorityEnabled()) {
    m_localInputPriorityTimeout.setEnabled(true);
  }
  m_localInputPriorityTimeout.setUnsignedInt(m_config->getLocalInputPriorityTimeout());

  m_grabTransparentWindows.check(m_config->getGrabTransparentWindowsFlag());
  m_useMirrorDriver.check(m_config->getMirrorIsAllowed());

  m_showTrayIcon.check(m_config->getShowTrayIconFlag());

  updateCheckboxesState();
  updateControlDependencies();
}

void ServerConfigDialog::apply()
{
  // Polling interval string storage
  StringStorage pollingIntervalText;

  m_pollingInterval.getText(&pollingIntervalText);

  int intVal = 0;

  StringParser::parseInt(pollingIntervalText.getString(), &intVal);
  m_config->setPollingInterval(intVal);

  m_config->enableFileTransfers(m_enableFileTransfers.isChecked());
  m_config->enableRemovingDesktopWallpaper(m_removeWallpaper.isChecked());

  m_config->useAuthentication(m_useAuthentication.isChecked());

  //
  // Primary password.
  //

  if (m_ppControl->hasPassword()) {
    m_config->setPrimaryPassword((const unsigned char *)m_ppControl->getCryptedPassword());
  } else {
    m_config->deletePrimaryPassword();
  }

  // Local input priority timeout string storage
  StringStorage liptStringStorage;
  m_localInputPriorityTimeout.getText(&liptStringStorage);
  int timeout = 0;

  m_config->setLocalInputPriority(m_localInputPriority.isChecked());
  if (StringParser::parseInt(liptStringStorage.getString(), &timeout)) {
    timeout = max(0, timeout);
    m_config->setLocalInputPriorityTimeout((unsigned int)timeout);
  }
  m_config->blockLocalInput(m_blockLocalInput.isChecked());
  m_config->blockRemoteInput(m_blockRemoteInput.isChecked());

  m_config->setMirrorAllowing(m_useMirrorDriver.isChecked());
  m_config->setGrabTransparentWindowsFlag(m_grabTransparentWindows.isChecked());
  m_config->setShowTrayIconFlag(m_showTrayIcon.isChecked());
}

void ServerConfigDialog::initControls()
{
  HWND hwnd = m_ctrlThis.getWindow();
  m_pollingInterval.setWindow(GetDlgItem(hwnd, IDC_POLLING_INTERVAL));
  m_grabTransparentWindows.setWindow(GetDlgItem(hwnd, IDC_GRAB_TRANSPARENT));
  m_useMirrorDriver.setWindow(GetDlgItem(hwnd, IDC_USE_MIRROR_DRIVER));
  m_enableFileTransfers.setWindow(GetDlgItem(hwnd, IDC_ENABLE_FILE_TRANSFERS));
  m_removeWallpaper.setWindow(GetDlgItem(hwnd, IDC_REMOVE_WALLPAPER));
  m_primaryPassword.setWindow(GetDlgItem(hwnd, IDC_PRIMARY_PASSWORD));
  m_useAuthentication.setWindow(GetDlgItem(hwnd, IDC_USE_AUTHENTICATION));
  m_unsetPrimaryPassword.setWindow(GetDlgItem(hwnd, IDC_UNSET_PRIMARY_PASSWORD_BUTTON));
  m_showTrayIcon.setWindow(GetDlgItem(hwnd, IDC_SHOW_TVNCONTROL_ICON_CHECKBOX));

  m_pollingIntervalSpin.setWindow(GetDlgItem(hwnd, IDC_POLLING_INTERVAL_SPIN));

  int limitersTmp[] = {50, 200};
  int deltasTmp[] = {5, 10};

  std::vector<int> limitters(limitersTmp, limitersTmp + sizeof(limitersTmp) /
                                                        sizeof(int));
  std::vector<int> deltas(deltasTmp, deltasTmp + sizeof(deltasTmp) /
                                                 sizeof(int));

  m_pollingIntervalSpin.setBuddy(&m_pollingInterval);
  m_pollingIntervalSpin.setAccel(0, 1);
  m_pollingIntervalSpin.setRange32(1, INT_MAX);
  m_pollingIntervalSpin.setAutoAccelerationParams(&limitters, &deltas, 50);
  m_pollingIntervalSpin.enableAutoAcceleration(true);

  m_blockLocalInput.setWindow(GetDlgItem(hwnd, IDC_BLOCK_LOCAL_INPUT));
  m_blockRemoteInput.setWindow(GetDlgItem(hwnd, IDC_BLOCK_REMOTE_INPUT));
  m_localInputPriority.setWindow(GetDlgItem(hwnd, IDC_LOCAL_INPUT_PRIORITY));
  m_localInputPriorityTimeout.setWindow(GetDlgItem(hwnd, IDC_LOCAL_INPUT_PRIORITY_TIMEOUT));
  m_inactivityTimeoutSpin.setWindow(GetDlgItem(hwnd, IDC_INACTIVITY_TIMEOUT_SPIN));

  m_inactivityTimeoutSpin.setBuddy(&m_localInputPriorityTimeout);
  m_inactivityTimeoutSpin.setAccel(0, 1);
  m_inactivityTimeoutSpin.setRange32(0, INT_MAX);

  // FIXME: This control is not used yet.
  m_grabTransparentWindows.check(true);

  m_ppControl = new PasswordControl(&m_primaryPassword, &m_unsetPrimaryPassword);
}

//
// TODO: Add comment to this method
//

void ServerConfigDialog::updateControlDependencies()
{
  bool passwordsAreEnabled = ((m_useAuthentication.isChecked()) && (m_useAuthentication.isEnabled()));

  m_ppControl->setEnabled(passwordsAreEnabled);

  m_pollingIntervalSpin.invalidate();
}

void ServerConfigDialog::onAuthenticationClick()
{
  updateControlDependencies();
  ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
}


void ServerConfigDialog::onShowTrayIconCheckBoxClick()
{
  bool oldVal = m_config->getShowTrayIconFlag();
  bool newVal = m_showTrayIcon.isChecked();

  if (oldVal != newVal) {
    ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
  }
}

void ServerConfigDialog::onPrimaryPasswordChange()
{
  if (m_ppControl->showChangePasswordModalDialog(&m_ctrlThis)) {
    ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
  }
}

void ServerConfigDialog::onUnsetPrimaryPasswordClick()
{
  m_ppControl->unsetPassword(true, m_ctrlThis.getWindow());

  ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
}

void ServerConfigDialog::onPollingIntervalSpinChangePos(LPNMUPDOWN message)
{
  m_pollingIntervalSpin.autoAccelerationHandler(message);
}

void ServerConfigDialog::onPollingIntervalUpdate()
{
  ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
}

void ServerConfigDialog::onFileTransferCheckBoxClick()
{
  ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
}

void ServerConfigDialog::onRemoveWallpaperCheckBoxClick()
{
  ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
}

void ServerConfigDialog::onGrabTransparentWindowsChanged()
{
  ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
}

void ServerConfigDialog::onBlockLocalInputChanged()
{
  updateCheckboxesState();
  ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
}

void ServerConfigDialog::onBlockRemoteInputChanged()
{
  updateCheckboxesState();
  ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
}

void ServerConfigDialog::onLocalInputPriorityChanged()
{
  updateCheckboxesState();
  ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
}

void ServerConfigDialog::onInactivityTimeoutUpdate()
{
  ((ConfigDialog *)m_parentDialog)->updateApplyButtonState();
}

void ServerConfigDialog::updateCheckboxesState()
{
  if (m_blockLocalInput.isChecked() || m_blockRemoteInput.isChecked()) {
    m_localInputPriority.check(false);
    m_localInputPriority.setEnabled(false);
  } else {
    m_localInputPriority.setEnabled(true);
  }

  if (m_localInputPriority.isChecked() && m_localInputPriority.isEnabled()) {
    m_localInputPriorityTimeout.setEnabled(true);
  } else {
    m_localInputPriorityTimeout.setEnabled(false);
  }
  m_inactivityTimeoutSpin.invalidate();
}
