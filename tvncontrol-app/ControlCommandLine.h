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

#ifndef _CONTROL_COMMAND_LINE_H_
#define _CONTROL_COMMAND_LINE_H_

#include "util/CommandLine.h"
#include "util/CommandLineFormatException.h"

#include "region/Rect.h"

class ControlCommandLine : private CommandLine
{
public:
  static const TCHAR CONTROL_APPLICATION[];
  static const TCHAR CONFIG_RELOAD[];
  static const TCHAR DISCONNECT_ALL[];
  static const TCHAR CONNECT[];
  static const TCHAR SHUTDOWN[];

  static const TCHAR SLAVE_MODE[];

public:
  ControlCommandLine();
  virtual ~ControlCommandLine();

  void parse(const CommandLineArgs *cmdArgs) throw(CommandLineFormatException);

  bool hasReloadFlag();
  bool hasKillAllFlag();
  bool hasConnectFlag();
  void getConnectHostName(StringStorage *hostName) const;
  bool hasShutdownFlag();
  bool hasControlAppFlag();
  bool isSlave();

  bool isCommandSpecified();

private:

  StringStorage m_connectHostName;
};

#endif
