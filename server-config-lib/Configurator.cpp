// Copyright (C) 2008,2009,2010,2011,2012 GlavSoft LLC.
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

#include "win-system/Environment.h"
#include "config-lib/RegistrySettingsManager.h"

#include "win-system/Registry.h"
#include "win-system/RegistryKey.h"

#include "Configurator.h"
#include "tvnserver-app/NamingDefs.h"

Configurator *Configurator::s_instance = NULL;
LocalMutex Configurator::m_instanceMutex;

Configurator::Configurator()
: m_isConfigLoadedPartly(false),
  m_isFirstLoad(true)
{
  AutoLock al(&m_instanceMutex);
  if (s_instance != 0) {
    throw Exception(_T("Configurator instance already exists"));
  }
  s_instance = this;
}

Configurator::~Configurator()
{
}

Configurator *Configurator::getInstance()
{
  AutoLock al(&m_instanceMutex);
  _ASSERT(s_instance != NULL);
  return s_instance;
}

void Configurator::setInstance(Configurator *conf)
{
  s_instance = conf;
}

void Configurator::notifyReload()
{
  AutoLock l(&m_listeners);

  for (size_t i = 0; i < m_listeners.size(); i++) {
    m_listeners.at(i)->onConfigReload(getServerConfig());
  }
}

bool Configurator::load()
{
  bool isOk = false;

  RegistrySettingsManager sm(HKEY_CURRENT_USER, RegistryPaths::SERVER_PATH, 0);

  isOk = load(&sm);

  notifyReload();

  return isOk;
}

bool Configurator::save()
{
  bool isOk = false;

  RegistrySettingsManager sm(HKEY_CURRENT_USER, RegistryPaths::SERVER_PATH, 0);

  isOk = save(&sm);

  return isOk;
}

bool Configurator::save(SettingsManager *sm)
{
  bool saveResult = true;
  if (!saveInputHandlingConfig(sm)) {
    saveResult = false;
  }
  if (!saveServerConfig(sm)) {
    saveResult = false;
  }
  return saveResult;
}

bool Configurator::load(SettingsManager *sm)
{
  bool loadResult = true;

  if (!loadInputHandlingConfig(sm, &m_serverConfig)) {
    loadResult = false;
  }

  if (!loadServerConfig(sm, &m_serverConfig)) {
    loadResult = false;
  }

  m_isFirstLoad = false;

  return loadResult;
}

bool Configurator::saveInputHandlingConfig(SettingsManager *sm)
{
  bool saveResult = true;
  if (!sm->setUINT(_T("LocalInputPriorityTimeout"), m_serverConfig.getLocalInputPriorityTimeout())) {
    saveResult = false;
  }
  if (!sm->setBoolean(_T("LocalInputPriority"), m_serverConfig.isLocalInputPriorityEnabled())) {
    saveResult = false;
  }
  if (!sm->setBoolean(_T("BlockRemoteInput"), m_serverConfig.isBlockingRemoteInput())) {
    saveResult = false;
  }
  if (!sm->setBoolean(_T("BlockLocalInput"), m_serverConfig.isBlockingLocalInput())) {
    saveResult = false;
  }
  return saveResult;
}

bool Configurator::loadInputHandlingConfig(SettingsManager *sm, ServerConfig *config)
{
  bool loadResult = true;

  //
  // Temporary variables
  //

  bool boolVal = false;
  UINT uintVal = 0;

  if (!sm->getUINT(_T("LocalInputPriorityTimeout"), &uintVal)) {
    loadResult = false;
  } else {
    m_isConfigLoadedPartly = true;
    config->setLocalInputPriorityTimeout(uintVal);
  }
  if (!sm->getBoolean(_T("LocalInputPriority"), &boolVal)) {
    loadResult = false;
  } else {
    m_isConfigLoadedPartly = true;
    config->setLocalInputPriority(boolVal);
  }
  if (!sm->getBoolean(_T("BlockRemoteInput"), &boolVal)) {
    loadResult = false;
  } else {
    m_isConfigLoadedPartly = true;
    config->blockRemoteInput(boolVal);
  }
  if (!sm->getBoolean(_T("BlockLocalInput"), &boolVal)) {
    loadResult = false;
  } else {
    m_isConfigLoadedPartly = true;
    config->blockLocalInput(boolVal);
  }

  return loadResult;
}

