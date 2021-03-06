// Copyright (C) 2012 GlavSoft LLC.
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

#ifndef _PUBLIC_NAMES_H_
#define _PUBLIC_NAMES_H_

// This file contains all public names.
#include "util/CommonHeader.h"

class NamingDefs
{
public:
  static const TCHAR MIRROR_DRIVER_MESSAGE_WINDOW_CLASS_NAME[];
};

class ProductNames
{
public:
  static const TCHAR PRODUCT_NAME[];
  static const TCHAR SERVER_PRODUCT_NAME[];
};

class IniFilePaths
{
public:
  static const TCHAR SERVER_PATH[];
};

class IniFileKeys
{
public:
  static const TCHAR SERVER_CONFIG[];
};

class WindowNames
{
public:
  static const TCHAR WINDOW_CLASS_NAME[];
};

class LogNames
{
public:
  static const TCHAR SERVER_LOG_FILE_STUB_NAME[];
  static const TCHAR VIEWER_LOG_FILE_STUB_NAME[];
  static const TCHAR LOG_DIR_NAME[];
  static const TCHAR LOG_PIPE_PUBLIC_NAME[];
};

class ClipboardNames
{
public:
  static const TCHAR CLIPBOARD_WIN_CLASS_NAME[];
};

class ServerApplicationNames
{
public:
  static const TCHAR SERVER_INSTANCE_MUTEX_NAME[];
  static const TCHAR FOR_APP_CONTROL_APP_SERVICE_PIPE_NAME[];
};

class HookDefinitions
{
public:
  static const TCHAR HOOK_LOADER_WINDOW_CLASS[];
  static const TCHAR HOOK_TARGET_WIN_CLASS_NAME[];
  static const TCHAR HOOK_LOADER_NAME[];
  static const UINT LOADER_CLOSE_CODE;
  static const UINT SPEC_IPC_CODE;
};

class DefaultNames
{
public:
  static const TCHAR DEFAULT_COMPUTER_NAME[];
};

#endif // _PUBLIC_NAMES_H_
