# Makefile para la biblioteca común

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic
INCLUDES = -I.

SRC = protocol.cpp models/pelicula.cpp models/sesion.cpp models/asiento.cpp models/billete.cpp models/venta.cpp
OBJ = $(SRC:.cpp=.o)
LIB = libcommon.a

all: $(LIB)

$(LIB): $(OBJ)
	ar rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) $(LIB)

.PHONY: all clean