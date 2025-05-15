# Makefile para cliente

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
LDFLAGS = -lws2_32

# Archivos fuente
SRC = src/main.cpp src/client.cpp src/menu.cpp ../common/protocol.cpp
OBJ = $(SRC:.cpp=.o)
BIN = cinegestion_client.exe

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	del /Q $(subst /,\,$(OBJ)) $(BIN)

.PHONY: all clean