///////////////////////////////////////////////////////////////////////////////
///
/// \file Precompiled.hpp
/// Precompiled header for windows library.
/// 
/// Authors: Chris Peters
/// Copyright 2010-2011, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include "Common/CommonStandard.hpp"
#include "Platform/PlatformStandard.hpp"

//Include the windows header.
#include "Windows.hpp"
#include "WindowsError.hpp"
#include <shellapi.h>
#include <shlwapi.h>
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include <shlobj.h>
#include <io.h>
#include <crtdbg.h>
#include <winhttp.h>
#include <VersionHelpers.h>
#include <Lmcons.h>
#include <Xinput.h>
#include <iptypes.h>
#include <iphlpapi.h>
#include <setupapi.h>
#include <devguid.h>
#include <commctrl.h>
#include <dlgs.h>
#include <commdlg.h>
#include <shobjidl.h>
#include <dbghelp.h>
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Winhttp.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "hid.lib")
#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "user32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

extern "C"
{
// This should probably be setup to not use ..
#include "../External/WinHid/include/hidsdi.h"
}

#include "StackWalker.hpp"
#include "ThreadIo.hpp"

#include "WString.hpp"
#include "WinUtility.hpp"

#ifdef RunVld
#include <vld.h>
#endif
