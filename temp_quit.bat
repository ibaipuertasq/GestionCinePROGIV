@echo off 
timeout /t 3 /nobreak >nul 
echo QUIT | ncat localhost 5000 
