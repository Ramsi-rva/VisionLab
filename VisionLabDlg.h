// VisionLabDlg.h - ventana principal: interacción GUI <-> OpenCV
#pragma once

#include "ImageProcessor.h"

class CVisionLabDlg : public CDialog
{
public:
    CVisionLabDlg(CWnd* pParent = nullptr);

    enum { IDD = IDD_VISIONLAB_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK();       // evita que Enter cierre la ventana

    afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnDestroy();
    afx_msg void OnBnClickedOpen();
    afx_msg void OnBnClickedSave();
    afx_msg void OnBnClickedCamera();
    afx_msg void OnCbnSelchangeFilter();
    DECLARE_MESSAGE_MAP()

private:
    // --- lógica de la interfaz ---
    void ConfigureSliders();           // ajusta rangos/etiquetas al filtro elegido
    void UpdateSliderLabels();
    void Process();                    // llama a OpenCV y refresca la pantalla
    void UpdateStatus();
    void StopCamera();
    void SetSource(const cv::Mat& bgr, bool keepFull);
    static cv::Mat CreateDemoImage();
    static void DrawMat(CDC& dc, const CRect& rc, const cv::Mat& img, const wchar_t* emptyText);

    // --- controles ---
    CComboBox    m_combo;
    CSliderCtrl  m_slider1;
    CSliderCtrl  m_slider2;

    // --- estado de visión por computadora ---
    ImageProcessor m_proc;
    cv::Mat        m_full;        // imagen cargada a resolución completa (para guardar)
    cv::Mat        m_src;         // imagen de trabajo (vista previa, máx. 1280 px)
    cv::Mat        m_dst;         // resultado del filtro
    cv::VideoCapture m_cap;
    bool           m_cameraOn = false;
    double         m_lastMs   = 0.0;
    CString        m_sourceName;

    HICON          m_hIcon = nullptr;
};
