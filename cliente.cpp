#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <memory>
#include <cstring>
#include <set>
#include <cmath>
#include <iomanip>

// Includes específicos para Windows
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define PORT 5000
#define MAX_BUFFER 4096

// ============================================================================
// CLIENTE DE RED CON RECONEXIÓN AUTOMÁTICA
// ============================================================================

class NetworkClient {
private:
    int sock;
    bool connected;
    
    bool reconectar() {
        if (connected) {
            close(sock);
            connected = false;
        }
        
        return conectar();
    }
    
public:
    NetworkClient() : sock(0), connected(false) {}
    
    ~NetworkClient() {
        if (connected) {
            desconectar();
        }
    }
    
    bool conectar() {
        struct sockaddr_in serv_addr;
        
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cout << "Error creando socket" << std::endl;
            return false;
        }
        
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        
        #ifdef _WIN32
            serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            if (serv_addr.sin_addr.s_addr == INADDR_NONE) {
                std::cout << "Direccion invalida" << std::endl;
                return false;
            }
        #else
            if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
                std::cout << "Direccion invalida" << std::endl;
                return false;
            }
        #endif
        
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            std::cout << "Error conectando al servidor" << std::endl;
            return false;
        }
        
        connected = true;
        return true;
    }
    
    std::string enviarComando(const std::string& comando) {
        // Intentar hasta 3 veces con reconexión
        for (int intento = 0; intento < 3; intento++) {
            if (!connected && !reconectar()) {
                return "ERROR:No se pudo conectar al servidor";
            }
            
            // Enviar comando
            if (send(sock, comando.c_str(), comando.length(), 0) < 0) {
                std::cout << "Error enviando, reintentando..." << std::endl;
                connected = false;
                continue;
            }
            
            // Recibir respuesta
            char buffer[MAX_BUFFER] = {0};
            int bytes_received = recv(sock, buffer, MAX_BUFFER - 1, 0);
            
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                
                // El servidor desconecta después de cada comando
                close(sock);
                connected = false;
                
                return std::string(buffer);
            } else {
                std::cout << "Sin respuesta del servidor, reintentando..." << std::endl;
                connected = false;
            }
        }
        
        return "ERROR:Sin respuesta del servidor despues de varios intentos";
    }
    
    void desconectar() {
        if (connected) {
            close(sock);
            connected = false;
        }
    }
    
    bool estaConectado() const { return connected; }
};

// ============================================================================
// JERARQUÍA DE CLASES CON HERENCIA Y POLIMORFISMO
// ============================================================================

// Clase base abstracta Entity
class Entity {
protected:
    int id;
    
public:
    Entity(int id = 0) : id(id) {}
    virtual ~Entity() = default;
    
    // Métodos virtuales puros - polimorfismo
    virtual void mostrar() const = 0;
    virtual std::string getTipo() const = 0;
    
    // Getters y setters comunes
    int getId() const { return id; }
    void setId(int newId) { id = newId; }
};

// Clase base abstracta User (hereda de Entity)
class User : public Entity {
protected:
    std::string nombre;
    std::string correo;
    std::string telefono;
    
public:
    User(int id = 0, const std::string& nombre = "", const std::string& correo = "", 
         const std::string& telefono = "")
        : Entity(id), nombre(nombre), correo(correo), telefono(telefono) {}
    
    // Métodos virtuales puros - cada tipo de usuario los implementa diferente
    virtual void mostrarMenu() = 0;
    virtual std::vector<std::string> getPermisos() const = 0;
    virtual bool puedeGestionar(const std::string& recurso) const = 0;
    virtual std::string getRol() const = 0;
    
    // Método virtual que puede ser sobrescrito
    void mostrar() const override {
        std::cout << "Usuario: " << nombre << " (" << correo << ")" << std::endl;
    }
    
    std::string getTipo() const override { return "User"; }
    
    // Getters comunes
    const std::string& getNombre() const { return nombre; }
    const std::string& getCorreo() const { return correo; }
    const std::string& getTelefono() const { return telefono; }
};

// Declaraciones adelantadas de funciones de menú
void menuAdmin();
void menuCliente();
void menuGestionPeliculas();
void menuGestionSalas();
void menuGestionSesiones();
void mostrarCartelera();
void mostrarMisCompras();
void comprarEntradas();
void mostrarAsientos(int sesion_id);
void menuGestionUsuarios();
void mostrarDetalleCompra(int venta_id);
std::vector<std::string> parsearRespuesta(const std::string& respuesta);

// Clase AdminUser (hereda de User)
class AdminUser : public User {
public:
    AdminUser(int id = 0, const std::string& nombre = "", const std::string& correo = "", 
              const std::string& telefono = "")
        : User(id, nombre, correo, telefono) {}
    
    void mostrar() const override {
        std::cout << "ADMINISTRADOR: " << nombre << " (" << correo << ")" << std::endl;
        std::cout << "   Permisos: Gestion completa del sistema" << std::endl;
    }
    
    std::string getTipo() const override { return "AdminUser"; }
    std::string getRol() const override { return "Administrador"; }
    
    std::vector<std::string> getPermisos() const override {
        return {
            "GESTIONAR_PELICULAS",
            "GESTIONAR_SALAS", 
            "GESTIONAR_SESIONES",
            "GESTIONAR_USUARIOS",
            "VER_REPORTES",
            "CONFIGURAR_SISTEMA"
        };
    }
    
    bool puedeGestionar(const std::string& recurso) const override {
        return true;
    }
    
    void mostrarMenu() override {
        menuAdmin();
    }
};

// Clase ClientUser (hereda de User)
class ClientUser : public User {
private:
    double saldo;
    
public:
    ClientUser(int id = 0, const std::string& nombre = "", const std::string& correo = "", 
               const std::string& telefono = "", double saldo = 0.0)
        : User(id, nombre, correo, telefono), saldo(saldo) {}
    
    void mostrar() const override {
        std::cout << "CLIENTE: " << nombre << " (" << correo << ")" << std::endl;
        std::cout << "   Saldo disponible: " << saldo << " euros" << std::endl;
    }
    
    std::string getTipo() const override { return "ClientUser"; }
    std::string getRol() const override { return "Cliente"; }
    
    std::vector<std::string> getPermisos() const override {
        return {
            "VER_CARTELERA",
            "COMPRAR_ENTRADAS",
            "VER_MIS_COMPRAS",
            "MODIFICAR_PERFIL"
        };
    }
    
