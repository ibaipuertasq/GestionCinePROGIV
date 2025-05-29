@echo off
echo 🔧 Compilando proyecto Cliente-Servidor...

echo Compilando servidor...
gcc -Wall -Wextra -std=c11 -g -Ilib -Imodels -Iutils -I. -o servidor.exe servidor.c config.c auth.c test_data.c models/usuario.c models/pelicula.c models/sala.c models/sesion.c models/venta.c models/billete.c models/asiento.c utils/logger.c utils/memory.c utils/database.c lib/sqlite3.c -lws2_32 -lm

echo Compilando cliente...
g++ -Wall -Wextra -std=c++17 -g -Ilib -Imodels -Iutils -I. -o cliente.exe cliente.cpp -lws2_32

echo ✅ Compilación completada
pause