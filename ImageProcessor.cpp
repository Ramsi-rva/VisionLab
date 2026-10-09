// ImageProcessor.cpp
#include "pch.h"
#include "ImageProcessor.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/geometry.hpp>   // OpenCV 5: contourArea y boundingRect pasaron al módulo geometry
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/utility.hpp>

#include <algorithm>
#include <cstdio>
#include <fstream>

// ---------------------------------------------------------------------------
// Tabla de filtros: nombre y parámetros que la interfaz muestra en los sliders
// ---------------------------------------------------------------------------
static const FilterInfo kFilters[] =
{
    // nombre                   p1                       min max def   p2                    min max def
    { L"Original",              nullptr,                  0,   0,   0,  nullptr,                0,   0,   0 },
    { L"Escala de grises",      nullptr,                  0,   0,   0,  nullptr,                0,   0,   0 },
    { L"Negativo",              nullptr,                  0,   0,   0,  nullptr,                0,   0,   0 },
    { L"Brillo y contraste",    L"Contraste (%)",         0, 300, 100,  L"Brillo (100 = 0)",    0, 200, 100 },
    { L"Desenfoque gaussiano",  L"Radio del kernel",      1,  30,   5,  nullptr,                0,   0,   0 },
    { L"Ecualización de histograma", nullptr,             0,   0,   0,  nullptr,                0,   0,   0 },
    { L"Sepia",                 L"Intensidad (%)",        0, 100, 100,  nullptr,                0,   0,   0 },
    { L"Umbralización",         L"Umbral",                0, 255, 127,  L"Adaptativo (0/1)",    0,   1,   0 },
    { L"Bordes (Canny)",        L"Umbral bajo",           0, 255,  50,  L"Umbral alto",         0, 255, 150 },
    { L"Caricatura",            L"Detalle de bordes",     2,  12,   4,  L"Suavizado",           1,  10,   5 },
    { L"Pixelado",              L"Tamaño de bloque",      2,  64,  12,  nullptr,                0,   0,   0 },
    { L"Contornos",             L"Umbral Canny",          0, 255, 100,  L"Área mínima",         0, 5000, 200 },
    { L"Detección de rostros",  L"Vecinos mínimos",       1,  10,   5,  L"Escala (x10)",       11,  20,  12 },
};
static_assert(sizeof(kFilters) / sizeof(kFilters[0]) == static_cast<size_t>(FilterType::Count),
              "La tabla de filtros debe coincidir con el enum FilterType");

const FilterInfo& ImageProcessor::Info(FilterType f)
{
    return kFilters[static_cast<int>(f)];
}

std::string ImageProcessor::OpenCVVersion()
{
    return cv::getVersionString();
}

// ---------------------------------------------------------------------------
// E/S con rutas Unicode
// ---------------------------------------------------------------------------
cv::Mat ImageProcessor::LoadImage(const std::wstring& path)
{
    FILE* fp = nullptr;
    if (_wfopen_s(&fp, path.c_str(), L"rb") != 0 || !fp)
        return cv::Mat();

    std::vector<uchar> buf;
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (size > 0)
    {
        buf.resize(static_cast<size_t>(size));
        size_t got = fread(buf.data(), 1, buf.size(), fp);
        buf.resize(got);
    }
    fclose(fp);
    if (buf.empty())
        return cv::Mat();

    // IMREAD_COLOR => siempre BGR de 3 canales y 8 bits
    return cv::imdecode(buf, cv::IMREAD_COLOR);
}

bool ImageProcessor::SaveImage(const std::wstring& path, const cv::Mat& img)
{
    if (img.empty())
        return false;

    // Se deduce el formato de la extensión
    std::wstring ext;
    size_t dot = path.find_last_of(L'.');
    if (dot != std::wstring::npos)
        ext = path.substr(dot);
    std::string cvExt = ".png";
    if (ext == L".jpg" || ext == L".jpeg" || ext == L".JPG" || ext == L".JPEG") cvExt = ".jpg";
    else if (ext == L".bmp" || ext == L".BMP") cvExt = ".bmp";

    std::vector<uchar> buf;
    if (!cv::imencode(cvExt, img, buf))
        return false;

    FILE* fp = nullptr;
    if (_wfopen_s(&fp, path.c_str(), L"wb") != 0 || !fp)
        return false;
    size_t written = fwrite(buf.data(), 1, buf.size(), fp);
    fclose(fp);
    return written == buf.size();
}

