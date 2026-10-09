// VisionLabDlg.cpp - implementación de la ventana principal
#include "pch.h"
#include "VisionLab.h"
#include "VisionLabDlg.h"

#include <algorithm>

static const UINT_PTR kCameraTimer = 1;
static const int      kMaxPreview  = 1280;   // lado máximo de la vista previa

BEGIN_MESSAGE_MAP(CVisionLabDlg, CDialog)
    ON_WM_DRAWITEM()
    ON_WM_HSCROLL()
    ON_WM_TIMER()
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_BTN_OPEN,   &CVisionLabDlg::OnBnClickedOpen)
    ON_BN_CLICKED(IDC_BTN_SAVE,   &CVisionLabDlg::OnBnClickedSave)
    ON_BN_CLICKED(IDC_BTN_CAMERA, &CVisionLabDlg::OnBnClickedCamera)
    ON_CBN_SELCHANGE(IDC_COMBO_FILTER, &CVisionLabDlg::OnCbnSelchangeFilter)
END_MESSAGE_MAP()

CVisionLabDlg::CVisionLabDlg(CWnd* pParent)
    : CDialog(IDD_VISIONLAB_DIALOG, pParent)
{
    m_hIcon = AfxGetApp()->LoadStandardIcon(IDI_APPLICATION);
}

void CVisionLabDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_FILTER, m_combo);
    DDX_Control(pDX, IDC_SLIDER_P1, m_slider1);
    DDX_Control(pDX, IDC_SLIDER_P2, m_slider2);
}

// ---------------------------------------------------------------------------
// Inicialización
// ---------------------------------------------------------------------------
BOOL CVisionLabDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    SetIcon(m_hIcon, TRUE);
    SetIcon(m_hIcon, FALSE);

    // El combo se llena con la tabla de filtros definida en ImageProcessor.cpp
    for (int i = 0; i < ImageProcessor::FilterCount(); ++i)
        m_combo.AddString(ImageProcessor::Info(static_cast<FilterType>(i)).name);
    m_combo.SetCurSel(static_cast<int>(FilterType::Canny));

    // Imagen de demostración generada con funciones de dibujo de OpenCV,
    // para que los filtros se puedan probar sin cargar ningún archivo.
    m_sourceName = L"imagen de demostración";
    SetSource(CreateDemoImage(), false);

    ConfigureSliders();
    Process();
    return TRUE;
}

void CVisionLabDlg::OnOK()
{
    // Intencionalmente vacío: Enter no debe cerrar la aplicación.
}

void CVisionLabDlg::OnDestroy()
{
    StopCamera();
    CDialog::OnDestroy();
}

