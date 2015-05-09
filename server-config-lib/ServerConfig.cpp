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

#include "ServerConfig.h"

#include "win-system/Environment.h"

#include "file-lib/File.h"

ServerConfig::ServerConfig()
: m_rfbPort(5900),
  m_disconnectAction(DA_DO_NOTHING), m_logLevel(0), m_useControlAuth(false),
  m_controlAuthAlwaysChecking(false),
  m_acceptRfbConnections(true), m_useAuthentication(true),
  m_enableFileTransfers(true),
  m_mirrorDriverAllowed(true),
  m_removeWallpaper(true), m_hasReadOnlyPassword(false),
  m_hasPrimaryPassword(false), m_alwaysShared(false), m_neverShared(false),
  m_disconnectClients(true), m_pollingInterval(1000), m_localInputPriorityTimeout(3),
  m_blockLocalInput(false), m_blockRemoteInput(false), m_localInputPriority(false),
  m_videoRecognitionInterval(3000), m_grabTransparentWindows(true),
  m_saveLogToAllUsersPath(false), m_hasControlPassword(false),
  m_showTrayIcon(true)
{
  memset(m_primaryPassword,  0, sizeof(m_primaryPassword));
  memset(m_readonlyPassword, 0, sizeof(m_readonlyPassword));
  memset(m_controlPassword,  0, sizeof(m_controlPassword));
}

ServerConfig::~ServerConfig()
{
}

void ServerConfig::serialize(DataOutputStream *output)
{
  AutoLock l(this);

  output->writeInt32(m_rfbPort);
  output->writeInt8(m_enableFileTransfers ? 1 : 0);
  output->writeInt8(m_removeWallpaper ? 1 : 0);
  output->writeInt8(m_mirrorDriverAllowed ? 1 : 0);
  output->writeInt32(m_disconnectAction);
  output->writeInt8(m_acceptRfbConnections ? 1 : 0);
  output->writeFully(m_primaryPassword, VNC_PASSWORD_SIZE);
  output->writeFully(m_readonlyPassword, VNC_PASSWORD_SIZE);
  output->writeFully(m_controlPassword, VNC_PASSWORD_SIZE);
  output->writeInt8(m_useAuthentication ? 1 : 0);
  output->writeInt32(m_logLevel);
  output->writeInt8(m_useControlAuth ? 1 : 0);
  output->writeInt8(m_controlAuthAlwaysChecking ? 1 : 0);
  output->writeInt8(m_alwaysShared ? 1 : 0);
  output->writeInt8(m_neverShared ? 1 : 0);
  output->writeInt8(m_disconnectClients ? 1 : 0);
  output->writeUInt32(m_pollingInterval);
  output->writeInt8(m_blockRemoteInput ? 1 : 0);
  output->writeInt8(m_blockLocalInput ? 1 : 0);
  output->writeInt8(m_localInputPriority ? 1 : 0);
  output->writeUInt32(m_localInputPriorityTimeout);

  m_portMappings.serialize(output);

  _ASSERT((UINT32)m_videoClassNames.size() == m_videoClassNames.size());
  output->writeUInt32((UINT32)m_videoClassNames.size());
  for (size_t i = 0; i < m_videoClassNames.size(); i++) {
    output->writeUTF8(m_videoClassNames.at(i).getString());
  }

  output->writeUInt32(m_videoRecognitionInterval);
  output->writeInt8(m_grabTransparentWindows ? 1 : 0);

  output->writeInt8(m_saveLogToAllUsersPath ? 1 : 0);
  output->writeInt8(m_hasPrimaryPassword ? 1 : 0);
  output->writeInt8(m_hasReadOnlyPassword ? 1 : 0);
  output->writeInt8(m_hasControlPassword ? 1 : 0);
  output->writeInt8(m_showTrayIcon ? 1 : 0);

  output->writeUTF8(m_logFilePath.getString());
}

