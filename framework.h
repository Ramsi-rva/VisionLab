// framework.h - encabezados de MFC y manifiesto de controles visuales
#pragma once

#include "targetver.h"

#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX                // evita que <windows.h> defina min/max (chocan con OpenCV y std::)
#define _AFX_NO_MFC_CONTROLS_IN_DIALOGS

#define VC_EXTRALEAN
#include <afxwin.h>             // componentes principales y estándar de MFC
#include <afxext.h>             // extensiones de MFC
#include <afxcmn.h>             // controles comunes (CSliderCtrl)

#ifdef _M_X64
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
