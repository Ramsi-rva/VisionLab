# publish-github.ps1 - crea el repositorio local y lo sube a GitHub.
# Uso:  .\scripts\publish-github.ps1 [-Name VisionLab] [-Private]
param([string]$Name = "VisionLab", [switch]$Private)
$ErrorActionPreference = "Stop"
Set-Location (Split-Path -Parent $PSScriptRoot)

if (-not (Get-Command git -ErrorAction SilentlyContinue)) { throw "Instala Git for Windows: https://git-scm.com" }
if (-not (Test-Path .git)) { git init -b main }
git add -A
git commit -m "VisionLab: app MFC con OpenCV 5 compilado desde el codigo fuente" 2>&1 | Out-Host

if (-not (Get-Command gh -ErrorAction SilentlyContinue)) {
    Write-Host "`nNo hay GitHub CLI. Crea un repo vacio '$Name' en https://github.com/new y ejecuta:" -ForegroundColor Yellow
    Write-Host "  git remote add origin https://github.com/TU_USUARIO/$Name.git`n  git push -u origin main"
    return
}
gh auth status 2>$null
if ($LASTEXITCODE -ne 0) { gh auth login }
$vis = if ($Private) { "--private" } else { "--public" }
gh repo create $Name $vis --source . --remote origin --push