void ServerConfig::deserialize(DataInputStream *input)
{
  AutoLock l(this);

  m_rfbPort = input->readInt32();
  m_enableFileTransfers = input->readInt8() == 1;
  m_removeWallpaper = input->readInt8() == 1;
  m_mirrorDriverAllowed = input->readInt8() != 0;
  m_disconnectAction = (ServerConfig::DisconnectAction)input->readInt32();
  m_acceptRfbConnections = input->readInt8() == 1;
  input->readFully(m_primaryPassword, VNC_PASSWORD_SIZE);
  input->readFully(m_readonlyPassword, VNC_PASSWORD_SIZE);
  input->readFully(m_controlPassword, VNC_PASSWORD_SIZE);
  m_useAuthentication = input->readInt8() == 1;
  m_logLevel = input->readInt32();
  m_useControlAuth = input->readInt8() == 1;
  m_controlAuthAlwaysChecking = input->readInt8() != 0;
  m_alwaysShared = input->readInt8() == 1;
  m_neverShared = input->readInt8() == 1;
  m_disconnectClients = input->readInt8() == 1;
  m_pollingInterval = input->readUInt32();
  m_blockRemoteInput = input->readInt8() == 1;
  m_blockLocalInput = input->readInt8() == 1;
  m_localInputPriority = input->readInt8() == 1;
  m_localInputPriorityTimeout = input->readUInt32();

  m_portMappings.deserialize(input);

  m_videoClassNames.clear();
  size_t count = input->readUInt32();
  StringStorage videoClass;
  for (size_t i = 0; i < count; i++) {
    input->readUTF8(&videoClass);
    m_videoClassNames.push_back(videoClass);
  }

  m_videoRecognitionInterval = input->readUInt32();
  m_grabTransparentWindows = input->readInt8() == 1;

  m_saveLogToAllUsersPath = input->readInt8() == 1;
  m_hasPrimaryPassword = input->readInt8() == 1;
  m_hasReadOnlyPassword = input->readInt8() == 1;
  m_hasControlPassword = input->readInt8() == 1;
  m_showTrayIcon = input->readInt8() == 1;

  input->readUTF8(&m_logFilePath);
}

bool ServerConfig::getShowTrayIconFlag()
{
  AutoLock l(this);

  return m_showTrayIcon;
}

void ServerConfig::setShowTrayIconFlag(bool val)
{
  AutoLock l(this);

  m_showTrayIcon = val;
}

void ServerConfig::getLogFileDir(StringStorage *logFilePath)
{
  AutoLock l(this);

  *logFilePath = m_logFilePath;
}

void ServerConfig::setLogFileDir(const TCHAR *logFilePath)
{
  AutoLock l(this);

  m_logFilePath.setString(logFilePath);
}

bool ServerConfig::isControlAuthEnabled()
{
  AutoLock l(&m_objectCS);

  return m_useControlAuth;
}

void ServerConfig::useControlAuth(bool useAuth)
{
  AutoLock l(&m_objectCS);

  m_useControlAuth = useAuth;
}

bool ServerConfig::getControlAuthAlwaysChecking()
{
  AutoLock l(&m_objectCS);

  return m_controlAuthAlwaysChecking;
}

void ServerConfig::setControlAuthAlwaysChecking(bool value)
{
  AutoLock l(&m_objectCS);

  m_controlAuthAlwaysChecking = value;
}

void ServerConfig::setRfbPort(int port)
{
  AutoLock lock(&m_objectCS);
  if (port > 65535) {
    m_rfbPort = 65535;
  } else if (port <= 0) {
    m_rfbPort = 1;
  } else {
    m_rfbPort = port;
  }
}

int ServerConfig::getRfbPort()
{
  AutoLock lock(&m_objectCS);
  return m_rfbPort;
}

void ServerConfig::enableFileTransfers(bool enabled)
{
  AutoLock lock(&m_objectCS);
  m_enableFileTransfers = enabled;
}

bool ServerConfig::isFileTransfersEnabled()
{
  AutoLock lock(&m_objectCS);
  return m_enableFileTransfers;
}

void ServerConfig::enableRemovingDesktopWallpaper(bool enabled)
{
  AutoLock lock(&m_objectCS);
  m_removeWallpaper = enabled;
}