    bool puedeGestionar(const std::string& recurso) const override {
        return (recurso == "PERFIL" || recurso == "MIS_COMPRAS");
    }
    
    void mostrarMenu() override {
        menuCliente();
    }
    
    bool tieneSaldoSuficiente(double precio) const {
        return saldo >= precio;
    }
    
    void restarSaldo(double cantidad) {
        if (cantidad <= saldo) {
            saldo -= cantidad;
        }
    }
    
    double getSaldo() const { return saldo; }
    void setSaldo(double nuevo_saldo) { saldo = nuevo_saldo; }
};

// Clase Movie
class Movie : public Entity {
private:
    std::string titulo;
    int duracion;
    std::string genero;
    
public:
    Movie(int id = 0, const std::string& titulo = "", int duracion = 0, const std::string& genero = "")
        : Entity(id), titulo(titulo), duracion(duracion), genero(genero) {}
    
    void mostrar() const override {
        std::cout << id << ". " << titulo << " (" << duracion << " min, " << genero << ")" << std::endl;
    }
    
    std::string getTipo() const override { return "Movie"; }
    
    const std::string& getTitulo() const { return titulo; }
    int getDuracion() const { return duracion; }
    const std::string& getGenero() const { return genero; }
    
    static std::vector<Movie> obtenerTodas();
    static bool crear(const std::string& titulo, int duracion, const std::string& genero);
};

class Room : public Entity {
private:
    int num_asientos;
    int asientos_libres;
    
public:
    Room(int id = 0, int asientos = 0, int libres = 0) 
        : Entity(id), num_asientos(asientos), asientos_libres(libres) {}
    
    void mostrar() const override {
        std::cout << "Sala " << id << " - " << num_asientos << " asientos ("
                  << asientos_libres << " libres)" << std::endl;
    }
    
    std::string getTipo() const override { return "Room"; }
    
    int getNumAsientos() const { return num_asientos; }
    int getAsientosLibres() const { return asientos_libres; }
    
    static std::vector<Room> obtenerTodas();
    static bool crear(int num_asientos);
};

class Session : public Entity {
private:
    int pelicula_id;
    int sala_id;
    std::string hora_inicio;
    std::string hora_fin;
    
public:
    Session(int id = 0, int pid = 0, int sid = 0, const std::string& inicio = "", const std::string& fin = "")
        : Entity(id), pelicula_id(pid), sala_id(sid), hora_inicio(inicio), hora_fin(fin) {}
    
    void mostrar() const override {
        std::cout << "Sesion " << id << " - Pelicula: " << pelicula_id 
                  << ", Sala: " << sala_id << ", " << hora_inicio << " - " << hora_fin << std::endl;
    }
    
    std::string getTipo() const override { return "Session"; }
    
    int getPeliculaId() const { return pelicula_id; }
    int getSalaId() const { return sala_id; }
    const std::string& getHoraInicio() const { return hora_inicio; }
    const std::string& getHoraFin() const { return hora_fin; }
    
    static std::vector<Session> obtenerTodas();
    static std::vector<Session> obtenerPorPelicula(int movie_id);
};

class Sale : public Entity {
private:
    std::string fecha;
    double precio_total;
    
public:
    Sale(int id = 0, const std::string& fecha = "", double precio = 0.0)
        : Entity(id), fecha(fecha), precio_total(precio) {}
    
    void mostrar() const override {
        std::cout << "Compra " << id << " - " << fecha << " - " << precio_total << " euros" << std::endl;
    }
    
    std::string getTipo() const override { return "Sale"; }
    
    const std::string& getFecha() const { return fecha; }
    double getPrecioTotal() const { return precio_total; }
    
    static std::vector<Sale> obtenerPorUsuario(int user_id);
};

// ============================================================================
// FACTORY PATTERN PARA CREAR USUARIOS
// ============================================================================

class UserFactory {
public:
    static std::unique_ptr<User> crearUsuario(const std::string& tipo, int id, 
                                            const std::string& nombre, 
                                            const std::string& correo,
                                            const std::string& telefono = "",
                                            double saldo = 0.0) {
        if (tipo == "admin" || tipo == "Administrador") {
            return std::make_unique<AdminUser>(id, nombre, correo, telefono);
        } else if (tipo == "cliente" || tipo == "Cliente") {
            return std::make_unique<ClientUser>(id, nombre, correo, telefono, saldo);
        }
        return nullptr;
    }
};

// ============================================================================
// SISTEMA DE AUTENTICACIÓN CON ESTADO PERSISTENTE
// ============================================================================

class AuthenticationService {
private:
    static std::unique_ptr<User> usuario_actual;
    static std::string email_actual;
    static std::string password_actual;
    
public:
    static bool login(const std::string& email, const std::string& password, NetworkClient& cliente) {
        std::string comando = "LOGIN:" + email + ":" + password;
        std::string respuesta = cliente.enviarComando(comando);
        
        std::cout << "Debug - Respuesta del servidor: " << respuesta << std::endl;
        
        if (respuesta.substr(0, 3) == "OK:") {
            // Parsear respuesta: "OK:id:nombre:tipo:correo:telefono:saldo"
            std::vector<std::string> datos = parsearRespuesta(respuesta);
            
            if (datos.size() >= 6) {
                int id = std::stoi(datos[1]);
                std::string nombre = datos[2];
                std::string tipo = datos[3];
                std::string correo = datos[4];
                std::string telefono = datos[5];
                double saldo = (datos.size() > 6) ? std::stod(datos[6]) : 0.0;
                
                // Guardar credenciales para reautenticación automática
                email_actual = email;
                password_actual = password;
                
                // Usar Factory para crear el usuario correcto
                usuario_actual = UserFactory::crearUsuario(tipo, id, nombre, correo, telefono, saldo);
                
                return usuario_actual != nullptr;
            }
        }
        
        return false;
    }
    
    static bool reautenticar(NetworkClient& cliente) {
        if (email_actual.empty()) return false;
        
        std::cout << "Reautenticando..." << std::endl;
        return login(email_actual, password_actual, cliente);
    }
    
    static void logout(NetworkClient& cliente) {
        if (usuario_actual) {
            cliente.enviarComando("LOGOUT");
            usuario_actual.reset();
            email_actual.clear();
            password_actual.clear();
        }
    }
    
    static User* getUsuarioActual() {
        return usuario_actual.get();
    }
    
    static bool isLoggedIn() {
        return usuario_actual != nullptr;
    }
    
