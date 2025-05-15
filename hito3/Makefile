# Makefile para servidor

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
LDFLAGS = -lws2_32 -lsqlite3

# Archivos fuente
SRC = src/main.cpp src/server.cpp ../common/protocol.cpp
OBJ = $(SRC:.cpp=.o)
BIN = cinegestion_server.exe

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q $(subst /,\,$(OBJ)) $(BIN)

.PHONY: all clean