// ---------------------------------------------------------------------------
// Imagen de demostración (cv::Mat dibujada con OpenCV)
// ---------------------------------------------------------------------------
cv::Mat CVisionLabDlg::CreateDemoImage()
{
    cv::Mat img(400, 640, CV_8UC3);

    // Degradado horizontal de color
    for (int x = 0; x < img.cols; ++x)
    {
        double t = static_cast<double>(x) / (img.cols - 1);
        img.col(x).setTo(cv::Scalar(40 + 150 * (1 - t), 90 + 60 * t, 60 + 170 * t));
    }
    cv::rectangle(img, cv::Rect(40, 60, 160, 120), cv::Scalar(255, 255, 255), cv::FILLED);
    cv::circle(img, cv::Point(320, 200), 80, cv::Scalar(0, 200, 255), cv::FILLED);
    cv::circle(img, cv::Point(320, 200), 80, cv::Scalar(20, 20, 20), 3);
    cv::ellipse(img, cv::Point(500, 260), cv::Size(90, 50), 20, 0, 360, cv::Scalar(30, 30, 200), cv::FILLED);
    cv::line(img, cv::Point(0, 360), cv::Point(639, 300), cv::Scalar(255, 255, 0), 4);
    cv::putText(img, "OpenCV 5.1 + MFC", cv::Point(150, 50),
                cv::FONT_HERSHEY_DUPLEX, 1.4, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::putText(img, "Abre una imagen o activa la camara", cv::Point(120, 385),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 2, cv::LINE_AA);
    return img;
}

// ---------------------------------------------------------------------------
// Fuente de imagen
// ---------------------------------------------------------------------------
void CVisionLabDlg::SetSource(const cv::Mat& bgr, bool keepFull)
{
    if (bgr.empty())
        return;

    m_full = keepFull ? bgr : cv::Mat();

    // Para que los sliders respondan en tiempo real, la vista previa se limita
    // a 1280 px; el archivo guardado se procesa con la resolución original.
    int big = std::max(bgr.cols, bgr.rows);
    if (big > kMaxPreview)
    {
        double s = static_cast<double>(kMaxPreview) / big;
        cv::resize(bgr, m_src, cv::Size(), s, s, cv::INTER_AREA);
    }
    else
    {
        m_src = bgr.clone();
    }
}

// ---------------------------------------------------------------------------
// Sliders y filtro
// ---------------------------------------------------------------------------
void CVisionLabDlg::ConfigureSliders()
{
    int sel = m_combo.GetCurSel();
    if (sel < 0) sel = 0;
    const FilterInfo& fi = ImageProcessor::Info(static_cast<FilterType>(sel));

    struct { CSliderCtrl* s; const wchar_t* name; int mn, mx, df; } cfg[2] =
    {
        { &m_slider1, fi.p1Name, fi.p1Min, fi.p1Max, fi.p1Def },
        { &m_slider2, fi.p2Name, fi.p2Min, fi.p2Max, fi.p2Def },
    };

    for (auto& c : cfg)
    {
        bool used = (c.name != nullptr);
        c.s->EnableWindow(used);
        if (used)
        {
            c.s->SetRange(c.mn, c.mx, TRUE);
            c.s->SetPos(c.df);
            int range = c.mx - c.mn;
            c.s->SetTicFreq(range > 20 ? range / 10 : 1);
        }
        else
        {
            c.s->SetRange(0, 1, TRUE);
            c.s->SetPos(0);
        }
    }
    UpdateSliderLabels();
}

void CVisionLabDlg::UpdateSliderLabels()
{
    int sel = m_combo.GetCurSel();
    if (sel < 0) sel = 0;
    const FilterInfo& fi = ImageProcessor::Info(static_cast<FilterType>(sel));

    CString s;
    if (fi.p1Name) s.Format(L"%s: %d", fi.p1Name, m_slider1.GetPos());
    else           s = L"(este filtro no usa parámetro 1)";
    SetDlgItemText(IDC_LBL_P1, s);

    if (fi.p2Name) s.Format(L"%s: %d", fi.p2Name, m_slider2.GetPos());
    else           s = L"(este filtro no usa parámetro 2)";
    SetDlgItemText(IDC_LBL_P2, s);
}

void CVisionLabDlg::OnCbnSelchangeFilter()
{
    ConfigureSliders();
    Process();
}

void CVisionLabDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    CDialog::OnHScroll(nSBCode, nPos, pScrollBar);

    HWND h = pScrollBar ? pScrollBar->GetSafeHwnd() : nullptr;
    if (h && (h == m_slider1.GetSafeHwnd() || h == m_slider2.GetSafeHwnd()))
    {
        UpdateSliderLabels();
        if (!m_cameraOn)       // con cámara activa el timer ya reprocesa cada cuadro
            Process();
    }
}

// ---------------------------------------------------------------------------
// Procesamiento: aquí se usa OpenCV
// ---------------------------------------------------------------------------
void CVisionLabDlg::Process()
{
    if (m_src.empty())
        return;

    int sel = m_combo.GetCurSel();
    if (sel < 0) sel = 0;

    int64 t0 = cv::getTickCount();
    try
    {
        m_dst = m_proc.Apply(m_src, static_cast<FilterType>(sel),
                             m_slider1.GetPos(), m_slider2.GetPos());
    }
    catch (const cv::Exception& e)
    {
        // Un error de OpenCV no debe cerrar la aplicación
        m_dst.release();
        CString msg(e.what());
        SetDlgItemText(IDC_STATIC_INFO, L"Error de OpenCV: " + msg);
        return;
    }
    m_lastMs = (cv::getTickCount() - t0) * 1000.0 / cv::getTickFrequency();

    UpdateStatus();
    GetDlgItem(IDC_PIC_ORIG)->Invalidate(FALSE);
    GetDlgItem(IDC_PIC_RES)->Invalidate(FALSE);
}

