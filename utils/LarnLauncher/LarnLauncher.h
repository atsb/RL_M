#pragma once

/*
    Sprinkled Launcher - a minimalist WIN32 API launcher for Doom Source Ports
    Copyright (C) 2021  Adam Bilbrough

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "resource.h"

// This file hold the global variables in-use in SprinkledLauncher.cpp

// Global Variables:
HINSTANCE hInst;                                // current instance

// Help text
const wchar_t* help = L"   Commands:\n -s = Show scores\n -i = Show all scores\n -0 to -9 = Difficulty setting\n -h or -? = This help text\n\n   Enter Commands: \n";

// The window for the Command Line Arguments edit control
HWND CommandLineArgs;

// The integer for the length of our argument string
int argTextLength;

// The string that we will use to get our arguments
LPTSTR argText;
