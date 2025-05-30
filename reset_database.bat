@echo off
echo ===============================================
echo     RESET BASE DE DATOS - CINE GESTION 
echo ===============================================
echo.
echo   ADVERTENCIA: Esto eliminara TODOS los datos
echo.
pause

echo  Eliminando base de datos actual...
if exist database\cine.db (
    del database\cine.db
    echo  Base de datos eliminada
) else (
    echo   No habia base de datos previa
)

echo.
echo  Creando directorios necesarios...
if not exist database mkdir database
if not exist logs mkdir logs
echo  Directorios creados

echo.
echo  Compilando y ejecutando servidor para recrear BD...
echo.
echo   El servidor iniciara automáticamente y se cerrara
echo     cuando la BD esté lista.
echo.
pause

gcc -Wall -Wextra -std=c11 -g -D_WIN32_WINNT=0x0601 ^
    -Wno-unknown-pragmas -Wno-unused-parameter -Wno-unused-but-set-variable ^
    -I. -Ilib -Imodels -Iutils ^
    -o temp_servidor.exe ^
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
    echo  Error compilando
    pause
    exit /b 1
)

echo  Iniciando servidor para crear BD...
echo.
timeout /t 2 /nobreak >nul
start /wait temp_servidor.exe

echo.
echo  Limpiando archivo temporal...
if exist temp_servidor.exe del temp_servidor.exe

echo.
echo ===============================================
echo  BASE DE DATOS RESETEADA EXITOSAMENTE
echo ===============================================
echo.
echo Nuevas credenciales:
echo    Admin: admin@cinegestion.com / admin123
echo    Cliente: juan@email.com / 123456
echo    Cliente: maria@email.com / 123456
echo    Cliente: pedro@email.com / 123456
echo    Admin: ana@cinegestion.com / admin456
echo.
echo Ahora puede ejecutar el servidor normalmente
echo.
pause