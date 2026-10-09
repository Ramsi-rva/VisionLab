# ============================================================
# build-opencv5.ps1
#
# OpenCV 5.x + opencv_contrib
# Windows + Visual Studio 2022 + x64
#
# ============================================================

$ErrorActionPreference = "Stop"

# ============================================================
# CONFIGURACIÓN PRINCIPAL
# ============================================================

$OpenCVRoot = "C:/cv5"

# ------------------------------------------------------------
# Rutas derivadas de OpenCVRoot
# ------------------------------------------------------------

$OpenCVDir  = Join-Path $OpenCVRoot "opencv"
$BuildDir   = Join-Path $OpenCVRoot "build"
$InstallDir = Join-Path $OpenCVRoot "install"

# opencv_contrib está fuera del árbol de OpenCV
$ContribRoot = $OpenCVRoot
$ContribDir  = Join-Path $ContribRoot "opencv_contrib"

# ------------------------------------------------------------
# Repositorios
# ------------------------------------------------------------

$OpenCVRepo  = "https://github.com/opencv/opencv.git"
$ContribRepo = "https://github.com/opencv/opencv_contrib.git"

$Branch = "5.x"

# Visual Studio   "Visual Studio 17 2022"
$Generator = "Visual Studio 18 2026"

# ============================================================
# FUNCIONES
# ============================================================

function Write-Step {
    param(
        [string]$Message
    )

    Write-Host ""
    Write-Host "============================================================" `
        -ForegroundColor DarkGray

    Write-Host $Message `
        -ForegroundColor Cyan

    Write-Host "============================================================" `
        -ForegroundColor DarkGray

    Write-Host ""
}

function Test-CommandExists {
    param(
        [string]$CommandName
    )

    if (-not (Get-Command $CommandName -ErrorAction SilentlyContinue)) {

        throw "$CommandName no está disponible en PATH."
    }
}

# ============================================================
# INICIO
# ============================================================

Write-Host ""
Write-Host "============================================================"
Write-Host " OpenCV 5.x + opencv_contrib"
Write-Host " Windows + Visual Studio 2022 + x64"
Write-Host "============================================================"
Write-Host ""

Write-Host "OpenCVRoot:"
Write-Host "  $OpenCVRoot"

Write-Host ""

# ============================================================
# 1. COMPROBAR HERRAMIENTAS
# ============================================================

Write-Step "1. Comprobando herramientas"

Test-CommandExists "git"
Test-CommandExists "cmake"

Write-Host "Git:"
git --version

Write-Host ""
Write-Host "CMake:"
cmake --version | Select-Object -First 1

# ============================================================
# 2. COMPROBAR VISUAL STUDIO
# ============================================================

Write-Step "2. Comprobando Visual Studio 2022"

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (Test-Path $vswhere) {

    $VSInfo = & $vswhere `
        -latest `
        -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath

    if (-not $VSInfo) {

        throw @"
No se encontró una instalación de Visual Studio 2022
con las herramientas de compilación C++.

Instala:
Desktop development with C++
"@
    }

    Write-Host "Visual Studio encontrado:"
    Write-Host "  $VSInfo"

}
else {

    Write-Warning "No se encontró vswhere.exe."
    Write-Warning "CMake intentará utilizar Visual Studio 2022."

}

# ============================================================
# 3. CREAR DIRECTORIOS
# ============================================================

Write-Step "3. Preparando directorios"

New-Item `
    -ItemType Directory `
    -Force `
    -Path $OpenCVRoot |
    Out-Null

New-Item `
    -ItemType Directory `
    -Force `
    -Path $ContribRoot |
    Out-Null

# ============================================================
# 4. DESCARGAR OPENCV
# ============================================================

Write-Step "4. Descargando OpenCV"

if (Test-Path (Join-Path $OpenCVDir ".git")) {

    Write-Host "OpenCV ya existe."
    Write-Host "Actualizando repositorio..."

    Push-Location $OpenCVDir

    git fetch origin
    git checkout $Branch
    git pull origin $Branch

    Pop-Location

}
else {

    if (Test-Path $OpenCVDir) {

        Write-Host "Existe un directorio que no es un repositorio Git:"
        Write-Host "  $OpenCVDir"

        Write-Host ""
        Write-Host "Eliminándolo..."

        Remove-Item `
            $OpenCVDir `
            -Recurse `
            -Force
    }

    git clone `
        --branch $Branch `
        --depth 1 `
        $OpenCVRepo `
        $OpenCVDir
}

