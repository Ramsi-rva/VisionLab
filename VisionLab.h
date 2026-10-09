// VisionLab.h - aplicación MFC basada en cuadro de diálogo
#pragma once

#ifndef __AFXWIN_H__
#error "incluir 'pch.h' antes de incluir este archivo para PCH"
#endif

#include "resource.h"

class CVisionLabApp : public CWinApp
{
public:
    CVisionLabApp() = default;

    virtual BOOL InitInstance();

    DECLARE_MESSAGE_MAP()
};

extern CVisionLabApp theApp;
