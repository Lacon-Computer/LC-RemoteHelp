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

#include "util/CommonHeader.h"
#include "util/winhdr.h"
#include "util/CommandLine.h"
#include "win-system/WinCommandLineArgs.h"

#include "tvnserver-app/TvnServerApplication.h"

#include "tvncontrol-app/ControlApplication.h"
#include "tvncontrol-app/ControlCommandLine.h"

#include "tvnserver/resource.h"
#include "tvnserver-app/NamingDefs.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                       LPTSTR lpCmdLine, int nCmdShow)
{
  LogWriter preLog(0);

  ResourceLoader resourceLoaderSingleton(hInstance);

  CommandLineFormat format[] = {
    { ControlCommandLine::CONFIG_APPLICATION, NO_ARG },
    { ControlCommandLine::CONTROL_APPLICATION, NO_ARG },
  };

  CommandLine parser;

  // We really don't care about parsing result, we only need the first specified key.

  StringStorage firstKey(_T(""));


  try {
    WinCommandLineArgs args(lpCmdLine);
    parser.parse(format,  sizeof(format) / sizeof(CommandLineFormat), &args);
  } catch (...) {
  }
  parser.getOption(0, &firstKey);

  // Check if need to start additional application that packed into tvnserver.exe.

  if (firstKey.isEqualTo(ControlCommandLine::CONFIG_APPLICATION) ||
             firstKey.isEqualTo(ControlCommandLine::CONTROL_APPLICATION)) {
    try {
      ControlApplication tvnControl(hInstance,
        WindowNames::WINDOW_CLASS_NAME,
        lpCmdLine);
      return tvnControl.run();
    } catch (Exception &fatalException) {
      MessageBox(0,
        fatalException.getMessage(),
        StringTable::getString(IDS_MBC_TVNCONTROL),
        MB_OK | MB_ICONERROR);
      return 1;
    }
  }

  // No additional applications, run TightVNC server as single application.
  TvnServerApplication tvnServer(hInstance,
    WindowNames::WINDOW_CLASS_NAME,
    lpCmdLine);

  return tvnServer.run();
}