    static bool isAdmin() {
        if (!usuario_actual) return false;
        return dynamic_cast<AdminUser*>(usuario_actual.get()) != nullptr;
    }
    
    static bool isClient() {
        if (!usuario_actual) return false;
        return dynamic_cast<ClientUser*>(usuario_actual.get()) != nullptr;
    }
    
private:
    static std::vector<std::string> parsearRespuesta(const std::string& respuesta) {
        std::vector<std::string> resultado;
        std::stringstream ss(respuesta);
        std::string item;
        
        while (std::getline(ss, item, ':')) {
            resultado.push_back(item);
        }
        
        return resultado;
    }
};

// Definición de las variables estáticas
std::unique_ptr<User> AuthenticationService::usuario_actual = nullptr;
std::string AuthenticationService::email_actual = "";
std::string AuthenticationService::password_actual = "";

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

NetworkClient cliente;

// ============================================================================
// IMPLEMENTACIONES DE MÉTODOS ESTÁTICOS CON REAUTENTICACIÓN
// ============================================================================

std::vector<Movie> Movie::obtenerTodas() {
    std::vector<Movie> peliculas;
    
    // Reautenticar antes de cada comando
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string respuesta = cliente.enviarComando("GET_MOVIES");
    std::cout << "Debug GET_MOVIES - Respuesta: " << respuesta << std::endl;
    
    if (respuesta.substr(0, 3) == "OK:") {
        std::string datos = respuesta.substr(3);
        std::stringstream ss(datos);
        std::string pelicula_str;
        
        while (std::getline(ss, pelicula_str, ';')) {
            if (pelicula_str.empty()) continue;
            
            std::stringstream ps(pelicula_str);
            std::string campo;
            std::vector<std::string> campos;
            
            while (std::getline(ps, campo, '|')) {
                campos.push_back(campo);
            }
            
            if (campos.size() >= 4) {
                int id = std::stoi(campos[0]);
                std::string titulo = campos[1];
                int duracion = std::stoi(campos[2]);
                std::string genero = campos[3];
                
                peliculas.emplace_back(id, titulo, duracion, genero);
            }
        }
    }
    
    return peliculas;
}

bool Movie::crear(const std::string& titulo, int duracion, const std::string& genero) {
    // Reautenticar antes de crear
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string comando = "CREATE_MOVIE:" + titulo + ":" + std::to_string(duracion) + ":" + genero;
    std::string respuesta = cliente.enviarComando(comando);
    std::cout << "Debug CREATE_MOVIE - Respuesta: " << respuesta << std::endl;
    
    return respuesta.substr(0, 2) == "OK";
}

std::vector<Room> Room::obtenerTodas() {
    std::vector<Room> salas;
    
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string respuesta = cliente.enviarComando("GET_ROOMS");
    
    if (respuesta.substr(0, 3) == "OK:") {
        std::string datos = respuesta.substr(3);
        std::stringstream ss(datos);
        std::string sala_str;
        
        while (std::getline(ss, sala_str, ';')) {
            if (sala_str.empty()) continue;
            
            std::stringstream ps(sala_str);
            std::string campo;
            std::vector<std::string> campos;
            
            while (std::getline(ps, campo, '|')) {
                campos.push_back(campo);
            }
            
            if (campos.size() >= 3) {
                int id = std::stoi(campos[0]);
                int asientos = std::stoi(campos[1]);
                int libres = std::stoi(campos[2]);
                
                salas.emplace_back(id, asientos, libres);
            }
        }
    }
    
    return salas;
}

bool Room::crear(int num_asientos) {
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string comando = "CREATE_ROOM:" + std::to_string(num_asientos);
    std::string respuesta = cliente.enviarComando(comando);
    return respuesta.substr(0, 2) == "OK";
}

std::vector<Session> Session::obtenerTodas() {
    std::vector<Session> sesiones;
    
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string respuesta = cliente.enviarComando("GET_SESSIONS");
    
    if (respuesta.substr(0, 3) == "OK:") {
        std::string datos = respuesta.substr(3);
        std::stringstream ss(datos);
        std::string sesion_str;
        
        while (std::getline(ss, sesion_str, ';')) {
            if (sesion_str.empty()) continue;
            
            std::stringstream ps(sesion_str);
            std::string campo;
            std::vector<std::string> campos;
            
            while (std::getline(ps, campo, '|')) {
                campos.push_back(campo);
            }
            
            if (campos.size() >= 5) {
                int id = std::stoi(campos[0]);
                int pid = std::stoi(campos[1]);
                int sid = std::stoi(campos[2]);
                std::string inicio = campos[3];
                std::string fin = campos[4];
                
                sesiones.emplace_back(id, pid, sid, inicio, fin);
            }
        }
    }
    
    return sesiones;
}

std::vector<Session> Session::obtenerPorPelicula(int movie_id) {
    std::vector<Session> sesiones;
    
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string comando = "GET_SESSIONS_BY_MOVIE:" + std::to_string(movie_id);
    std::string respuesta = cliente.enviarComando(comando);
    
    if (respuesta.substr(0, 3) == "OK:") {
        std::string datos = respuesta.substr(3);
        std::stringstream ss(datos);
        std::string sesion_str;
        
        while (std::getline(ss, sesion_str, ';')) {
            if (sesion_str.empty()) continue;
            
            std::stringstream ps(sesion_str);
            std::string campo;
            std::vector<std::string> campos;
            
            while (std::getline(ps, campo, '|')) {
                campos.push_back(campo);
            }
            
            if (campos.size() >= 5) {
                int id = std::stoi(campos[0]);
                int pid = std::stoi(campos[1]);
                int sid = std::stoi(campos[2]);
                std::string inicio = campos[3];
                std::string fin = campos[4];
                
                sesiones.emplace_back(id, pid, sid, inicio, fin);
            }
        }
    }
    
    return sesiones;
}

std::vector<Sale> Sale::obtenerPorUsuario(int user_id) {
    std::vector<Sale> ventas;
    
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string comando = "GET_USER_PURCHASES:" + std::to_string(user_id);
    std::string respuesta = cliente.enviarComando(comando);
    
    if (respuesta.substr(0, 3) == "OK:") {
        std::string datos = respuesta.substr(3);
        std::stringstream ss(datos);
        std::string venta_str;
        
        while (std::getline(ss, venta_str, ';')) {
            if (venta_str.empty()) continue;
            
            std::stringstream ps(venta_str);
            std::string campo;
            std::vector<std::string> campos;
            
            while (std::getline(ps, campo, '|')) {
                campos.push_back(campo);
            }
            
            if (campos.size() >= 3) {
                int id = std::stoi(campos[0]);
                std::string fecha = campos[1];
                double precio = std::stod(campos[2]);
                
                ventas.emplace_back(id, fecha, precio);
            }
        }
    }
    
    return ventas;
}