bool Configurator::saveServerConfig(SettingsManager *sm)
{
  bool saveResult = true;

  //
  // Temporary variables
  //

  StringStorage stringVal;

  if (!sm->setBoolean(_T("UseVncAuthentication"), m_serverConfig.isUsingAuthentication())) {
    saveResult = false;
  }
  if (!sm->setUINT(_T("LogLevel"), (UINT)m_serverConfig.getLogLevel())) {
    saveResult = false;
  }
  if (!sm->setBoolean(_T("EnableFileTransfers"), m_serverConfig.isFileTransfersEnabled())) {
    saveResult = false;
  }
  if (!sm->setBoolean(_T("RemoveWallpaper"), m_serverConfig.isRemovingDesktopWallpaperEnabled())) {
    saveResult = false;
  }
  if (!sm->setBoolean(_T("UseMirrorDriver"), m_serverConfig.getMirrorIsAllowed())) {
    saveResult = false;
  }
  if (!sm->setUINT(_T("LastSessionId"), m_serverConfig.getLastSessionId())) {
    saveResult = false;
  }
  if (m_serverConfig.hasPrimaryPassword()) {
    unsigned char password[VNC_PASSWORD_SIZE];

    m_serverConfig.getPrimaryPassword(&password[0]);

    if (!sm->setBinaryData(_T("Password"), &password[0], VNC_PASSWORD_SIZE)) {
      saveResult = false;
    }
  } else {
    sm->deleteKey(_T("Password"));
  }
  if (!sm->setUINT(_T("PollingInterval"), m_serverConfig.getPollingInterval())) {
    saveResult = false;
  }
  if (!sm->setBoolean(_T("GrabTransparentWindows"), m_serverConfig.getGrabTransparentWindowsFlag())) {
    saveResult = false;
  }
  if (!sm->setBoolean(_T("RunControlInterface"), m_serverConfig.getShowTrayIconFlag())) {
    saveResult = false;
  }
  m_serverConfig.getLogFileDir(&stringVal);
  if (!sm->setString(_T("LogFileDir"), stringVal.getString())) {
    saveResult = false;
  }
  return saveResult;
}

bool Configurator::loadServerConfig(SettingsManager *sm, ServerConfig *config)
{
  bool loadResult = true;

  //
  // Temporary variables
  //

  bool boolVal;
  UINT uintVal;
  StringStorage stringVal;

  if (!sm->getBoolean(_T("UseVncAuthentication"), &boolVal)) {
    loadResult = false;
  } else {
    m_isConfigLoadedPartly = true;
    m_serverConfig.useAuthentication(boolVal);
  }
  if (!sm->getUINT(_T("LogLevel"), &uintVal)) {
    loadResult = false;
  } else {
    m_isConfigLoadedPartly = true;
    m_serverConfig.setLogLevel(uintVal);
  }
  if (!sm->getBoolean(_T("EnableFileTransfers"), &boolVal)) {
    loadResult = false;
  } else {
    m_isConfigLoadedPartly = true;
    m_serverConfig.enableFileTransfers(boolVal);
  }
  if (!sm->getBoolean(_T("RemoveWallpaper"), &boolVal)) {
    loadResult = false;
  } else {
    m_isConfigLoadedPartly = true;
    m_serverConfig.enableRemovingDesktopWallpaper(boolVal);
  }
  if (!sm->getBoolean(_T("UseMirrorDriver"), &boolVal)) {
    loadResult = false;
  } else {
    m_isConfigLoadedPartly = true;
    m_serverConfig.setMirrorAllowing(boolVal);
  }
  if (!sm->getUINT(_T("LastSessionId"), &uintVal)) {
    loadResult = false;
  } else {
    m_isConfigLoadedPartly = true;
    m_serverConfig.setLastSessionId(uintVal);
  }

  size_t passSize = 8;
  unsigned char buffer[VNC_PASSWORD_SIZE] = {0};

  if (!sm->getBinaryData(_T("Password"), (void *)&buffer, &passSize)) {
    loadResult = false;
    m_serverConfig.deletePrimaryPassword();
  } else {
    m_isConfigLoadedPartly = true;
    m_serverConfig.setPrimaryPassword(&buffer[0]);
  }

  if (!sm->getUINT(_T("PollingInterval"), &uintVal)) {
    loadResult = false;
  } else {
    m_isConfigLoadedPartly = true;
    m_serverConfig.setPollingInterval(uintVal);
  }
  if (!sm->getBoolean(_T("GrabTransparentWindows"), &boolVal)) {
    loadResult = false;
  } else {
    m_isConfigLoadedPartly = true;
    m_serverConfig.setGrabTransparentWindowsFlag(boolVal);
  }
  if (!sm->getBoolean(_T("RunControlInterface"), &boolVal)) {
    loadResult = false;
  } else {
    m_isConfigLoadedPartly = true;
    m_serverConfig.setShowTrayIconFlag(boolVal);
  }
  if (!sm->getString(_T("LogFileDir"), &stringVal)) {
    loadResult = false;
  } else {
    m_isConfigLoadedPartly = true;
    m_serverConfig.setLogFileDir(stringVal.getString());
  }
  return loadResult;
}