void CVisionLabDlg::UpdateStatus()
{
    int sel = m_combo.GetCurSel();
    if (sel < 0) sel = 0;

    CString extra;
    if (static_cast<FilterType>(sel) == FilterType::FaceDetect)
        extra.Format(L"  |  Rostros: %d", m_proc.LastFaceCount());

    CString s;
    s.Format(L" OpenCV %S  |  %s: %d x %d  |  %s  |  %.1f ms%s%s",
             ImageProcessor::OpenCVVersion().c_str(),
             m_cameraOn ? L"cámara" : static_cast<LPCWSTR>(m_sourceName),
             m_src.cols, m_src.rows,
             ImageProcessor::Info(static_cast<FilterType>(sel)).name,
             m_lastMs, static_cast<LPCWSTR>(extra),
             (!m_full.empty() && !m_cameraOn) ? L"  |  al guardar se usa la resolución original" : L"");
    SetDlgItemText(IDC_STATIC_INFO, s);
}

// ---------------------------------------------------------------------------
// Botones
// ---------------------------------------------------------------------------
void CVisionLabDlg::OnBnClickedOpen()
{
    CFileDialog dlg(TRUE, nullptr, nullptr, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                    L"Imágenes|*.jpg;*.jpeg;*.png;*.bmp;*.tif;*.tiff;*.webp|Todos los archivos|*.*||",
                    this);
    if (dlg.DoModal() != IDOK)
        return;

    cv::Mat img = ImageProcessor::LoadImage(std::wstring(dlg.GetPathName().GetString()));
    if (img.empty())
    {
        AfxMessageBox(L"No se pudo leer la imagen. Verifica que el formato sea compatible.",
                      MB_ICONWARNING);
        return;
    }

    StopCamera();
    m_sourceName = dlg.GetFileName();
    SetSource(img, true);
    Process();
}

void CVisionLabDlg::OnBnClickedSave()
{
    if (m_dst.empty())
    {
        AfxMessageBox(L"No hay ningún resultado para guardar.", MB_ICONINFORMATION);
        return;
    }

    CFileDialog dlg(FALSE, L"png", L"resultado",
                    OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY,
                    L"PNG|*.png|JPEG|*.jpg|BMP|*.bmp||", this);
    if (dlg.DoModal() != IDOK)
        return;

    // Si se cargó un archivo, el filtro se vuelve a aplicar a la imagen original
    // a resolución completa; con cámara se guarda el cuadro actual.
    cv::Mat toSave = m_dst;
    if (!m_full.empty() && !m_cameraOn)
    {
        int sel = m_combo.GetCurSel();
        if (sel < 0) sel = 0;
        CWaitCursor wait;
        toSave = m_proc.Apply(m_full, static_cast<FilterType>(sel),
                              m_slider1.GetPos(), m_slider2.GetPos());
    }

    if (!ImageProcessor::SaveImage(std::wstring(dlg.GetPathName().GetString()), toSave))
        AfxMessageBox(L"No se pudo guardar el archivo.", MB_ICONERROR);
}

void CVisionLabDlg::OnBnClickedCamera()
{
    if (m_cameraOn)
    {
        StopCamera();
        return;
    }

    CWaitCursor wait;
    // CAP_DSHOW abre la cámara mucho más rápido que el backend por defecto en Windows
    if (!m_cap.open(0, cv::CAP_DSHOW) && !m_cap.open(0))
    {
        AfxMessageBox(L"No se pudo abrir la cámara.", MB_ICONWARNING);
        return;
    }

    m_cameraOn = true;
    SetDlgItemText(IDC_BTN_CAMERA, L"Detener cámara");
    GetDlgItem(IDC_BTN_OPEN)->EnableWindow(FALSE);
    SetTimer(kCameraTimer, 33, nullptr);      // ~30 cuadros por segundo
}