// ============================================================================
// FUNCIONES DE UTILIDAD
// ============================================================================

void limpiarPantalla() {
    system("cls");
}

void pausar() {
    std::cout << "\nPresione Enter para continuar...";
    std::cin.ignore();
    std::cin.get();
}

int leerEntero(const std::string& prompt, int min, int max) {
    int valor;
    while (true) {
        std::cout << prompt << " (" << min << "-" << max << "): ";
        std::cin >> valor;
        
        if (std::cin.fail() || valor < min || valor > max) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Valor invalido. Intente de nuevo." << std::endl;
        } else {
            std::cin.ignore();
            return valor;
        }
    }
}

std::string leerTexto(const std::string& prompt) {
    std::string texto;
    std::cout << prompt << ": ";
    std::getline(std::cin, texto);
    return texto;
}

bool confirmar(const std::string& prompt) {
    std::string respuesta;
    std::cout << prompt << " (s/n): ";
    std::getline(std::cin, respuesta);
    return (respuesta == "s" || respuesta == "S" || respuesta == "si" || respuesta == "SI");
}

void mostrarError(const std::string& mensaje) {
    std::cout << "Error: " << mensaje << std::endl;
}

void mostrarExito(const std::string& mensaje) {
    std::cout << mensaje << std::endl;
}

// ============================================================================
// FUNCIONES AUXILIARES PARA EL SISTEMA DE COMPRAS
// ============================================================================

// Función auxiliar para parsear respuestas del servidor
std::vector<std::string> parsearRespuesta(const std::string& respuesta) {
    std::vector<std::string> resultado;
    std::stringstream ss(respuesta);
    std::string item;
    
    while (std::getline(ss, item, ':')) {
        resultado.push_back(item);
    }
    
    return resultado;
}

// ============================================================================
// FUNCIONES DEL SISTEMA DE COMPRAS
// ============================================================================

// Función para mostrar asientos de una sesión
void mostrarAsientos(int sesion_id) {
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string comando = "GET_SESSION_SEATS:" + std::to_string(sesion_id);
    std::string respuesta = cliente.enviarComando(comando);
    
    if (respuesta.substr(0, 3) != "OK:") {
        std::cout << "Error al obtener informacion de asientos" << std::endl;
        return;
    }
    
    // Parsear respuesta: "OK:sala_id:num_asientos:asientos_ocupados"
    std::vector<std::string> datos = parsearRespuesta(respuesta);
    if (datos.size() < 4) {
        std::cout << "Error en formato de respuesta" << std::endl;
        return;
    }
    
    int sala_id = std::stoi(datos[1]);
    int num_asientos = std::stoi(datos[2]);
    std::string asientos_ocupados_str = datos[3];
    
    // Parsear asientos ocupados (formato: "1,3,5,7")
    std::set<int> asientos_ocupados;
    if (!asientos_ocupados_str.empty() && asientos_ocupados_str != "NONE") {
        std::stringstream ss(asientos_ocupados_str);
        std::string asiento;
        while (std::getline(ss, asiento, ',')) {
            if (!asiento.empty()) {
                asientos_ocupados.insert(std::stoi(asiento));
            }
        }
    }
    
    // Mostrar layout de asientos
    std::cout << "\n\n";
    std::cout << "                  PANTALLA" << std::endl;
    std::cout << "       ";
    for (int j = 0; j < 30; j++) std::cout << "-";
    std::cout << "\n\n" << std::endl;
    
    // Calcular filas y columnas para visualización
    int filas = (int)sqrt(num_asientos) + 1;
    int columnas = (num_asientos / filas) + 1;
    
    for (int fila = 0; fila < filas; fila++) {
        std::cout << std::setw(3) << (fila + 1) << "   ";
        
        for (int col = 0; col < columnas; col++) {
            int asiento_num = fila * columnas + col + 1;
            
            if (asiento_num <= num_asientos) {
                if (asientos_ocupados.find(asiento_num) != asientos_ocupados.end()) {
                    std::cout << " XX  ";  // Ocupado
                } else {
                    std::cout << "[" << std::setw(2) << asiento_num << "] ";  // Libre
                }
            } else {
                std::cout << "     ";  // No hay asiento
            }
        }
        std::cout << std::endl;
    }
    
    std::cout << "\nLeyenda: [##] Asiento libre, XX Asiento ocupado\n" << std::endl;
}

