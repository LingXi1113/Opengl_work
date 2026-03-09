param(
  [string]$Root = "C:\Opengl_work"
)

$ErrorActionPreference = "Continue"

function Try-Remove($path) {
  if (!(Test-Path $path)) { return }
  try {
    Remove-Item -Recurse -Force $path
    Write-Host "Removed: $path" -ForegroundColor Green
  } catch {
    Write-Host "Skip (in use?): $path" -ForegroundColor Yellow
    Write-Host $_.Exception.Message -ForegroundColor DarkYellow
  }
}

# CMake / VS intermediate
Try-Remove (Join-Path $Root 'build')
Try-Remove (Join-Path $Root '.vs')

# VS Code / tooling cache (may be locked while VS Code is open)
Try-Remove (Join-Path $Root 'out')

# Regeneratable new_scene outputs (final exe already packaged in release/)
Try-Remove (Join-Path $Root 'bin\new_scene\Debug')
Try-Remove (Join-Path $Root 'bin\new_scene\Release')

Write-Host "Done. If 'out' cannot be removed, close VS Code and rerun this script." -ForegroundColor Cyan