// ---------------------------------------------------------------------------
// Detector de rostros (Haar). El XML se copia junto al .exe en la compilación;
// si no está allí se busca en la instalación de OpenCV compilada en C:\cv5.
// ---------------------------------------------------------------------------
bool ImageProcessor::EnsureFaceCascade()
{
    if (m_cascadeTried)
        return m_cascadeOk;
    m_cascadeTried = true;

    wchar_t exe[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    std::wstring dir(exe);
    dir = dir.substr(0, dir.find_last_of(L"\\/") + 1);

    const std::wstring candidates[] =
    {
        dir + L"haarcascade_frontalface_default.xml",
        L"C:\\cv5\\install\\etc\\haarcascades\\haarcascade_frontalface_default.xml",
    };

    for (const auto& c : candidates)
    {
        // CascadeClassifier::load recibe std::string; se convierte desde UTF-16
        int n = WideCharToMultiByte(CP_ACP, 0, c.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (n <= 1) continue;
        std::string a(static_cast<size_t>(n - 1), '\0');
        WideCharToMultiByte(CP_ACP, 0, c.c_str(), -1, &a[0], n, nullptr, nullptr);
        if (m_faceCascade.load(a))
        {
            m_cascadeOk = true;
            break;
        }
    }
    return m_cascadeOk;
}

// ---------------------------------------------------------------------------
// Aplicación de filtros
// ---------------------------------------------------------------------------
cv::Mat ImageProcessor::Apply(const cv::Mat& bgr, FilterType f, int p1, int p2)
{
    CV_Assert(bgr.empty() || (bgr.type() == CV_8UC3));
    if (bgr.empty())
        return cv::Mat();

    cv::Mat out;

    switch (f)
    {
    case FilterType::Original:
        out = bgr.clone();
        break;

    case FilterType::Gray:
    {
        cv::Mat g;
        cv::cvtColor(bgr, g, cv::COLOR_BGR2GRAY);
        cv::cvtColor(g, out, cv::COLOR_GRAY2BGR);
        break;
    }

    case FilterType::Negative:
        cv::bitwise_not(bgr, out);
        break;

    case FilterType::BrightnessContrast:
        // out = alfa * in + beta   (convertTo satura a 0..255 automáticamente)
        bgr.convertTo(out, -1, p1 / 100.0, static_cast<double>(p2 - 100));
        break;

    case FilterType::GaussianBlur:
    {
        int k = 2 * std::max(1, p1) + 1;           // el kernel debe ser impar
        cv::GaussianBlur(bgr, out, cv::Size(k, k), 0);
        break;
    }

    case FilterType::Equalize:
    {
        // Se ecualiza solo el canal de luminancia para no alterar los colores
        cv::Mat ycrcb;
        cv::cvtColor(bgr, ycrcb, cv::COLOR_BGR2YCrCb);
        std::vector<cv::Mat> ch;
        cv::split(ycrcb, ch);
        cv::equalizeHist(ch[0], ch[0]);
        cv::merge(ch, ycrcb);
        cv::cvtColor(ycrcb, out, cv::COLOR_YCrCb2BGR);
        break;
    }

    case FilterType::Sepia:
    {
        // Matriz de transformación de color (filas = B, G, R de salida)
        const cv::Matx33f m(0.131f, 0.534f, 0.272f,
                            0.168f, 0.686f, 0.349f,
                            0.189f, 0.769f, 0.393f);
        cv::Mat sepia;
        cv::transform(bgr, sepia, m);
        double a = std::clamp(p1, 0, 100) / 100.0;
        cv::addWeighted(sepia, a, bgr, 1.0 - a, 0.0, out);
        break;
    }

    case FilterType::Threshold:
    {
        cv::Mat g, b;
        cv::cvtColor(bgr, g, cv::COLOR_BGR2GRAY);
        if (p2 > 0)
            cv::adaptiveThreshold(g, b, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C,
                                  cv::THRESH_BINARY, 11, 2);
        else
            cv::threshold(g, b, p1, 255, cv::THRESH_BINARY);
        cv::cvtColor(b, out, cv::COLOR_GRAY2BGR);
        break;
    }

    case FilterType::Canny:
    {
        cv::Mat g, e;
        cv::cvtColor(bgr, g, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(g, g, cv::Size(5, 5), 1.4);
        cv::Canny(g, e, std::min(p1, p2), std::max(p1, p2));
        cv::cvtColor(e, out, cv::COLOR_GRAY2BGR);
        break;
    }

    case FilterType::Cartoon:
    {
        // 1) Bordes con umbral adaptativo sobre la imagen en grises suavizada
        cv::Mat g, edges;
        cv::cvtColor(bgr, g, cv::COLOR_BGR2GRAY);
        cv::medianBlur(g, g, 2 * std::max(1, p2) + 1);
        int block = 2 * std::max(2, p1) + 1;
        cv::adaptiveThreshold(g, edges, 255, cv::ADAPTIVE_THRESH_MEAN_C,
                              cv::THRESH_BINARY, block, 9);
        // 2) Colores planos con filtro bilateral (conserva bordes)
        cv::Mat color;
        cv::bilateralFilter(bgr, color, 9, 75, 75);
        // 3) Se combinan: los bordes negros quedan sobre los colores suavizados
        cv::Mat edges3;
        cv::cvtColor(edges, edges3, cv::COLOR_GRAY2BGR);
        cv::bitwise_and(color, edges3, out);
        break;
    }

    case FilterType::Pixelate:
    {
        int b = std::max(2, p1);
        cv::Size small(std::max(1, bgr.cols / b), std::max(1, bgr.rows / b));
        cv::Mat tmp;
        cv::resize(bgr, tmp, small, 0, 0, cv::INTER_AREA);
        cv::resize(tmp, out, bgr.size(), 0, 0, cv::INTER_NEAREST);
        break;
    }

    case FilterType::Contours:
    {
        cv::Mat g, e;
        cv::cvtColor(bgr, g, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(g, g, cv::Size(5, 5), 1.4);
        cv::Canny(g, e, p1, p1 * 2);
        cv::dilate(e, e, cv::Mat(), cv::Point(-1, -1), 1);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(e, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        out = bgr.clone();
        int kept = 0;
        for (size_t i = 0; i < contours.size(); ++i)
        {
            if (cv::contourArea(contours[i]) < p2)
                continue;
            ++kept;
            cv::drawContours(out, contours, static_cast<int>(i), cv::Scalar(0, 255, 0), 2);
            cv::Rect r = cv::boundingRect(contours[i]);
            cv::rectangle(out, r, cv::Scalar(0, 0, 255), 1);
        }
        cv::putText(out, "Contornos: " + std::to_string(kept), cv::Point(10, 25),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 0), 2);
        break;
    }

    case FilterType::FaceDetect:
    {
        out = bgr.clone();
        m_lastFaces = 0;
        if (!EnsureFaceCascade())
        {
            cv::putText(out, "No se encontro haarcascade_frontalface_default.xml",
                        cv::Point(10, 25), cv::FONT_HERSHEY_SIMPLEX, 0.6,
                        cv::Scalar(0, 0, 255), 2);
            break;
        }
        cv::Mat g;
        cv::cvtColor(bgr, g, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(g, g);

        std::vector<cv::Rect> faces;
        m_faceCascade.detectMultiScale(g, faces, p2 / 10.0, std::max(1, p1), 0, cv::Size(40, 40));
        for (const auto& r : faces)
            cv::rectangle(out, r, cv::Scalar(0, 255, 0), 2);
        m_lastFaces = static_cast<int>(faces.size());
        cv::putText(out, "Rostros: " + std::to_string(faces.size()), cv::Point(10, 25),
                    cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 255), 2);
        break;
    }

    default:
        out = bgr.clone();
        break;
    }

    return out;
}
