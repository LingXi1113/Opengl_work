param(
  [string]$Root = "C:\Opengl_work"
)

$ErrorActionPreference = "Stop"

function Copy-Dir($src, $dst) {
  if (!(Test-Path $src)) { throw "Missing: $src" }
  New-Item -ItemType Directory -Force -Path $dst | Out-Null
  Copy-Item -Recurse -Force -Path (Join-Path $src '*') -Destination $dst
}

$codeDir    = Join-Path $Root 'code'
$releaseDir = Join-Path $Root 'release'
$demoDir    = Join-Path $Root 'demo'

New-Item -ItemType Directory -Force -Path $codeDir    | Out-Null
New-Item -ItemType Directory -Force -Path $releaseDir | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $demoDir 'images') | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $demoDir 'video')  | Out-Null

# ---- code/ : minimal buildable + runnable subset ----
Copy-Item -Force -Path (Join-Path $Root 'CMakeLists.txt') -Destination (Join-Path $codeDir 'CMakeLists.txt')
if (Test-Path (Join-Path $Root 'build_windows.sh')) {
  Copy-Item -Force -Path (Join-Path $Root 'build_windows.sh') -Destination (Join-Path $codeDir 'build_windows.sh')
}

Copy-Dir (Join-Path $Root 'cmake')          (Join-Path $codeDir 'cmake')
Copy-Dir (Join-Path $Root 'configuration') (Join-Path $codeDir 'configuration')
Copy-Dir (Join-Path $Root 'dlls')          (Join-Path $codeDir 'dlls')
Copy-Dir (Join-Path $Root 'includes')      (Join-Path $codeDir 'includes')
Copy-Dir (Join-Path $Root 'lib')           (Join-Path $codeDir 'lib')

# src subset needed by CMake (GLAD + STB_IMAGE + new_scene)
New-Item -ItemType Directory -Force -Path (Join-Path $codeDir 'src') | Out-Null
Copy-Item -Force -Path (Join-Path $Root 'src\glad.c')      -Destination (Join-Path $codeDir 'src\glad.c')
Copy-Item -Force -Path (Join-Path $Root 'src\stb_image.cpp') -Destination (Join-Path $codeDir 'src\stb_image.cpp')
Copy-Dir (Join-Path $Root 'src\new_scene') (Join-Path $codeDir 'src\new_scene')

Copy-Dir (Join-Path $Root 'resources') (Join-Path $codeDir 'resources')
Copy-Dir (Join-Path $Root 'city')      (Join-Path $codeDir 'city')
Copy-Dir (Join-Path $Root 'HOUSE')     (Join-Path $codeDir 'HOUSE')
Copy-Dir (Join-Path $Root 'BIRDS')     (Join-Path $codeDir 'BIRDS')

# ---- release/ : runnable bundle ----
# Expect build output at bin/new_scene/Release after building Release config
$binRelease = Join-Path $Root 'bin\new_scene\Release'
if (!(Test-Path $binRelease)) {
  Write-Host "Release binaries not found at: $binRelease" -ForegroundColor Yellow
  Write-Host "Build with: cmake --build build --config Release" -ForegroundColor Yellow
} else {
  Copy-Item -Force -Path (Join-Path $binRelease '*.exe') -Destination $releaseDir
  Copy-Item -Force -Path (Join-Path $binRelease '*.dll') -Destination $releaseDir -ErrorAction SilentlyContinue
  Copy-Item -Force -Path (Join-Path $binRelease '*.vs')  -Destination $releaseDir -ErrorAction SilentlyContinue
  Copy-Item -Force -Path (Join-Path $binRelease '*.fs')  -Destination $releaseDir -ErrorAction SilentlyContinue
}

# helper launcher: sets LOGL_ROOT_PATH to ../code relative to release/
$runBat = Join-Path $releaseDir 'run_new_scene.bat'
@"
@echo off
setlocal
set ROOT=%~dp0..
set LOGL_ROOT_PATH=%ROOT%\code
echo LOGL_ROOT_PATH=%LOGL_ROOT_PATH%

REM Prefer Release exe name
if exist "%~dp0new_scene.exe" (
  "%~dp0new_scene.exe"
) else if exist "%~dp0new_scene_dbg.exe" (
  "%~dp0new_scene_dbg.exe"
) else (
  echo Cannot find new_scene executable in %~dp0
  pause
)
endlocal
"@ | Set-Content -Encoding ASCII -Path $runBat

Write-Host "Packaged submission folders:" -ForegroundColor Green
Write-Host "  $codeDir" -ForegroundColor Green
Write-Host "  $releaseDir" -ForegroundColor Green
Write-Host "  $demoDir" -ForegroundColor Green
