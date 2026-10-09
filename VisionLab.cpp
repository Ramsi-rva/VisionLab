// VisionLab.cpp - clase de la aplicación
#include "pch.h"
#include "VisionLab.h"
#include "VisionLabDlg.h"

BEGIN_MESSAGE_MAP(CVisionLabApp, CWinApp)
    ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CVisionLabApp theApp;

BOOL CVisionLabApp::InitInstance()
{
    // Requerido para los controles comunes (trackbar) con estilos visuales
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC  = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);

    CWinApp::InitInstance();

    CVisionLabDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();

    // El cuadro de diálogo se cerró: se devuelve FALSE para terminar la aplicación
    return FALSE;
}
