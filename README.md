# 🎬 Sistema de Gestión de Cine - Cliente-Servidor

## 📋 Descripción

Sistema cliente-servidor para gestión de cine desarrollado en C (servidor) y C++ (cliente) con arquitectura orientada a objetos que incluye herencia, polimorfismo y patrones de diseño.

## 🏗️ Arquitectura

### **Servidor (C)**
- Reutiliza toda la funcionalidad del Hito 2
- Maneja la lógica de negocio y acceso a datos
- Comunicación por sockets (send/recv)
- Sistema de autenticación y permisos

### **Cliente (C++)**
- Interfaz orientada a objetos
- Jerarquía de clases con herencia múltiple
- Polimorfismo con métodos virtuales
- Factory Pattern para creación de usuarios

## 🎯 Características Orientadas a Objetos

### **Jerarquía de Herencia**
Entity (abstract)
├── User (abstract)
│   ├── AdminUser
│   └── ClientUser
├── Movie
├── Room
├── Session
└── Sale

### **Polimorfismo**
- Métodos virtuales puros en clases base
- Comportamiento específico por tipo de usuario
- Factory Pattern para creación de objetos
- Dynamic casting para funcionalidades específicas

## 🚀 Instalación y Ejecución

### **Requisitos**
- GCC (para servidor en C)
- G++ (para cliente en C++)
- SQLite3
- Make

### **Compilación**
```bash
# Compilar ambos
make

# O individualmente
make servidor
make cliente


Comandos Disponibles

LOGIN:email:password → Autenticación
GET_MOVIES → Obtener todas las películas
CREATE_MOVIE:titulo:duracion:genero → Crear película
GET_ROOMS → Obtener todas las salas
CREATE_ROOM:num_asientos → Crear sala
GET_SESSIONS → Obtener todas las sesiones
GET_SESSIONS_BY_MOVIE:movie_id → Sesiones por película
GET_USER_PURCHASES:user_id → Compras de usuario
QUIT → Desconectar

Respuestas del Servidor

OK:datos → Operación exitosa
ERROR:mensaje → Error en la operación
BYE → Confirmación de desconexión