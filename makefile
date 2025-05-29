# Compilador y flags
CXX = g++
CXXFLAGS = -Iinclude
LDFLAGS = -lmingw32 -lws2_32 -static-libgcc -static-libstdc++ -lwinpthread -static -pthread

# Directorios
BUILD_DIR = build
SRC_DIR = src

# Archivos fuente
SERVER_SRC = $(SRC_DIR)/server.cpp
CLIENT_SRC = $(SRC_DIR)/client.cpp

# Ejecutables
SERVER_TARGET = $(BUILD_DIR)/server
CLIENT_TARGET = $(BUILD_DIR)/client

# Regla principal
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# Crear directorio build si no existe
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilar servidor
$(SERVER_TARGET): $(SERVER_SRC) | $(BUILD_DIR)
	$(CXX) $(SERVER_SRC) $(CXXFLAGS) -o $(SERVER_TARGET) $(LDFLAGS)

# Compilar cliente
$(CLIENT_TARGET): $(CLIENT_SRC) | $(BUILD_DIR)
	$(CXX) $(CLIENT_SRC) $(CXXFLAGS) -o $(CLIENT_TARGET) $(LDFLAGS)

# Limpiar archivos compilados
clean:
	rm -rf $(BUILD_DIR)

# Ejecutar servidor
run-server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

# Ejecutar cliente
run-client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET)

# Reglas que no son archivos
.PHONY: all clean run-server run-client