// Función para comprar entradas
void comprarEntradas() {
    limpiarPantalla();
    std::cout << "=== COMPRAR ENTRADAS ===" << std::endl;
    
    // Verificar autenticación
    if (!AuthenticationService::isLoggedIn() || !AuthenticationService::isClient()) {
        std::cout << "Debe estar logueado como cliente para comprar entradas" << std::endl;
        pausar();
        return;
    }
    
    // Obtener películas disponibles
    auto peliculas = Movie::obtenerTodas();
    if (peliculas.empty()) {
        std::cout << "No hay peliculas disponibles" << std::endl;
        pausar();
        return;
    }
    
    std::cout << "\nPELICULAS DISPONIBLES:\n" << std::endl;
    for (size_t i = 0; i < peliculas.size(); i++) {
        std::cout << (i + 1) << ". " << peliculas[i].getTitulo() 
                 << " (" << peliculas[i].getGenero() << ", " 
                 << peliculas[i].getDuracion() << " min)" << std::endl;
    }
    
    int opcion_pelicula = leerEntero("Seleccione una pelicula", 1, (int)peliculas.size());
    int pelicula_id = peliculas[opcion_pelicula - 1].getId();
    
    // Obtener sesiones para la película seleccionada
    auto sesiones = Session::obtenerPorPelicula(pelicula_id);
    if (sesiones.empty()) {
        std::cout << "No hay sesiones disponibles para esta pelicula" << std::endl;
        pausar();
        return;
    }
    
    std::cout << "\nSESIONES DISPONIBLES:\n" << std::endl;
    for (size_t i = 0; i < sesiones.size(); i++) {
        std::string fecha = sesiones[i].getHoraInicio().substr(0, 10);
        std::string hora_inicio = sesiones[i].getHoraInicio().substr(11, 5);
        std::string hora_fin = sesiones[i].getHoraFin().substr(11, 5);
        
        // Obtener información de asientos libres
        if (AuthenticationService::isLoggedIn()) {
            AuthenticationService::reautenticar(cliente);
        }
        
        std::string respuesta = cliente.enviarComando("GET_ROOM_INFO:" + std::to_string(sesiones[i].getSalaId()));
        int asientos_libres = 0;
        if (respuesta.substr(0, 3) == "OK:") {
            std::vector<std::string> datos = parsearRespuesta(respuesta);
            if (datos.size() >= 3) {
                asientos_libres = std::stoi(datos[2]);
            }
        }
        
        std::cout << (i + 1) << ". Fecha: " << fecha 
                 << " | Hora: " << hora_inicio << "-" << hora_fin 
                 << " | Sala: " << sesiones[i].getSalaId() 
                 << " | Asientos libres: " << asientos_libres << std::endl;
    }
    
    int opcion_sesion = leerEntero("Seleccione una sesion", 1, (int)sesiones.size());
    int sesion_id = sesiones[opcion_sesion - 1].getId();
    
    // Mostrar asientos y permitir selección
    limpiarPantalla();
    std::cout << "=== SELECCIONAR ASIENTOS ===" << std::endl;
    
    mostrarAsientos(sesion_id);
    
    // Obtener información de la sala
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string respuesta = cliente.enviarComando("GET_ROOM_INFO:" + std::to_string(sesiones[opcion_sesion - 1].getSalaId()));
    if (respuesta.substr(0, 3) != "OK:") {
        std::cout << "Error al obtener informacion de la sala" << std::endl;
        pausar();
        return;
    }
    
    std::vector<std::string> datos = parsearRespuesta(respuesta);
    int num_asientos = std::stoi(datos[1]);
    
    // Preguntar cuántas entradas
    int num_entradas = leerEntero("Cuantas entradas desea comprar?", 1, 10);
    
    std::vector<int> asientos_seleccionados;
    
    // Seleccionar asientos
    for (int i = 0; i < num_entradas; i++) {
        bool asiento_valido = false;
        
        while (!asiento_valido) {
            std::cout << "\nEntrada #" << (i + 1) << ":" << std::endl;
            int numero_asiento = leerEntero("Ingrese el numero de asiento", 1, num_asientos);
            
            // Verificar si el asiento está disponible
            if (AuthenticationService::isLoggedIn()) {
                AuthenticationService::reautenticar(cliente);
            }
            
            std::string comando = "CHECK_SEAT_AVAILABLE:" + std::to_string(sesion_id) + ":" + std::to_string(numero_asiento);
            std::string respuesta_disponible = cliente.enviarComando(comando);
            
            if (respuesta_disponible == "OK:AVAILABLE") {
                // Verificar que no hayamos seleccionado ya este asiento
                bool ya_seleccionado = false;
                for (int asiento : asientos_seleccionados) {
                    if (asiento == numero_asiento) {
                        ya_seleccionado = true;
                        break;
                    }
                }
                
                if (!ya_seleccionado) {
                    asientos_seleccionados.push_back(numero_asiento);
                    asiento_valido = true;
                    std::cout << "Asiento " << numero_asiento << " seleccionado" << std::endl;
                } else {
                    std::cout << "Ya ha seleccionado ese asiento. Elija otro." << std::endl;
                }
            } else {
                std::cout << "El asiento " << numero_asiento << " no esta disponible. Elija otro." << std::endl;
            }
        }
    }
    
    // Mostrar resumen
    limpiarPantalla();
    std::cout << "=== RESUMEN DE COMPRA ===" << std::endl;
    
    double precio_unitario = 8.50; // Precio base
    double total = precio_unitario * num_entradas;
    
    std::cout << "\nPelicula: " << peliculas[opcion_pelicula - 1].getTitulo() << std::endl;
    std::cout << "Sesion: " << sesiones[opcion_sesion - 1].getHoraInicio().substr(0, 16) << std::endl;
    std::cout << "Sala: " << sesiones[opcion_sesion - 1].getSalaId() << std::endl;
    
    std::cout << "\nAsientos seleccionados:" << std::endl;
    for (size_t i = 0; i < asientos_seleccionados.size(); i++) {
        std::cout << "- Asiento " << asientos_seleccionados[i] << ": " << precio_unitario << " euros" << std::endl;
    }
    
    std::cout << "\nTotal: " << total << " euros" << std::endl;
    
    if (confirmar("Confirmar compra?")) {
        // Procesar la compra
        if (AuthenticationService::isLoggedIn()) {
            AuthenticationService::reautenticar(cliente);
        }
        
        User* usuario = AuthenticationService::getUsuarioActual();
        
        // Crear comando de compra
        std::string comando = "PURCHASE_TICKETS:" + std::to_string(usuario->getId()) + 
                             ":" + std::to_string(sesion_id) + ":" + std::to_string(num_entradas);
        
        // Añadir asientos al comando
        for (int asiento : asientos_seleccionados) {
            comando += ":" + std::to_string(asiento);
        }
        
        std::string respuesta_compra = cliente.enviarComando(comando);
        
        if (respuesta_compra.substr(0, 2) == "OK") {
            mostrarExito("Compra realizada con exito!");
            std::cout << "ID de compra: " << respuesta_compra.substr(3) << std::endl;
        } else {
            mostrarError("Error al procesar la compra: " + respuesta_compra.substr(6));
        }
    } else {
        std::cout << "Compra cancelada" << std::endl;
    }
    
    pausar();
}

