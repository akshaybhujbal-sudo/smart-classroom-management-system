@echo off
REM Launcher script for Python backend
cd /d "%~dp0.."
echo Starting Smart Classroom Management System (Python Backend)...
python backend_python\app.py --port 8080
pause
