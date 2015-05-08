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

#include "Font.h"

Font::Font()
  :m_font(0)
{
}

Font::~Font()
{
  release();
}

void Font::setFont(const TCHAR *typeface, int size, bool bold)
{
  release();
  m_font = CreateFont(size, 0, 0, 0, bold ? FW_BOLD : FW_DONTCARE, FALSE,
    FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, typeface);
}

void Font::release()
{
  if (m_font != 0) {
    DeleteObject(m_font);
    m_font = 0;
  }
}
