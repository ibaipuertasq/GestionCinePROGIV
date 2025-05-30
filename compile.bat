@echo off
cls
echo ===============================================
echo    🔧 COMPILACIÓN FINAL - CINE GESTION 🔧
echo ===============================================
echo.

echo 🧹 Limpiando archivos previos...
if exist servidor.exe del servidor.exe
if exist cliente.exe del cliente.exe
if exist *.o del *.o

echo.
echo 🔧 Compilando servidor con DEBUG mejorado...
gcc -Wall -Wextra -std=c11 -g -D_WIN32_WINNT=0x0601 ^
    -Wno-unknown-pragmas -Wno-unused-parameter -Wno-unused-but-set-variable ^
    -I. -Ilib -Imodels -Iutils ^
    -o servidor.exe ^
    servidor.c ^
    config.c ^
    auth.c ^
    test_data.c ^
    models/usuario.c ^
    models/pelicula.c ^
    models/sala.c ^
    models/sesion.c ^
    models/venta.c ^
    models/billete.c ^
    models/asiento.c ^
    utils/logger.c ^
    utils/memory.c ^
    utils/database.c ^
    lib/sqlite3.c ^
    -lws2_32 -lm

if errorlevel 1 (
    echo ❌ Error compilando servidor
    pause
    exit /b 1
)

echo ✅ Servidor compilado exitosamente

echo.
echo 🔧 Compilando cliente con reconexión automática...
g++ -Wall -Wextra -std=c++17 -g -D_WIN32_WINNT=0x0601 ^
    -Wno-unknown-pragmas ^
    -I. -Ilib -Imodels -Iutils ^
    -o cliente.exe ^
    cliente.cpp ^
    -lws2_32 -lm

if errorlevel 1 (
    echo ❌ Error compilando cliente
    pause
    exit /b 1
)

echo ✅ Cliente compilado exitosamente

echo.
echo 🗃️ Verificando/Corrigiendo base de datos...

REM Verificar si existe la BD y corregir correos si es necesario
if exist database\cine.db (
    echo 📧 Corrigiendo correos electrónicos en BD existente...
    sqlite3 database\cine.db "UPDATE Usuarios SET CorreoElectronico = 'juan@email.com' WHERE CorreoElectronico = 'juan@example.com';"
    sqlite3 database\cine.db "UPDATE Usuarios SET CorreoElectronico = 'maria@email.com' WHERE CorreoElectronico = 'maria@example.com';"
    sqlite3 database\cine.db "UPDATE Usuarios SET CorreoElectronico = 'pedro@email.com' WHERE CorreoElectronico = 'carlos@example.com';"
    echo ✅ Correos corregidos
) else (
    echo 📁 Creando directorios necesarios...
    if not exist database mkdir database
    if not exist logs mkdir logs
    echo ✅ Directorios creados
)

echo.
echo ===============================================
echo ✅ COMPILACIÓN COMPLETADA EXITOSAMENTE
echo ===============================================
echo.
echo 🚀 Para ejecutar:
echo   1. Abrir terminal 1: .\servidor.exe
echo   2. Abrir terminal 2: .\cliente.exe
echo.
echo 🔑 Credenciales de prueba:
echo   👑 Admin: admin@cinegestion.com / admin123
echo   👤 Cliente: juan@email.com / 123456
echo   👤 Cliente: maria@email.com / 123456
echo   👤 Cliente: pedro@email.com / 123456
echo   👤 Admin: ana@cinegestion.com / admin456
echo.
echo 🎯 Mejoras aplicadas:
echo   ✅ Cliente con reconexión automática
echo   ✅ Servidor con DEBUG mejorado
echo   ✅ Manejo de sesiones corregido
echo   ✅ Correos electrónicos arreglados
echo   ✅ Sistema de autenticación persistente
echo.
echo ¡El sistema debería funcionar perfectamente ahora!
echo.
pause