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

#include "ControlCommandLine.h"

#include "util/CommonHeader.h"
#include "util/CommandLine.h"
#include "region/RectSerializer.h"
#include "util/StringParser.h"

#include "ConnectStringParser.h"

const TCHAR ControlCommandLine::CONTROL_APPLICATION[] = _T("-controlapp");
const TCHAR ControlCommandLine::CONFIG_RELOAD[]  = _T("-reload");
const TCHAR ControlCommandLine::SHUTDOWN[] = _T("-shutdown");

const TCHAR ControlCommandLine::SLAVE_MODE[] = _T("-slave");

ControlCommandLine::ControlCommandLine()
{
}

ControlCommandLine::~ControlCommandLine()
{
}

void ControlCommandLine::parse(const CommandLineArgs *cmdArgs)
{
  CommandLineFormat fmt[] = {
    { CONFIG_RELOAD, NO_ARG },
    { SHUTDOWN, NO_ARG },
    { CONTROL_APPLICATION, NO_ARG },
    { SLAVE_MODE, NO_ARG },
  };

  if (!CommandLine::parse(fmt, sizeof(fmt) / sizeof(CommandLineFormat), cmdArgs)) {
    throw CommandLineFormatException();
  }

  if (hasControlAppFlag() && (isSlave()) && (m_foundKeys.size() > 2)) {
    throw CommandLineFormatException();
  }

  bool hasNotSlaveControl = hasControlAppFlag() && !isSlave();
  if ((hasNotSlaveControl && m_foundKeys.size() > 2)) {
    throw CommandLineFormatException();
  }

  if (m_foundKeys.size() == 0) {
    throw CommandLineFormatException();
  }
}

bool ControlCommandLine::hasReloadFlag()
{
  return optionSpecified(CONFIG_RELOAD);
}

void ControlCommandLine::getConnectHostName(StringStorage *hostName) const
{
  *hostName = m_connectHostName;
}

bool ControlCommandLine::hasShutdownFlag()
{
  return optionSpecified(SHUTDOWN);
}

bool ControlCommandLine::hasControlAppFlag()
{
  return optionSpecified(CONTROL_APPLICATION);
}

bool ControlCommandLine::isSlave()
{
  return optionSpecified(SLAVE_MODE);
}

bool ControlCommandLine::isCommandSpecified()
{
  return hasReloadFlag() || hasShutdownFlag();
}
