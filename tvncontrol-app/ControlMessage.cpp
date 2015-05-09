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

#include "ControlMessage.h"
#include "ControlProto.h"
#include "server-config-lib/Configurator.h"
#include "config-lib/RegistrySettingsManager.h"
#include "util/VncPassCrypt.h"
#include "util/AnsiStringStorage.h"
#include "tvnserver-app/NamingDefs.h"
#include "file-lib/WinFile.h"

#include "tvnserver/resource.h"

#include <crtdbg.h>

ControlMessage::ControlMessage(UINT32 messageId, ControlGate *gate)
: DataOutputStream(0), m_messageId(messageId), m_gate(gate)
{
  m_tunnel = new ByteArrayOutputStream(2048);

  m_outStream = m_tunnel;
}

ControlMessage::~ControlMessage()
{
  delete m_tunnel;
}

void ControlMessage::send()
{
  sendData();

  checkRetCode();
}

void ControlMessage::sendData()
{
  m_gate->writeUInt32(m_messageId);
  _ASSERT((UINT32)m_tunnel->size() == m_tunnel->size());
  m_gate->writeUInt32((UINT32)m_tunnel->size());
  m_gate->writeFully(m_tunnel->toByteArray(), m_tunnel->size());
}

void ControlMessage::checkRetCode()
{
  UINT32 messageId = m_gate->readUInt32();

  switch (messageId) {
  case ControlProto::REPLY_ERROR:
    {
      StringStorage message;
      m_gate->readUTF8(&message);
      throw RemoteException(message.getString());
    }
    break;
  case ControlProto::REPLY_OK:
    break;
  default:
    _ASSERT(FALSE);
    throw RemoteException(_T("Unknown ret code."));
  }
}