// Función para mostrar detalles de una compra
void mostrarDetalleCompra(int venta_id) {
    limpiarPantalla();
    std::cout << "=== DETALLE DE COMPRA ===" << std::endl;
    
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    std::string comando = "GET_PURCHASE_DETAILS:" + std::to_string(venta_id);
    std::string respuesta = cliente.enviarComando(comando);
    
    if (respuesta.substr(0, 3) != "OK:") {
        std::cout << "Error al obtener detalles de la compra" << std::endl;
        pausar();
        return;
    }
    
    // Parsear respuesta con detalles de la compra
    // Formato: "OK:fecha:total:num_billetes:billete1_info;billete2_info;..."
    std::vector<std::string> datos = parsearRespuesta(respuesta);
    if (datos.size() < 4) {
        std::cout << "Error en formato de respuesta" << std::endl;
        pausar();
        return;
    }
    
    std::string fecha = datos[1];
    double total = std::stod(datos[2]);
    int num_billetes = std::stoi(datos[3]);
    
    std::cout << "Fecha: " << fecha << std::endl;
    std::cout << "Total: " << total << " euros\n" << std::endl;
    
    if (num_billetes > 0 && datos.size() > 4) {
        std::cout << "ENTRADAS:\n" << std::endl;
        std::cout << std::left << std::setw(5) << "ID"
                 << std::setw(20) << "Pelicula"
                 << std::setw(15) << "Fecha"
                 << std::setw(10) << "Sala"
                 << std::setw(10) << "Asiento" << std::endl;
        std::cout << "---------------------------------------------------------------------" << std::endl;
        
        // Parsear información de billetes
        std::string billetes_info = datos[4];
        std::stringstream ss(billetes_info);
        std::string billete_str;
        
        while (std::getline(ss, billete_str, ';')) {
            if (billete_str.empty()) continue;
            
            // Formato de cada billete: "id|pelicula|fecha_sesion|sala|asiento"
            std::stringstream bs(billete_str);
            std::string campo;
            std::vector<std::string> campos_billete;
            
            while (std::getline(bs, campo, '|')) {
                campos_billete.push_back(campo);
            }
            
            if (campos_billete.size() >= 5) {
                std::cout << std::left << std::setw(5) << campos_billete[0]
                         << std::setw(20) << campos_billete[1]
                         << std::setw(15) << campos_billete[2]
                         << std::setw(10) << campos_billete[3]
                         << std::setw(10) << campos_billete[4] << std::endl;
            }
        }
    }
    
    pausar();
}

// ============================================================================
// FUNCIONES DE MENÚ
// ============================================================================

void mostrarMenuPrincipal() {
    limpiarPantalla();
    std::cout << "========================================" << std::endl;
    std::cout << "    SISTEMA DE GESTION DE CINE" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "1. Iniciar sesion" << std::endl;
    std::cout << "2. Salir" << std::endl;
    std::cout << "========================================" << std::endl;
}

bool login() {
    limpiarPantalla();
    std::cout << "=== INICIAR SESION ===" << std::endl;
    
    std::string email = leerTexto("Correo electronico");
    std::string password = leerTexto("Contrasena");
    
    if (AuthenticationService::login(email, password, cliente)) {
        std::cout << "\nBienvenido!" << std::endl;
        AuthenticationService::getUsuarioActual()->mostrar();
        pausar();
        return true;
    } else {
        mostrarError("Credenciales incorrectas");
        pausar();
        return false;
    }
}

void logout() {
    AuthenticationService::logout(cliente);
    mostrarExito("Sesion cerrada correctamente");
    pausar();
}

// Función para mostrar la cartelera
void mostrarCartelera() {
    limpiarPantalla();
    std::cout << "=== CARTELERA ===" << std::endl;
    
    // Reautenticar antes de obtener datos
    if (AuthenticationService::isLoggedIn()) {
        AuthenticationService::reautenticar(cliente);
    }
    
    // Obtener películas
    auto peliculas = Movie::obtenerTodas();
    if (peliculas.empty()) {
        std::cout << "No hay peliculas en cartelera actualmente." << std::endl;
        pausar();
        return;
    }
    
    std::cout << "\nPELICULAS EN CARTELERA:\n" << std::endl;
    
    // Mostrar cada película con sus sesiones
    for (const auto& pelicula : peliculas) {
        std::cout << "\n" << pelicula.getId() << ". " << pelicula.getTitulo() << std::endl;
        std::cout << "   Genero: " << pelicula.getGenero() << " | Duracion: " << pelicula.getDuracion() << " minutos" << std::endl;
        
        // Obtener sesiones para esta película
        auto sesiones = Session::obtenerPorPelicula(pelicula.getId());
        if (!sesiones.empty()) {
            std::cout << "   Sesiones disponibles:" << std::endl;
            
            for (const auto& sesion : sesiones) {
                // Obtener información de la sala
                if (AuthenticationService::isLoggedIn()) {
                    AuthenticationService::reautenticar(cliente);
                }
                
                std::string respuesta = cliente.enviarComando("GET_ROOM_INFO:" + std::to_string(sesion.getSalaId()));
                
                int asientos_libres = 0;
                if (respuesta.substr(0, 3) == "OK:") {
                    std::vector<std::string> datos = parsearRespuesta(respuesta);
                    if (datos.size() >= 3) {
                        asientos_libres = std::stoi(datos[2]);
                    }
                }
                
                // Formatear hora (quitar segundos)
                std::string hora_inicio = sesion.getHoraInicio();
                std::string hora_fin = sesion.getHoraFin();
                
                // Extraer solo HH:MM de "YYYY-MM-DD HH:MM:SS"
                if (hora_inicio.length() >= 16) {
                    hora_inicio = hora_inicio.substr(11, 5);
                }
                if (hora_fin.length() >= 16) {
                    hora_fin = hora_fin.substr(11, 5);
                }
                
                // Extraer fecha
                std::string fecha = sesion.getHoraInicio().substr(0, 10);
                
                std::cout << "     - Sesion ID: " << sesion.getId() 
                         << " | Fecha: " << fecha 
                         << " | Hora: " << hora_inicio << "-" << hora_fin 
                         << " | Sala: " << sesion.getSalaId() 
                         << " | Asientos libres: " << asientos_libres << std::endl;
            }
        } else {
            std::cout << "   No hay sesiones disponibles para esta pelicula." << std::endl;
        }
        
        std::cout << "\n-------------------------------------------------" << std::endl;
    }
    
    pausar();
}

