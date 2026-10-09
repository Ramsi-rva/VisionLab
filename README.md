# VisionLab

Aplicación MFC (C++17, Visual Studio 2026) que usa **OpenCV 5.1.0-dev compilado desde el código fuente**
(`C:\cv5`) para aplicar filtros de visión por computadora a imágenes y a la cámara web.

## Cómo compilar y ejecutar

1. Tener OpenCV instalado en `C:\cv5\install` (script: `scripts\build-opencv5.ps1`).
2. Abrir `VisionLab.sln` en Visual Studio, elegir **Release | x64** y pulsar **Compilar > Compilar solución**.
   (o desde PowerShell: `.\scripts\build-visionlab.ps1 -Run`)
3. El ejecutable queda en `bin\x64\Release\VisionLab.exe`; la compilación copia junto a él
   `opencv_world510.dll`, `opencv_videoio_ffmpeg510_64.dll` y `haarcascade_frontalface_default.xml`.

Si OpenCV está en otra carpeta, cambiar la propiedad `OpenCV5Root` al inicio de `VisionLab.vcxproj`.

## Estructura

| Archivo | Contenido |
|---|---|
| `ImageProcessor.h/.cpp` | Lógica de OpenCV (filtros, carga/guardado, detector de rostros). No usa MFC. |
| `VisionLabDlg.h/.cpp` | Ventana principal: eventos, sliders, cámara, dibujo de `cv::Mat`. |
| `VisionLab.h/.cpp` | Clase de la aplicación MFC. |
| `VisionLab.rc`, `resource.h` | Diseño del cuadro de diálogo. |
| `scripts/` | Compilación de OpenCV y del proyecto. |
| `docs/` | Ejemplo de resultado guardado por la aplicación. |

Solo se incluye la configuración **Release | x64** porque OpenCV se compiló únicamente en Release
(mezclar con el CRT de depuración no es válido).
