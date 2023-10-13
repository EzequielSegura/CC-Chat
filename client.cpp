#include <iostream>
#include <cstdlib> 
#include <asio.hpp>

using asio::ip::tcp;
using namespace std;

// Funcion gotoxy para posicionar el puntero en la consola
void gotoxy(int x, int y) {
    cout << "\x1b[" << y << ";" << x << "H";
}

void receiveMessages(tcp::socket& socket, vector<string>& bulletin) {
    try {
        while (true) {
            asio::streambuf buffer;
            asio::error_code error;

            // Intenta leer desde el socket
            size_t bytes_transferred = asio::read_until(socket, buffer, "\n", error);

            if (error == asio::error::eof) {
                // El servidor ha cerrado la conexión
                break;
            } else if (error) {
                // Error al leer desde el socket
                throw asio::system_error(error);
            }

            // Recibe el mensaje del servidor y lo muestra por consola
            istream input(&buffer);
            string message;
            getline(input, message);

            // Almacenar mensajes segun se reciben
            bulletin.push_back(message);

            gotoxy(0, 4);
            cout << "Mensajes:" << endl;

            int startIdx = 0; // Índice inicial para imprimir los mensajes

            if (bulletin.size() > 40) {
                startIdx = bulletin.size() - 40; // Si hay más de 40 mensajes, empezamos desde los últimos 20
            }

            for (int i = startIdx; i < bulletin.size(); i++) {
                cout << "   " << bulletin[i] << endl;
            }
            gotoxy(1, 1);
        }
    } catch (exception& e) {
        // No se puede recibir mensajes
        cerr << "Error al recibir mensajes: " << e.what() << endl;
    }
}

int main(int argc, char* argv[]) {
    // Para poder iniciar la aplicacion necesita recibir un nombre de usuario y una ip
    if (argc != 5 || string(argv[1]) != "-u" || string(argv[3]) != "-i") {
        cerr << "Uso: servidor.exe -u <nombre> -i <ip del servidor>" << endl;
        return 1;
    }

    // Recibe el nombre de usuario
    string username = argv[2];
    // Recibre la ip del servidor
    string ip = argv[4];

    system("cls");

    try {
        asio::io_context io_context;
        tcp::socket socket(io_context);
        
        // Lista de mensajes
        vector<string> bulletin; // Lista dee mensajes

        // Conectar con el servidor usando la ip que se ingreso
        socket.connect(tcp::endpoint(asio::ip::address::from_string(ip), 8080));

        // Mensaje de que pudo conectar
        cout << "Conectado" << endl;

        // Envia mensaje inicial al entrar al chat
        asio::async_write(socket, asio::buffer("(" + username + ") " + "Entró al chat" + "\n"), [](const asio::error_code& error, size_t bytes_transferred) {});

        // Inicia un hilo para recibir y mostrar mensajes
        thread(receiveMessages, ref(socket), ref(bulletin)).detach();

        while (true) {
            // Recibir el mensaje para enviar
            gotoxy(1,1);
            string message;
            cout << ">";
            getline(cin, message);
            
            // Desconectar de forma voluntaria y enviar un mensaje de despedida
            if (message == "exit") {
                asio::write(socket, asio::buffer("(" + username + ") " + "Desconexión voluntaria\n"));
                break; 
            }
            
            // Si los mensajes no son solo espacio vacio se envia
            if (message != "") {
                asio::async_write(socket, asio::buffer("(" + username + ") " + message + "\n"), [](const asio::error_code& error, size_t bytes_transferred) {});
            }
        }
        
        socket.close();
    } catch (exception& e) {
        // No se puede conectar con el servidor
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}
