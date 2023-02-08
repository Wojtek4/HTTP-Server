TARGET := server
SOURCES := $(wildcard src/*.cpp)
HEADERS := $(wildcard src/*.hpp)
OBJECTS := $(patsubst src%,obj%, $(patsubst %.cpp,%.o,$(SOURCES)))

INCLUDE := -I.
LIBPATH :=
LIBS :=

FLAGS := -Wall -Wextra -g -std=c++2a
CXXFLAGS := $(FLAGS)

CC := gcc
CXX := g++

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(OBJECTS) -lpthread -o $(TARGET) $(LIBPATH) $(LIBS)

obj/%.o: src/%.cpp $(HEADERS) | obj_mkdir
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(OBJECTS): | obj_mkdir

obj_mkdir:
	mkdir -p obj

.PHONY: clean

clean:
	rm -rf obj
	rm -f $(TARGET)