# ============================================================
# 5. DESCARGAR OPENCV_CONTRIB
# ============================================================

Write-Step "5. Descargando opencv_contrib"

if (Test-Path (Join-Path $ContribDir ".git")) {

    Write-Host "opencv_contrib ya existe."
    Write-Host "Actualizando repositorio..."

    Push-Location $ContribDir

    git fetch origin
    git checkout $Branch
    git pull origin $Branch

    Pop-Location

}
else {

    if (Test-Path $ContribDir) {

        Write-Host "Existe un directorio que no es un repositorio Git:"
        Write-Host "  $ContribDir"

        Write-Host ""
        Write-Host "Eliminándolo..."

        Remove-Item `
            $ContribDir `
            -Recurse `
            -Force
    }

    git clone `
        --branch $Branch `
        --depth 1 `
        $ContribRepo `
        $ContribDir
}

# ============================================================
# 6. MOSTRAR VERSIONES
# ============================================================

Write-Step "6. Verificando versiones"

Write-Host "OpenCV:"
Push-Location $OpenCVDir
git branch --show-current
git log -1 --oneline
Pop-Location

Write-Host ""

Write-Host "opencv_contrib:"
Push-Location $ContribDir
git branch --show-current
git log -1 --oneline
Pop-Location

# ============================================================
# 7. VERIFICAR CONTRIB
# ============================================================

Write-Step "7. Verificando opencv_contrib"

$ContribModules = Join-Path $ContribDir "modules"

if (-not (Test-Path $ContribModules)) {

    throw "No existe el directorio de módulos contrib:"
}

Write-Host "Módulos contrib:"
Write-Host "  $ContribModules"

# ============================================================
# 8. PREPARAR BUILD
# ============================================================

Write-Step "8. Preparando directorio de compilación"

if (Test-Path $BuildDir) {

    Write-Host "Eliminando compilación anterior:"
    Write-Host "  $BuildDir"

    Remove-Item `
        $BuildDir `
        -Recurse `
        -Force
}

New-Item `
    -ItemType Directory `
    -Force `
    -Path $BuildDir |
    Out-Null

# ============================================================
# 9. EVITAR IN-SOURCE BUILD
# ============================================================

$SourceCache = Join-Path $OpenCVDir "CMakeCache.txt"

if (Test-Path $SourceCache) {

    throw @"
ERROR: Se encontró:

$SourceCache

Esto indica que anteriormente se ejecutó CMake
dentro del directorio fuente.

Elimina ese archivo antes de continuar.
"@
}

$SourceCMakeFiles = Join-Path $OpenCVDir "CMakeFiles"

if (Test-Path $SourceCMakeFiles) {

    throw @"
ERROR: Se encontró:

$SourceCMakeFiles

OpenCV contiene archivos de una compilación in-source.

Limpia esa compilación antes de continuar.
"@
}

# ============================================================
# 10. CONFIGURAR CMAKE
# ============================================================

Write-Step "10. Configurando CMake"

Push-Location $BuildDir

$CMakeArgs = @(

    "-G", $Generator,
    "-A", "x64",

    "-DCMAKE_CXX_STANDARD=17",

    "-DBUILD_WITH_DEBUG_INFO=ON",

    "-DCMAKE_CXX_STANDARD_REQUIRED=ON",

    "-DOPENCV_EXTRA_MODULES_PATH=$ContribModules",

    "-DBUILD_TESTS=OFF",

    "-DBUILD_PERF_TESTS=OFF",

    "-DBUILD_opencv_world=ON",

    "-DBUILD_EXAMPLES=ON",

    "-DBUILD_DOCS=OFF",

    "-DBUILD_opencv_apps=ON",

    "-DBUILD_opencv_python3=OFF",

    "-DBUILD_opencv_java=OFF",

    "-DWITH_VTK=OFF",

    "-DWITH_OPENCL=OFF",

    "-DWITH_TBB=OFF",

    "-DWITH_OPENMP=OFF",

    "-DWITH_FFMPEG=ON",

    "-DCMAKE_INSTALL_PREFIX=$InstallDir",

    $OpenCVDir
)

Write-Host "Directorio BUILD:"
Write-Host "  $(Get-Location)"

Write-Host ""
Write-Host "Directorio SOURCE:"
Write-Host "  $OpenCVDir"

Write-Host ""
Write-Host "Directorio CONTRIB:"
Write-Host "  $ContribModules"

Write-Host ""
Write-Host "Directorio INSTALL:"
Write-Host "  $InstallDir"

Write-Host ""
Write-Host "Ejecutando CMake..."
Write-Host ""

cmake @CMakeArgs

if ($LASTEXITCODE -ne 0) {

    Pop-Location

    throw "La configuración de CMake falló."
}

# ============================================================
# 11. VERIFICAR CMAKECACHE
# ============================================================

Write-Step "11. Verificando configuración"

$CacheFile = Join-Path $BuildDir "CMakeCache.txt"

if (-not (Test-Path $CacheFile)) {

    Pop-Location

    throw "No se encontró CMakeCache.txt."
}

Write-Host "CMakeCache:"
Write-Host "  $CacheFile"

Write-Host ""

$ContribEntry = Get-Content $CacheFile |
    Where-Object {
        $_ -like "OPENCV_EXTRA_MODULES_PATH:*"
    }

if ($ContribEntry) {

    Write-Host "opencv_contrib detectado:" `
        -ForegroundColor Green

    Write-Host $ContribEntry

}
else {

    Write-Warning `
        "No se encontró OPENCV_EXTRA_MODULES_PATH."

}

# ============================================================
# 12. COMPILAR RELEASE
# ============================================================

Write-Step "12. Compilando Release"

$Jobs = [Environment]::ProcessorCount

Write-Host "Procesadores disponibles: $Jobs"
Write-Host ""

cmake `
    --build . `
    --config Release `
    --parallel $Jobs

if ($LASTEXITCODE -ne 0) {

    Pop-Location

    throw "La compilación falló."
}

# ============================================================
# 13. INSTALAR
# ============================================================

Write-Step "13. Instalando OpenCV"

cmake `
    --build . `
    --config Release `
    --target INSTALL `
    --parallel $Jobs

if ($LASTEXITCODE -ne 0) {

    Pop-Location

    throw "La instalación falló."
}

Pop-Location

# ============================================================
# 14. VERIFICAR INSTALACIÓN
# ============================================================

Write-Step "14. Verificando instalación"

$VersionExe = Get-ChildItem `
    $InstallDir `
    -Filter "opencv_version.exe" `
    -Recurse `
    -ErrorAction SilentlyContinue |
    Select-Object -First 1

if ($VersionExe) {

    Write-Host ""
    Write-Host "OpenCV instalado correctamente." `
        -ForegroundColor Green

    Write-Host ""
    Write-Host "Ejecutable:"
    Write-Host "  $($VersionExe.FullName)"

    Write-Host ""
    Write-Host "Versión:"
    & $VersionExe.FullName

}
else {

    Write-Warning `
        "No se encontró opencv_version.exe."

}

# ============================================================
# 15. RESUMEN FINAL
# ============================================================

Write-Host ""
Write-Host "============================================================"
Write-Host "           OPENCV 5 - PROCESO TERMINADO"
Write-Host "============================================================"
Write-Host ""

Write-Host "OpenCVRoot:"
Write-Host "  $OpenCVRoot"

Write-Host ""
Write-Host "Source:"
Write-Host "  $OpenCVDir"

Write-Host ""
Write-Host "Build:"
Write-Host "  $BuildDir"

Write-Host ""
Write-Host "Contrib:"
Write-Host "  $ContribDir"

Write-Host ""
Write-Host "Install:"
Write-Host "  $InstallDir"

Write-Host ""
Write-Host "============================================================"