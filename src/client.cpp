#include <iostream>
#include <cstdlib> 
#include <asio.hpp>
#include <thread>
#include <vector>
#include <string>
#include <mutex>

using asio::ip::tcp;
using namespace std;

// Variables globales para el manejo de la interfaz
mutex console_mutex;
string current_input = "";
int cursor_pos = 0;

// Función gotoxy para posicionar el puntero en la consola
void gotoxy(int x, int y) {
    cout << "\x1b[" << y << ";" << x << "H";
}

// Función para limpiar una línea específica
void clearLine(int y) {
    gotoxy(1, y);
    cout << "\x1b[K"; // Limpiar desde el cursor hasta el final de la línea
}

// Función para obtener el tamaño de la consola (aproximado)
int getConsoleHeight() {
    // En Windows, típicamente son 25 líneas por defecto
    // Puedes ajustar esto según tu configuración
    return 25;
}

void displayMessages(const vector<string>& bulletin, const string& username) {
    lock_guard<mutex> lock(console_mutex);
    
    int console_height = getConsoleHeight();
    int message_area_height = console_height - 4; // Reservar 4 líneas para la interfaz
    int input_line = console_height - 1;
    int separator_line = console_height - 3;
    
    // Limpiar solo el área de mensajes (no toda la pantalla)
    for (int i = 4; i <= separator_line - 1; i++) {
        clearLine(i);
    }
    
    // Mostrar mensajes
    gotoxy(1, 4);
    cout << "=== MENSAJES ===" << endl;
    
    int start_idx = 0;
    int max_messages = message_area_height - 2; // -2 por el header y separador
    
    if (bulletin.size() > max_messages) {
        start_idx = bulletin.size() - max_messages;
    }
    
    for (int i = start_idx; i < bulletin.size(); i++) {
        cout << "   " << bulletin[i] << endl;
    }
    
    // Dibujar separador
    gotoxy(1, separator_line);
    cout << string(50, '=') << endl;
    
    // Mostrar área de escritura
    gotoxy(1, separator_line + 1);
    cout << "Escribir mensaje (exit para salir):" << endl;
    
    // Restaurar el input actual
    gotoxy(1, input_line);
    cout << "(" << username << ") " << current_input;
    
    // Posicionar el cursor correctamente
    gotoxy(username.length() + 4 + cursor_pos, input_line);
}

void receiveMessages(tcp::socket& socket, vector<string>& bulletin, const string& username) {
    try {
        while (true) {
            asio::streambuf buffer;
            asio::error_code error;

            size_t bytes_transferred = asio::read_until(socket, buffer, "\n", error);

            if (error == asio::error::eof) {
                break;
            } else if (error) {
                throw asio::system_error(error);
            }

            istream input(&buffer);
            string message;
            getline(input, message);

            // Almacenar el mensaje
            bulletin.push_back(message);

            // Actualizar la pantalla manteniendo el input actual
            displayMessages(bulletin, username);
        }
    } catch (exception& e) {
        lock_guard<mutex> lock(console_mutex);
        cerr << "Error al recibir mensajes: " << e.what() << endl;
    }
}

// Función para manejar el input de manera más sofisticada
string getInputLine(const string& username, vector<string>& bulletin) {
    current_input = "";
    cursor_pos = 0;
    
    while (true) {
        // Mostrar el estado actual
        {
            lock_guard<mutex> lock(console_mutex);
            int input_line = getConsoleHeight() - 1;
            gotoxy(1, input_line);
            cout << "\x1b[K"; // Limpiar la línea
            cout << "(" << username << ") " << current_input;
            gotoxy(username.length() + 4 + cursor_pos, input_line);
        }
        
        char ch = cin.get();
        
        if (ch == '\n') {
            string result = current_input;
            current_input = "";
            cursor_pos = 0;
            return result;
        } else if (ch == '\b' || ch == 127) { // Backspace
            if (cursor_pos > 0 && !current_input.empty()) {
                current_input.erase(cursor_pos - 1, 1);
                cursor_pos--;
            }
        } else if (ch >= 32 && ch <= 126) { // Caracteres imprimibles
            current_input.insert(cursor_pos, 1, ch);
            cursor_pos++;
        }
        // Ignorar otros caracteres de control
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5 || string(argv[1]) != "-u" || string(argv[3]) != "-i") {
        cerr << "Uso: client.exe -u <nombre> -i <ip del servidor>" << endl;
        return 1;
    }

    string username = argv[2];
    string ip = argv[4];
    
    // Configurar la consola
    system("cls");
    string comando = "title Chat - " + username;
    system(comando.c_str());
    
    // Mostrar información inicial
    gotoxy(1, 1);
    cout << "=== CLIENTE DE CHAT ===" << endl;
    cout << "Usuario: " << username << endl;
    cout << "Servidor: " << ip << ":8080" << endl;

    try {
        asio::io_context io_context;
        tcp::socket socket(io_context);
        vector<string> bulletin;

        // Conectar al servidor
        socket.connect(tcp::endpoint(asio::ip::address::from_string(ip), 8080));
        
        gotoxy(1, 3);
        cout << "Estado: CONECTADO" << endl;

        // Enviar mensaje de entrada
        asio::async_write(socket, asio::buffer("(" + username + ") " + "Entró al chat" + "\n"), 
                         [](const asio::error_code& error, size_t bytes_transferred) {});

        // Iniciar hilo para recibir mensajes
        thread receive_thread(receiveMessages, ref(socket), ref(bulletin), ref(username));
        receive_thread.detach();

        // Dar tiempo para que se establezca la conexión y se muestre la interfaz inicial
        this_thread::sleep_for(chrono::milliseconds(500));
        displayMessages(bulletin, username);

        // Loop principal para enviar mensajes
        while (true) {
            string message = getInputLine(username, bulletin);
            
            if (message == "exit") {
                asio::write(socket, asio::buffer("(" + username + ") " + "Desconexión voluntaria\n"));
                break;
            }
            
            if (!message.empty()) {
                asio::async_write(socket, asio::buffer("(" + username + ") " + message + "\n"), 
                                 [](const asio::error_code& error, size_t bytes_transferred) {});
            }
        }
        
        socket.close();
        
        // Mensaje de despedida
        {
            lock_guard<mutex> lock(console_mutex);
            gotoxy(1, getConsoleHeight());
            cout << "Desconectado del chat. Presiona Enter para salir...";
        }
        cin.get();
        
    } catch (exception& e) {
        lock_guard<mutex> lock(console_mutex);
        cerr << "Error: " << e.what() << endl;
        cin.get();
    }

    return 0;
}