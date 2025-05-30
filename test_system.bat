@echo off
echo ===============================================
echo    🧪 PRUEBAS DEL SISTEMA - CINE GESTION 🧪
echo ===============================================
echo.

echo 🔍 Verificando archivos compilados...
if not exist servidor.exe (
    echo ❌ servidor.exe no encontrado. Ejecute compile_fixed.bat primero
    pause
    exit /b 1
)

if not exist cliente.exe (
    echo ❌ cliente.exe no encontrado. Ejecute compile_fixed.bat primero
    pause
    exit /b 1
)

echo ✅ Ejecutables encontrados

echo.
echo 🗃️ Verificando base de datos...
if not exist database\cine.db (
    echo ⚠️  Base de datos no existe, se creará al ejecutar el servidor
) else (
    echo ✅ Base de datos encontrada
    echo 📋 Usuarios en la base de datos:
    sqlite3 database\cine.db "SELECT '  - ' || Nombre || ' (' || CorreoElectronico || ') - ' || TipoUsuario FROM Usuarios;"
)

echo.
echo 🚀 Iniciando prueba del servidor...
echo ⚠️  El servidor se iniciará y se detendrá automáticamente
echo.
pause

echo 📊 Ejecutando servidor para verificar inicialización...
timeout /t 2 /nobreak >nul

REM Crear un pequeño script de prueba que envíe QUIT después de unos segundos
echo @echo off > temp_quit.bat
echo timeout /t 3 /nobreak ^>nul >> temp_quit.bat
echo echo QUIT ^| ncat localhost 5000 >> temp_quit.bat

start /min temp_quit.bat
servidor.exe

if errorlevel 1 (
    echo ❌ Error ejecutando el servidor
    if exist temp_quit.bat del temp_quit.bat
    pause
    exit /b 1
)

echo ✅ Servidor se ejecutó correctamente

if exist temp_quit.bat del temp_quit.bat

echo.
echo ===============================================
echo ✅ PRUEBAS COMPLETADAS EXITOSAMENTE
echo ===============================================
echo.
echo 🎯 El sistema está listo para usar:
echo.
echo 1️⃣ Terminal 1: .\servidor.exe
echo 2️⃣ Terminal 2: .\cliente.exe
echo.
echo 🔑 Credenciales para probar:
echo   👑 admin@cinegestion.com / admin123
echo   👤 juan@email.com / 123456
echo.
echo 🧪 Funcionalidades a probar:
echo   ✅ Login/Logout múltiples veces
echo   ✅ Ver películas en cartelera
echo   ✅ Crear películas (admin)
echo   ✅ Ver salas
echo   ✅ Crear salas (admin)
echo   ✅ Ver sesiones
echo.
pause