// Función para mostrar compras del usuario
void mostrarMisCompras() {
    limpiarPantalla();
    std::cout << "=== MIS COMPRAS ===" << std::endl;
    
    if (!AuthenticationService::isLoggedIn()) {
        std::cout << "Debe estar logueado para ver sus compras" << std::endl;
        pausar();
        return;
    }
    
    User* usuario = AuthenticationService::getUsuarioActual();
    auto compras = Sale::obtenerPorUsuario(usuario->getId());
    
    if (compras.empty()) {
        std::cout << "No ha realizado ninguna compra aun." << std::endl;
        pausar();
        return;
    }
    
    std::cout << "\nHISTORIAL DE COMPRAS:\n" << std::endl;
    std::cout << std::left << std::setw(5) << "ID" 
             << std::setw(20) << "Fecha" 
             << std::setw(10) << "Total" << std::endl;
    std::cout << "---------------------------------------------" << std::endl;
    
    for (const auto& compra : compras) {
        std::string fecha = compra.getFecha().substr(0, 10); // Solo la fecha, sin hora
        
        std::cout << std::left << std::setw(5) << compra.getId()
                 << std::setw(20) << fecha
                 << std::setw(10) << compra.getPrecioTotal() << " euros" << std::endl;
    }
    
    std::cout << std::endl;
    int opcion = leerEntero("Seleccione una compra para ver detalles (0 para volver)", 0, (int)compras.size());
    
    if (opcion > 0) {
        mostrarDetalleCompra(compras[opcion - 1].getId());
    }
}

void menuAdmin() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== MENU ADMINISTRADOR ===" << std::endl;
        AuthenticationService::getUsuarioActual()->mostrar();
        std::cout << "\n1. Gestionar Peliculas" << std::endl;
        std::cout << "2. Gestionar Salas" << std::endl;
        std::cout << "3. Gestionar Sesiones" << std::endl;
        std::cout << "4. Gestion de Usuarios" << std::endl;
        std::cout << "5. Cerrar sesion" << std::endl;
        
        int opcion = leerEntero("Seleccione una opcion", 1, 4);
        
        switch (opcion) {
            case 1:
                menuGestionPeliculas();
                break;
            case 2:
                menuGestionSalas();
                break;
            case 3:
                menuGestionSesiones();
                break;
            case 4:
                menuGestionUsuarios();
                break;
            case 5:
                logout();
                return;
        }
    }
}

void menuCliente() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== MENU CLIENTE ===" << std::endl;
        AuthenticationService::getUsuarioActual()->mostrar();
        std::cout << "\n1. Ver Cartelera" << std::endl;
        std::cout << "2. Comprar Entradas" << std::endl;
        std::cout << "3. Mis Compras" << std::endl;
        std::cout << "4. Cerrar sesion" << std::endl;
        
        int opcion = leerEntero("Seleccione una opcion", 1, 4);
        
        switch (opcion) {
            case 1:
                mostrarCartelera();
                break;
            case 2:
                comprarEntradas();
                break;
            case 3:
                mostrarMisCompras();
                break;
            case 4:
                logout();
                return;
        }
    }
}

// Implementaciones básicas para los menús de administrador
void menuGestionPeliculas() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== GESTION DE PELICULAS ===" << std::endl;
        std::cout << "1. Listar peliculas" << std::endl;
        std::cout << "2. Anadir pelicula" << std::endl;
        std::cout << "3. Eliminar pelicula" << std::endl;
        std::cout << "4. Volver" << std::endl;

        int opcion = leerEntero("Seleccione una opcion", 1, 4);

        switch (opcion) {
            case 1: {
                auto peliculas = Movie::obtenerTodas();
                limpiarPantalla();
                std::cout << "=== LISTA DE PELICULAS ===" << std::endl;
                for (const auto& p : peliculas) {
                    p.mostrar();
                }
                pausar();
                break;
            }
            case 2: {
                std::string titulo = leerTexto("Titulo");
                int duracion = leerEntero("Duracion en minutos", 1, 500);
                std::string genero = leerTexto("Genero");

                if (Movie::crear(titulo, duracion, genero)) {
                    mostrarExito("Pelicula anadida correctamente");
                } else {
                    mostrarError("Error al anadir pelicula");
                }
                pausar();
                break;
            }
            case 3: {
                auto peliculas = Movie::obtenerTodas();
                if (peliculas.empty()) {
                    mostrarError("No hay peliculas para eliminar");
                    pausar();
                    break;
                }

                for (size_t i = 0; i < peliculas.size(); ++i) {
                    std::cout << i + 1 << ". ";
                    peliculas[i].mostrar();
                }

                int idx = leerEntero("Seleccione pelicula a eliminar", 1, (int)peliculas.size());
                int id = peliculas[idx - 1].getId();

                std::string respuesta = cliente.enviarComando("DELETE_MOVIE:" + std::to_string(id));
                if (respuesta.substr(0, 2) == "OK") {
                    mostrarExito("Pelicula eliminada correctamente");
                } else {
                    mostrarError("Error al eliminar pelicula");
                }
                pausar();
                break;
            }
            case 4:
                return;
        }
    }
}


void menuGestionSalas() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== GESTION DE SALAS ===" << std::endl;
        std::cout << "1. Listar salas" << std::endl;
        std::cout << "2. Anadir sala" << std::endl;
        std::cout << "3. Eliminar sala" << std::endl;
        std::cout << "4. Volver" << std::endl;

        int opcion = leerEntero("Seleccione una opcion", 1, 4);

        switch (opcion) {
            case 1: {
                auto salas = Room::obtenerTodas();
                limpiarPantalla();
                std::cout << "=== LISTA DE SALAS ===" << std::endl;
                for (const auto& sala : salas) {
                    sala.mostrar();
                }
                pausar();
                break;
            }
            case 2: {
                int asientos = leerEntero("Numero total de asientos", 1, 300);
                if (Room::crear(asientos)) {
                    mostrarExito("Sala anadida correctamente");
                } else {
                    mostrarError("Error al añadir sala");
                }
                pausar();
                break;
            }
            case 3: {
                auto salas = Room::obtenerTodas();
                if (salas.empty()) {
                    mostrarError("No hay salas para eliminar");
                    pausar();
                    break;
                }

                for (size_t i = 0; i < salas.size(); ++i) {
                    std::cout << i + 1 << ". ";
                    salas[i].mostrar();
                }

                int idx = leerEntero("Seleccione sala a eliminar", 1, (int)salas.size());
                int id = salas[idx - 1].getId();

                std::string respuesta = cliente.enviarComando("DELETE_ROOM:" + std::to_string(id));
                if (respuesta.substr(0, 2) == "OK") {
                    mostrarExito("Sala eliminada correctamente");
                } else {
                    mostrarError("Error al eliminar sala");
                }
                pausar();
                break;
            }
            case 4:
                return;
        }
    }
}


