// pch.h - encabezado precompilado (MFC + OpenCV)
#pragma once

#include "framework.h"

// ---- OpenCV 5.x compilado desde código fuente en C:\cv5 ----
// Solo se incluyen los módulos que usa el proyecto.
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/geometry.hpp>   // OpenCV 5: contourArea y boundingRect pasaron al módulo geometry
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/xobjdetect.hpp>
