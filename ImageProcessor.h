// ImageProcessor.h
// Capa de procesamiento de imagen basada ÚNICAMENTE en OpenCV (sin código MFC).
// La interfaz gráfica (VisionLabDlg) solo llama a esta clase, de modo que la
// lógica de visión por computadora queda separada de la ventana.
#pragma once

#include <opencv2/core.hpp>
// En OpenCV 5.x la clase CascadeClassifier (Haar/LBP) se movió al módulo
// contrib "xobjdetect", por eso se incluye este encabezado y no objdetect.hpp.
#include <opencv2/xobjdetect.hpp>
#include <string>
#include <vector>

enum class FilterType
{
    Original = 0,
    Gray,
    Negative,
    BrightnessContrast,
    GaussianBlur,
    Equalize,
    Sepia,
    Threshold,
    Canny,
    Cartoon,
    Pixelate,
    Contours,
    FaceDetect,
    Count
};

// Descripción de un filtro y de sus dos parámetros ajustables (sliders).
struct FilterInfo
{
    const wchar_t* name;
    const wchar_t* p1Name;  // nullptr => el parámetro no se usa
    int p1Min, p1Max, p1Def;
    const wchar_t* p2Name;
    int p2Min, p2Max, p2Def;
};

class ImageProcessor
{
public:
    static const FilterInfo& Info(FilterType f);
    static int FilterCount() { return static_cast<int>(FilterType::Count); }

    // Aplica el filtro a una imagen BGR de 8 bits y 3 canales.
    // Siempre devuelve una imagen BGR de 8 bits y 3 canales.
    cv::Mat Apply(const cv::Mat& bgr, FilterType f, int p1, int p2);

    // Carga/guardado seguros con rutas Unicode (imread/imwrite de OpenCV
    // usan std::string y fallan con ciertos caracteres no ASCII en Windows).
    static cv::Mat LoadImage(const std::wstring& path);
    static bool    SaveImage(const std::wstring& path, const cv::Mat& img);

    // Número de rostros encontrados en la última llamada a Apply(FaceDetect).
    int LastFaceCount() const { return m_lastFaces; }

    // Para mostrar en la barra de estado.
    static std::string OpenCVVersion();

private:
    bool EnsureFaceCascade();

    cv::CascadeClassifier m_faceCascade;
    bool                  m_cascadeTried = false;
    bool                  m_cascadeOk    = false;
    int                   m_lastFaces    = 0;
};
