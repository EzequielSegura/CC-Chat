#include <iostream>
#include <vector>
#include <cstdlib>
#include <asio.hpp>
#include <thread>
#include <memory>

using asio::ip::tcp;
using namespace std;

void gotoxy(int x, int y) {
    cout << "\x1b[" << y << ";" << x << "H";
}

void handleConnection(shared_ptr<tcp::socket> socket, vector<shared_ptr<tcp::socket>>& activeConnections, vector<string>& bulletin, int& conexiones) {
    try {
        conexiones++;

        gotoxy(0, 2);
        printf("Numero de conexiones: %d", conexiones);

        // Agregar la conexión a la lista de conexiones activas
        activeConnections.push_back(socket);

        while (true) {
            asio::streambuf buffer;
            asio::error_code error;

            // Intenta leer desde el socket
            size_t bytes_transferred = asio::read_until(*socket, buffer, "\n", error);

            if (error == asio::error::eof) {
                // El cliente ha cerrado la conexión de manera ordenada
                conexiones--;
                gotoxy(0, 2);
                printf("Numero de conexiones: %d", conexiones);
                break;
            } else if (error) {
                // Error al leer desde el socket
                throw asio::system_error(error);
            }

            istream input(&buffer);
            string message;
            getline(input, message);

            gotoxy(0, 3);
            cout << "                                                                                        ";
            gotoxy(0, 3);
            cout << "Mensaje mas reciente: " << message << endl;

            // Agregar el mensaje al tablón
            bulletin.push_back(message);

            // Mostrar el tablón
            gotoxy(0, 4);
            cout << "Tablón:" << endl;
            for (const string& msg : bulletin) {
                cout << "   -" << msg << endl;
            }

            // Enviar el mensaje a todos los clientes activos
            for (auto& clientSocket : activeConnections) {
                asio::write(*clientSocket, asio::buffer(message + "\n"));
            }
        }
    } catch (exception& e) {
        conexiones--;
        gotoxy(0, 2);
        printf("Numero de conexiones: %d", conexiones);
    }
}

int main() {
    system("cls");
    int conexiones = 0;
    vector<shared_ptr<tcp::socket>> activeConnections; // Lista de conexiones activas

    try {
        asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));

        vector<string> bulletin; 
        cout << "Servidor funcionando..." << endl;

        while (true) {
            shared_ptr<tcp::socket> socket = make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);

            // Crear un hilo para manejar la conexión y pasar socket como shared_ptr
            thread(handleConnection, socket, ref(activeConnections), ref(bulletin), ref(conexiones)).detach();
        }
        io_context.run(); 

    } catch (exception& e) {
        cerr << "Error en el servidor: " << e.what() << endl;
    }

    return 0;
}
