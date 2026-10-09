# ============================================================
# build-visionlab.ps1
# Compila VisionLab (MFC + OpenCV 5) desde PowerShell, sin abrir
# Visual Studio. Equivale a: Compilar > Compilar solución (Release|x64).
#
# Uso:   .\scripts\build-visionlab.ps1            (compila)
#        .\scripts\build-visionlab.ps1 -Run       (compila y ejecuta)
# ============================================================
param([switch]$Run)

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$Solution    = Join-Path $ProjectRoot "VisionLab.sln"
$OpenCVRoot  = "C:\cv5\install"

# 1. Comprobar que OpenCV está instalado
if (-not (Test-Path "$OpenCVRoot\x64\vc18\lib\opencv_world510.lib")) {
    throw "No se encontró opencv_world510.lib en $OpenCVRoot. Ejecuta primero build-opencv5.ps1."
}

# 2. Localizar MSBuild con vswhere (viene con Visual Studio)
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild `
            -find "MSBuild\**\Bin\MSBuild.exe" | Select-Object -First 1
if (-not $msbuild) { throw "No se encontró MSBuild. Instala 'Desarrollo para el escritorio con C++'." }

Write-Host "MSBuild: $msbuild"

# 3. Compilar
& $msbuild $Solution /p:Configuration=Release /p:Platform=x64 /m /nologo /verbosity:minimal
if ($LASTEXITCODE -ne 0) { throw "La compilación falló." }

$exe = Join-Path $ProjectRoot "bin\x64\Release\VisionLab.exe"
Write-Host "`nListo: $exe" -ForegroundColor Green

if ($Run) { Start-Process $exe }
