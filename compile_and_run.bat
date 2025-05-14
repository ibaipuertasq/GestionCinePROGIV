@echo off
echo Compilando CineGestion...

cd hito2

gcc -o ../cinegestion.exe ^
    src/main.c ^
    src/config.c ^
    src/database.c ^
    src/auth.c ^
    src/menu.c ^
    src/utils/logger.c ^
    src/utils/memory.c ^
    src/models/usuario.c ^
    src/models/pelicula.c ^
    src/models/sala.c ^
    src/models/asiento.c ^
    src/models/sesion.c ^
    src/models/billete.c ^
    src/models/venta.c ^
    src/test_data.c ^
    lib/sqlite3.c ^
    -I. ^
    -Ilib

cd ..

if %ERRORLEVEL% EQU 0 (
    echo Compilación exitosa!
    
    rem Crear estructura de directorios si no existe
    if not exist hito2\config mkdir hito2\config
    if not exist hito2\data mkdir hito2\data
    if not exist hito2\data\backup mkdir hito2\data\backup
    if not exist hito2\logs mkdir hito2\logs
    
    rem Crear archivos de configuración si no existen
    if not exist hito2\config\config.ini (
        echo [general] > hito2\config\config.ini
        echo app_name=CineGestion >> hito2\config\config.ini
        echo version=1.0 >> hito2\config\config.ini
        echo. >> hito2\config\config.ini
        echo [database] >> hito2\config\config.ini
        echo db_path=data/cine.db >> hito2\config\config.ini
        echo db_backup_path=data/backup/cine_backup.db >> hito2\config\config.ini
        echo. >> hito2\config\config.ini
        echo [logs] >> hito2\config\config.ini
        echo log_path=logs/system.log >> hito2\config\config.ini
        echo log_level=INFO >> hito2\config\config.ini
        echo. >> hito2\config\config.ini
        echo [ui] >> hito2\config\config.ini
        echo max_menu_items=10 >> hito2\config\config.ini
        echo clear_screen=true >> hito2\config\config.ini
    )
    
    if not exist hito2\config\admin.ini (
        echo [admin] > hito2\config\admin.ini
        echo username=admin >> hito2\config\admin.ini
        echo password=admin123 >> hito2\config\admin.ini
        echo email=admin@cinegestion.com >> hito2\config\admin.ini
        echo. >> hito2\config\admin.ini
        echo [security] >> hito2\config\admin.ini
        echo session_timeout=30 >> hito2\config\admin.ini
        echo max_login_attempts=3 >> hito2\config\admin.ini
    )
    
    echo.
    echo Ejecutando programa...
    echo -----------------------------
    cinegestion.exe
) else (
    echo Error en la compilación!
    pause
)