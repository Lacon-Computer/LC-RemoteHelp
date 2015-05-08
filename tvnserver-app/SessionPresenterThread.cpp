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

#include "SessionPresenterThread.h"
#include "TvnServerApplication.h"

SessionPresenterThread::SessionPresenterThread(Thread *parentThread,
  unsigned int sessionId)
  : WindowsApplication((HINSTANCE)GetModuleHandle(NULL), _T("SessionPresenterWindow")),
  m_parentThread(parentThread),
  m_sessionId(sessionId)
{
}

SessionPresenterThread::~SessionPresenterThread()
{
  WindowsApplication::shutdown();
  Thread::wait();
}

void SessionPresenterThread::execute()
{
  SessionPresenterDialog sessionPresenterDialog(this, m_sessionId);

  sessionPresenterDialog.show();
  sessionPresenterDialog.setForeground();
  addModelessDialog(sessionPresenterDialog.getControl()->getWindow());

  WindowsApplication::run();

  int result = sessionPresenterDialog.getResult();
  if (result == 1) {
    m_parentThread->terminate();
  }
}
