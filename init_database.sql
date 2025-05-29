-- Script para inicializar la base de datos del cine
-- Crear todas las tablas necesarias

-- Tabla de usuarios
CREATE TABLE IF NOT EXISTS usuarios (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    nombre TEXT NOT NULL,
    correo TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    telefono TEXT,
    tipo INTEGER DEFAULT 0, -- 0 = cliente, 1 = administrador
    fecha_creacion DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Tabla de películas
CREATE TABLE IF NOT EXISTS peliculas (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    titulo TEXT NOT NULL,
    duracion INTEGER NOT NULL,
    genero TEXT NOT NULL,
    descripcion TEXT,
    clasificacion TEXT,
    fecha_estreno DATE,
    activa INTEGER DEFAULT 1
);

-- Tabla de salas
CREATE TABLE IF NOT EXISTS salas (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    nombre TEXT,
    numero_asientos INTEGER NOT NULL,
    tipo TEXT DEFAULT 'Standard',
    activa INTEGER DEFAULT 1
);

-- Tabla de asientos
CREATE TABLE IF NOT EXISTS asientos (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sala_id INTEGER NOT NULL,
    fila INTEGER NOT NULL,
    numero INTEGER NOT NULL,
    tipo TEXT DEFAULT 'Normal',
    estado INTEGER DEFAULT 0, -- 0 = libre, 1 = ocupado, 2 = fuera de servicio
    FOREIGN KEY (sala_id) REFERENCES salas(id),
    UNIQUE(sala_id, fila, numero)
);

-- Tabla de sesiones
CREATE TABLE IF NOT EXISTS sesiones (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    pelicula_id INTEGER NOT NULL,
    sala_id INTEGER NOT NULL,
    hora_inicio TEXT NOT NULL,
    hora_fin TEXT NOT NULL,
    fecha DATE NOT NULL,
    precio DECIMAL(6,2) DEFAULT 8.50,
    estado INTEGER DEFAULT 1, -- 1 = activa, 0 = cancelada
    FOREIGN KEY (pelicula_id) REFERENCES peliculas(id),
    FOREIGN KEY (sala_id) REFERENCES salas(id)
);

-- Tabla de ventas
CREATE TABLE IF NOT EXISTS ventas (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    usuario_id INTEGER NOT NULL,
    sesion_id INTEGER NOT NULL,
    fecha DATETIME DEFAULT CURRENT_TIMESTAMP,
    precio_total DECIMAL(8,2) NOT NULL,
    estado TEXT DEFAULT 'Completada',
    FOREIGN KEY (usuario_id) REFERENCES usuarios(id),
    FOREIGN KEY (sesion_id) REFERENCES sesiones(id)
);

-- Tabla de billetes
CREATE TABLE IF NOT EXISTS billetes (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    venta_id INTEGER NOT NULL,
    asiento_id INTEGER NOT NULL,
    precio DECIMAL(6,2) NOT NULL,
    fecha_emision DATETIME DEFAULT CURRENT_TIMESTAMP,
    estado TEXT DEFAULT 'Válido',
    FOREIGN KEY (venta_id) REFERENCES ventas(id),
    FOREIGN KEY (asiento_id) REFERENCES asientos(id)
);

-- Insertar usuario administrador por defecto
INSERT OR IGNORE INTO usuarios (nombre, correo, password, tipo) 
VALUES ('Administrador', 'admin@cinegestion.com', 'admin123', 1);

-- Insertar usuario cliente de prueba
INSERT OR IGNORE INTO usuarios (nombre, correo, password, telefono, tipo) 
VALUES ('Cliente Demo', 'cliente@demo.com', '123456', '666123456', 0);

-- Insertar películas de prueba
INSERT OR IGNORE INTO peliculas (titulo, duracion, genero, descripcion) VALUES
('El Padrino', 175, 'Drama', 'La saga de una familia mafiosa'),
('Pulp Fiction', 154, 'Crimen', 'Historias entrelazadas en Los Ángeles'),
('El Señor de los Anillos', 178, 'Fantasía', 'La aventura épica de Frodo'),
('Matrix', 136, 'Ciencia Ficción', 'Neo descubre la realidad'),
('Titanic', 194, 'Romance', 'Historia de amor en el barco');

-- Insertar salas de prueba
INSERT OR IGNORE INTO salas (nombre, numero_asientos) VALUES
('Sala 1', 50),
('Sala 2', 75),
('Sala 3', 100);

-- Crear asientos para Sala 1 (50 asientos)
INSERT OR IGNORE INTO asientos (sala_id, fila, numero) 
SELECT 1, 
       (ROW_NUMBER() OVER() - 1) / 10 + 1 as fila,
       (ROW_NUMBER() OVER() - 1) % 10 + 1 as numero
FROM (SELECT 1 UNION SELECT 2 UNION SELECT 3 UNION SELECT 4 UNION SELECT 5
      UNION SELECT 6 UNION SELECT 7 UNION SELECT 8 UNION SELECT 9 UNION SELECT 10) t1,
     (SELECT 1 UNION SELECT 2 UNION SELECT 3 UNION SELECT 4 UNION SELECT 5) t2
LIMIT 50;

-- Crear asientos para Sala 2 (75 asientos)
INSERT OR IGNORE INTO asientos (sala_id, fila, numero) 
SELECT 2, 
       (ROW_NUMBER() OVER() - 1) / 15 + 1 as fila,
       (ROW_NUMBER() OVER() - 1) % 15 + 1 as numero
FROM (SELECT 1 UNION SELECT 2 UNION SELECT 3 UNION SELECT 4 UNION SELECT 5
      UNION SELECT 6 UNION SELECT 7 UNION SELECT 8 UNION SELECT 9 UNION SELECT 10) t1,
     (SELECT 1 UNION SELECT 2 UNION SELECT 3 UNION SELECT 4 UNION SELECT 5
      UNION SELECT 6 UNION SELECT 7 UNION SELECT 8) t2
LIMIT 75;

-- Crear asientos para Sala 3 (100 asientos)
INSERT OR IGNORE INTO asientos (sala_id, fila, numero) 
SELECT 3, 
       (ROW_NUMBER() OVER() - 1) / 20 + 1 as fila,
       (ROW_NUMBER() OVER() - 1) % 20 + 1 as numero
FROM (SELECT 1 UNION SELECT 2 UNION SELECT 3 UNION SELECT 4 UNION SELECT 5
      UNION SELECT 6 UNION SELECT 7 UNION SELECT 8 UNION SELECT 9 UNION SELECT 10) t1,
     (SELECT 1 UNION SELECT 2 UNION SELECT 3 UNION SELECT 4 UNION SELECT 5
      UNION SELECT 6 UNION SELECT 7 UNION SELECT 8 UNION SELECT 9 UNION SELECT 10) t2
LIMIT 100;

-- Insertar sesiones de prueba
INSERT OR IGNORE INTO sesiones (pelicula_id, sala_id, hora_inicio, hora_fin, fecha, precio) VALUES
(1, 1, '18:00', '21:00', '2025-05-30', 8.50),
(2, 2, '20:30', '23:00', '2025-05-30', 9.00),
(3, 3, '16:00', '19:00', '2025-05-30', 8.50),
(4, 1, '21:30', '23:45', '2025-05-30', 8.50),
(5, 2, '17:00', '20:15', '2025-05-30', 9.50);

-- Mostrar información de inicialización
SELECT 'Base de datos inicializada correctamente' as Resultado;
SELECT COUNT(*) as Total_Usuarios FROM usuarios;
SELECT COUNT(*) as Total_Peliculas FROM peliculas;
SELECT COUNT(*) as Total_Salas FROM salas;
SELECT COUNT(*) as Total_Asientos FROM asientos;
SELECT COUNT(*) as Total_Sesiones FROM sesiones;