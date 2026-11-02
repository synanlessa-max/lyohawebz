@echo off
rem ============================================================
rem  LOBO source checker / fixer - DOUBLE CLICK this file
rem  from your SOURCE folder (where csgo-sdk.sln is).
rem  Do NOT run it from inside the zip - extract the zip first!
rem  Results are saved to check_result.txt next to this file.
rem ============================================================
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0check_source.ps1"
echo.
pause
