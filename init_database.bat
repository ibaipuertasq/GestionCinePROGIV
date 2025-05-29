@echo off
echo ===================================
echo   INICIALIZANDO BASE DE DATOS
echo ===================================

REM Crear directorio database si no existe
if not exist database mkdir database

REM Eliminar base de datos anterior si existe
if exist database\cine.db (
    echo Eliminando base de datos anterior...
    del database\cine.db
)

REM Verificar si SQLite3 está disponible
where sqlite3 >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: sqlite3.exe no encontrado
    echo Descarga SQLite3 desde: https://www.sqlite.org/download.html
    echo Y colócalo en tu PATH o en este directorio
    pause
    exit /b 1
)

REM Ejecutar script SQL
echo Ejecutando script de inicialización...
sqlite3 database\cine.db < init_database.sql

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ✅ Base de datos creada exitosamente en: database\cine.db
    echo.
    echo USUARIOS CREADOS:
    echo - admin@cinegestion.com / admin123 (Administrador)
    echo - cliente@demo.com / 123456 (Cliente)
    echo.
    echo La base de datos incluye:
    echo - 5 películas de prueba
    echo - 3 salas con asientos
    echo - 5 sesiones de prueba
    echo.
) else (
    echo ❌ Error creando la base de datos
    pause
    exit /b 1
)

echo Presiona cualquier tecla para continuar...
pause