void CVisionLabDlg::StopCamera()
{
    if (!m_cameraOn && !m_cap.isOpened())
        return;

    bool wasOn = m_cameraOn;
    KillTimer(kCameraTimer);
    m_cap.release();
    m_cameraOn = false;

    if (::IsWindow(m_hWnd))
    {
        SetDlgItemText(IDC_BTN_CAMERA, L"Iniciar cámara");
        GetDlgItem(IDC_BTN_OPEN)->EnableWindow(TRUE);

        // El último cuadro queda como imagen de trabajo: se puede seguir
        // probando filtros o guardarlo. La barra de estado lo indica.
        if (wasOn && !m_src.empty())
        {
            m_sourceName = L"captura de cámara";
            UpdateStatus();
        }
    }
}

void CVisionLabDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == kCameraTimer && m_cameraOn)
    {
        cv::Mat frame;
        if (m_cap.read(frame) && !frame.empty())
        {
            if (frame.type() != CV_8UC3)
                cv::cvtColor(frame, frame, frame.channels() == 1 ? cv::COLOR_GRAY2BGR
                                                                  : cv::COLOR_BGRA2BGR);
            if (IsDlgButtonChecked(IDC_CHK_MIRROR) == BST_CHECKED)
                cv::flip(frame, frame, 1);

            m_full.release();
            m_src = frame;
            Process();
        }
    }
    CDialog::OnTimer(nIDEvent);
}

// ---------------------------------------------------------------------------
// Dibujo: cv::Mat -> controles estáticos "owner-draw"
// ---------------------------------------------------------------------------
void CVisionLabDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDIS)
{
    if (nIDCtl == IDC_PIC_ORIG || nIDCtl == IDC_PIC_RES)
    {
        CDC* pDC = CDC::FromHandle(lpDIS->hDC);
        CRect rc(lpDIS->rcItem);
        const cv::Mat& img = (nIDCtl == IDC_PIC_ORIG) ? m_src : m_dst;
        DrawMat(*pDC, rc, img, L"Sin imagen");
        return;
    }
    CDialog::OnDrawItem(nIDCtl, lpDIS);
}

void CVisionLabDlg::DrawMat(CDC& dc, const CRect& rc, const cv::Mat& img, const wchar_t* emptyText)
{
    // Doble búfer para evitar parpadeo al actualizar con la cámara
    CDC mem;
    mem.CreateCompatibleDC(&dc);
    CBitmap bmp;
    bmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
    CBitmap* old = mem.SelectObject(&bmp);

    CRect local(0, 0, rc.Width(), rc.Height());
    mem.FillSolidRect(local, RGB(32, 32, 32));

    if (img.empty() || img.type() != CV_8UC3)
    {
        mem.SetTextColor(RGB(200, 200, 200));
        mem.SetBkMode(TRANSPARENT);
        mem.DrawText(emptyText, local, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    else
    {
        // Escala que conserva la proporción (letterbox)
        double s = std::min(static_cast<double>(local.Width())  / img.cols,
                            static_cast<double>(local.Height()) / img.rows);
        int tw = std::max(4, static_cast<int>(img.cols * s));
        int th = std::max(1, static_cast<int>(img.rows * s));
        tw &= ~3;                                  // filas DIB de 24 bpp alineadas a 4 bytes
        if (tw < 4) tw = 4;

        cv::Mat view;
        cv::resize(img, view, cv::Size(tw, th), 0, 0,
                   s < 1.0 ? cv::INTER_AREA : cv::INTER_LINEAR);

        BITMAPINFO bi = {};
        bi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
        bi.bmiHeader.biWidth       = tw;
        bi.bmiHeader.biHeight      = -th;          // negativo = filas de arriba hacia abajo
        bi.bmiHeader.biPlanes      = 1;
        bi.bmiHeader.biBitCount    = 24;           // BGR: mismo orden que cv::Mat
        bi.bmiHeader.biCompression = BI_RGB;

        int x = (local.Width()  - tw) / 2;
        int y = (local.Height() - th) / 2;
        SetDIBitsToDevice(mem.GetSafeHdc(), x, y, tw, th, 0, 0, 0, th,
                          view.data, &bi, DIB_RGB_COLORS);
    }

    dc.BitBlt(rc.left, rc.top, rc.Width(), rc.Height(), &mem, 0, 0, SRCCOPY);
    mem.SelectObject(old);
}