bool ServerConfig::isRemovingDesktopWallpaperEnabled()
{
  AutoLock lock(&m_objectCS);
  return m_removeWallpaper;
}

void ServerConfig::setDisconnectAction(DisconnectAction action)
{
  AutoLock lock(&m_objectCS);
  m_disconnectAction = action;
}

ServerConfig::DisconnectAction ServerConfig::getDisconnectAction()
{
  AutoLock lock(&m_objectCS);
  return m_disconnectAction;
}

bool ServerConfig::getMirrorIsAllowed()
{
  AutoLock lock(&m_objectCS);
  return m_mirrorDriverAllowed;
}

void ServerConfig::setMirrorAllowing(bool value)
{
  AutoLock lock(&m_objectCS);
  m_mirrorDriverAllowed = value;
}

bool ServerConfig::isAcceptingRfbConnections()
{
  AutoLock lock(&m_objectCS);
  return m_acceptRfbConnections;
}

void ServerConfig::acceptRfbConnections(bool accept)
{
  AutoLock lock(&m_objectCS);
  m_acceptRfbConnections = accept;
}

void ServerConfig::getPrimaryPassword(unsigned char *password)
{
  AutoLock lock(&m_objectCS);

  memcpy(password, m_primaryPassword, VNC_PASSWORD_SIZE);
}

void ServerConfig::setPrimaryPassword(const unsigned char *value)
{
  AutoLock lock(&m_objectCS);

  m_hasPrimaryPassword = true;

  memcpy((void *)&m_primaryPassword[0], (void *)value, VNC_PASSWORD_SIZE);
}

void ServerConfig::getReadOnlyPassword(unsigned char *password)
{
  AutoLock lock(&m_objectCS);

  memcpy(password, m_readonlyPassword, VNC_PASSWORD_SIZE);
}

void ServerConfig::setReadOnlyPassword(const unsigned char *value)
{
  AutoLock lock(&m_objectCS);

  m_hasReadOnlyPassword = true;

  memcpy((void *)&m_readonlyPassword[0], (void *)value, VNC_PASSWORD_SIZE);
}

void ServerConfig::getControlPassword(unsigned char *password)
{
  AutoLock lock(&m_objectCS);

  memcpy(password, m_controlPassword, VNC_PASSWORD_SIZE);
}

void ServerConfig::setControlPassword(const unsigned char *password)
{
  AutoLock lock(&m_objectCS);

  memcpy((void *)&m_controlPassword[0], (const void *)password, VNC_PASSWORD_SIZE);

  m_hasControlPassword = true;
}

bool ServerConfig::hasPrimaryPassword()
{
  AutoLock lock(&m_objectCS);

  return m_hasPrimaryPassword;
}

bool ServerConfig::hasReadOnlyPassword()
{
  AutoLock lock(&m_objectCS);

  return m_hasReadOnlyPassword;
}

bool ServerConfig::hasControlPassword()
{
  AutoLock lock(&m_objectCS);

  return m_hasControlPassword;
}

void ServerConfig::deletePrimaryPassword()
{
  AutoLock lock(&m_objectCS);

  m_hasPrimaryPassword = false;
}

void ServerConfig::deleteReadOnlyPassword()
{
  AutoLock lock(&m_objectCS);

  m_hasReadOnlyPassword = false;
}

void ServerConfig::deleteControlPassword()
{
  AutoLock lock(&m_objectCS);

  m_hasControlPassword = false;
}

bool ServerConfig::isUsingAuthentication()
{
  AutoLock lock(&m_objectCS);
  return m_useAuthentication;
}

void ServerConfig::useAuthentication(bool enabled)
{
  AutoLock lock(&m_objectCS);
  m_useAuthentication = enabled;
}

int ServerConfig::getLogLevel()
{
  AutoLock lock(&m_objectCS);
  return m_logLevel;
}

void ServerConfig::setLogLevel(int logLevel)
{
  AutoLock lock(&m_objectCS);
  if (logLevel < 0) {
    m_logLevel = 0;
  } else if (logLevel > 9) {
    m_logLevel = 9;
  } else {
    m_logLevel = logLevel;
  }
}