void menuGestionSesiones() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== GESTION DE SESIONES ===" << std::endl;
        std::cout << "1. Listar sesiones" << std::endl;
        std::cout << "2. Anadir sesion" << std::endl;
        std::cout << "3. Eliminar sesion" << std::endl;
        std::cout << "4. Volver" << std::endl;

        int opcion = leerEntero("Seleccione una opcion", 1, 4);

        switch (opcion) {
            case 1: {
                auto sesiones = Session::obtenerTodas();
                limpiarPantalla();
                std::cout << "=== LISTA DE SESIONES ===" << std::endl;
                for (const auto& s : sesiones) {
                    s.mostrar();
                }
                pausar();
                break;
            }
            case 2: {
                auto peliculas = Movie::obtenerTodas();
                auto salas = Room::obtenerTodas();

                if (peliculas.empty() || salas.empty()) {
                    mostrarError("Debe haber al menos una pelicula y una sala");
                    pausar();
                    break;
                }

                std::cout << "\nPeliculas disponibles:" << std::endl;
                for (size_t i = 0; i < peliculas.size(); ++i) {
                    std::cout << i + 1 << ". ";
                    peliculas[i].mostrar();
                }

                int p_idx = leerEntero("Seleccione pelicula", 1, (int)peliculas.size());
                int pelicula_id = peliculas[p_idx - 1].getId();

                std::cout << "\nSalas disponibles:" << std::endl;
                for (size_t i = 0; i < salas.size(); ++i) {
                    std::cout << i + 1 << ". ";
                    salas[i].mostrar();
                }

                int s_idx = leerEntero("Seleccione sala", 1, (int)salas.size());
                int sala_id = salas[s_idx - 1].getId();

                std::string inicio = leerTexto("Hora de inicio (YYYY-MM-DD HH:MM:SS)");
                std::string fin = leerTexto("Hora de fin (YYYY-MM-DD HH:MM:SS)");

                std::string comando = "CREATE_SESSION:" + std::to_string(pelicula_id) + ":" +
                                      std::to_string(sala_id) + ":" + inicio + ":" + fin;

                std::string respuesta = cliente.enviarComando(comando);
                if (respuesta.substr(0, 2) == "OK") {
                    mostrarExito("Sesion creada correctamente");
                } else {
                    mostrarError("Error al crear sesion");
                }
                pausar();
                break;
            }
            case 3: {
                auto sesiones = Session::obtenerTodas();
                if (sesiones.empty()) {
                    mostrarError("No hay sesiones para eliminar");
                    pausar();
                    break;
                }

                for (size_t i = 0; i < sesiones.size(); ++i) {
                    std::cout << i + 1 << ". ";
                    sesiones[i].mostrar();
                }

                int idx = leerEntero("Seleccione sesion a eliminar", 1, (int)sesiones.size());
                int id = sesiones[idx - 1].getId();

                std::string respuesta = cliente.enviarComando("DELETE_SESSION:" + std::to_string(id));
                if (respuesta.substr(0, 2) == "OK") {
                    mostrarExito("Sesion eliminada correctamente");
                } else {
                    mostrarError("Error al eliminar sesion");
                }
                pausar();
                break;
            }
            case 4:
                return;
        }
    }
}

void menuGestionUsuarios() {
    while (true) {
        limpiarPantalla();
        std::cout << "=== GESTION DE USUARIOS ===\n";
        std::cout << "1. Ver usuarios\n";
        std::cout << "2. Anadir usuario\n";
        std::cout << "3. Eliminar usuario\n";
        std::cout << "4. Volver al menu anterior\n";

        int opcion = leerEntero("Seleccione una opcion", 1, 4);
        if (opcion == 1) {
            std::string respuesta = cliente.enviarComando("GET_USERS");
            if (respuesta.substr(0, 3) == "OK:") {
                std::cout << "\nLISTA DE USUARIOS:\n";
                std::stringstream ss(respuesta.substr(3));
                std::string linea;
                while (std::getline(ss, linea, ';')) {
                    std::stringstream ls(linea);
                    std::string campo;
                    std::vector<std::string> campos;
                    while (std::getline(ls, campo, '|')) {
                        campos.push_back(campo);
                    }
                    if (campos.size() >= 4) {
                        std::cout << "ID: " << campos[0]
                                  << ", Nombre: " << campos[1]
                                  << ", Tipo: " << campos[2]
                                  << ", Correo: " << campos[3] << "\n";
                    }
                }
            } else {
                mostrarError("Error al obtener usuarios: " + respuesta);
            }
            pausar();
        } else if (opcion == 2) {
            std::string nombre = leerTexto("Nombre");
            std::string correo = leerTexto("Correo");
            std::string password = leerTexto("Contrasena");
            std::string tipo = leerTexto("Tipo (admin/cliente)");

            std::string comando = "CREATE_USER:" + nombre + ":" + correo + ":" + password + ":" + tipo;
            std::string respuesta = cliente.enviarComando(comando);
            if (respuesta.substr(0, 2) == "OK") {
                mostrarExito("Usuario creado con exito");
            } else {
                mostrarError("Error al crear usuario: " + respuesta);
            }
            pausar();
        } else if (opcion == 3) {
            int user_id = leerEntero("ID del usuario a eliminar", 1, 100000);
            std::string comando = "DELETE_USER:" + std::to_string(user_id);
            std::string respuesta = cliente.enviarComando(comando);
            if (respuesta.substr(0, 2) == "OK") {
                mostrarExito("Usuario eliminado con exito");
            } else {
                mostrarError("Error al eliminar usuario: " + respuesta);
            }
            pausar();
        } else {
            break;
        }
    }
}


// ============================================================================
// FUNCIÓN PRINCIPAL
// ============================================================================

int main() {
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cout << "Error inicializando Winsock" << std::endl;
        return -1;
    }
    #endif
    
    std::cout << "=== CLIENTE SISTEMA DE CINE ===" << std::endl;
    std::cout << "Preparando conexion al servidor..." << std::endl;
    
    std::cout << "Sistema listo para conectar!" << std::endl;
    pausar();
    
    while (true) {
        mostrarMenuPrincipal();
        
        int opcion = leerEntero("Seleccione una opcion", 1, 2);
        
        switch (opcion) {
            case 1:
                if (login()) {
                    User* usuario = AuthenticationService::getUsuarioActual();
                    usuario->mostrarMenu();
                }
                break;
                
            case 2:
                cliente.enviarComando("QUIT");
                cliente.desconectar();
                std::cout << "\nHasta luego!" << std::endl;
                #ifdef _WIN32
                WSACleanup();
                #endif
                return 0;
        }
    }
    
    #ifdef _WIN32
    WSACleanup();
    #endif
    return 0;
}