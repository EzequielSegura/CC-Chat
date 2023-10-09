#include <iostream>
#include <cstdlib> 
#include <asio.hpp>

using asio::ip::tcp;
using namespace std;

void gotoxy(int x, int y) {
    cout << "\x1b[" << y << ";" << x << "H";
}

void receiveMessages(tcp::socket& socket) {
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

            istream input(&buffer);
            string message;
            getline(input, message);

            // Muestra el mensaje recibido en la pantalla
            cout << message << endl;
        }
    } catch (exception& e) {
        cerr << "Error al recibir mensajes: " << e.what() << endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3 || string(argv[1]) != "-u") {
        cerr << "Uso: servidor.exe -u <nombre>" << endl;
        return 1;
    }

    string username = argv[2];

    system("cls");

    try {
        asio::io_context io_context;
        tcp::socket socket(io_context);
        
        socket.connect(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 8080));

        cout << "Conectado" << endl;

        asio::async_write(socket, asio::buffer("(" + username + ") " + "Entró al chat" + "\n"), [](const asio::error_code& error, size_t bytes_transferred) {});

        // Inicia un hilo para recibir y mostrar mensajes
        thread(receiveMessages, ref(socket)).detach();
        while (true) {
            string message;
            getline(cin, message);
            
            if (message == "exit") {
                asio::write(socket, asio::buffer("(" + username + ") " + "Desconexión voluntaria\n"));
                break; 
            }
            
            if (message != "") {
                asio::async_write(socket, asio::buffer("(" + username + ") " + message + "\n"), [](const asio::error_code& error, size_t bytes_transferred) {});
            }
        }
        
        socket.close();
    } catch (exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}