bool ServerConfig::isAlwaysShared()
{
  AutoLock lock(&m_objectCS);
  return m_alwaysShared;
}

bool ServerConfig::isNeverShared()
{
  AutoLock lock(&m_objectCS);
  return m_neverShared;
}

bool ServerConfig::isDisconnectingExistingClients()
{
  AutoLock lock(&m_objectCS);
  return m_disconnectClients;
}

void ServerConfig::setAlwaysShared(bool enabled)
{
  AutoLock lock(&m_objectCS);
  m_alwaysShared = enabled;
}

void ServerConfig::setNeverShared(bool enabled)
{
  AutoLock lock(&m_objectCS);
  m_neverShared = enabled;
}

void ServerConfig::disconnectExistingClients(bool disconnectExisting)
{
  AutoLock lock(&m_objectCS);
  m_disconnectClients = disconnectExisting;
}

void ServerConfig::setPollingInterval(unsigned int interval)
{
  AutoLock lock(&m_objectCS);
  if (interval < MINIMAL_POLLING_INTERVAL) {
    m_pollingInterval = MINIMAL_POLLING_INTERVAL;
  } else {
    m_pollingInterval = interval;
  }
}

unsigned int ServerConfig::getPollingInterval()
{
  AutoLock lock(&m_objectCS);
  return m_pollingInterval;
}

void ServerConfig::blockRemoteInput(bool blockEnabled)
{
  AutoLock lock(&m_objectCS);
  m_blockRemoteInput = blockEnabled;
}

bool ServerConfig::isBlockingRemoteInput()
{
  AutoLock lock(&m_objectCS);
  return m_blockRemoteInput;
}

void ServerConfig::setLocalInputPriority(bool localPriority)
{
  AutoLock lock(&m_objectCS);
  m_localInputPriority = localPriority;
}

bool ServerConfig::isLocalInputPriorityEnabled()
{
  AutoLock lock(&m_objectCS);
  return m_localInputPriority;
}

unsigned int ServerConfig::getLocalInputPriorityTimeout()
{
  AutoLock lock(&m_objectCS);
  return m_localInputPriorityTimeout;
}

void ServerConfig::setLocalInputPriorityTimeout(unsigned int value)
{
  AutoLock lock(&m_objectCS);
  if (value < MINIMAL_LOCAL_INPUT_PRIORITY_TIMEOUT) {
    m_localInputPriorityTimeout = MINIMAL_LOCAL_INPUT_PRIORITY_TIMEOUT;
  } else {
    m_localInputPriorityTimeout = value;
  }
}

void ServerConfig::blockLocalInput(bool enabled)
{
  AutoLock lock(&m_objectCS);
  m_blockLocalInput = enabled;
}

bool ServerConfig::isBlockingLocalInput()
{
  AutoLock lock(&m_objectCS);
  return m_blockLocalInput;
}

PortMappingContainer *ServerConfig::getPortMappingContainer()
{
  return &m_portMappings;
}

StringVector *ServerConfig::getVideoClassNames()
{
  return &m_videoClassNames;
}

unsigned int ServerConfig::getVideoRecognitionInterval()
{
  AutoLock lock(&m_objectCS);
  return m_videoRecognitionInterval;
}

void ServerConfig::setVideoRecognitionInterval(unsigned int interval)
{
  AutoLock lock(&m_objectCS);

  m_videoRecognitionInterval = interval;
}

void ServerConfig::saveLogToAllUsersPath(bool enabled)
{
  AutoLock lock(&m_objectCS);

  m_saveLogToAllUsersPath = enabled;
}

bool ServerConfig::isSaveLogToAllUsersPathFlagEnabled()
{
  AutoLock l(&m_objectCS);

  return m_saveLogToAllUsersPath;
}

void ServerConfig::setGrabTransparentWindowsFlag(bool grab)
{
  AutoLock lock(&m_objectCS);
  m_grabTransparentWindows = grab;
}

bool ServerConfig::getGrabTransparentWindowsFlag()
{
  AutoLock lock(&m_objectCS);
  return m_grabTransparentWindows;
}
