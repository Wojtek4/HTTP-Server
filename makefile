BUILD := RELEASE

BUILD_DIR.DEBUG := debug
BUILD_DIR.RELEASE := release
BUILD_DIR := $(BUILD_DIR.$(BUILD))

TARGET := server
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src%,$(BUILD_DIR)/build_files%, $(patsubst %.cpp,%.o,$(SOURCES)))
DEPS := $(patsubst %.o,%.d,$(OBJECTS))

CXXFLAGS.DEBUG := -Wall -Wextra -g -std=c++2a -DDEBUG
CXXFLAGS.RELEASE := -O3 -std=c++2a
CXXFLAGS := $(CXXFLAGS.$(BUILD))

CXX := g++
INCLUDE := -I.
LIBPATH :=
LIBS :=

all: $(BUILD_DIR)/$(TARGET)

-include $(DEPS)

$(BUILD_DIR)/$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(OBJECTS) -lpthread -o $@ $(LIBPATH) $(LIBS)

$(BUILD_DIR)/build_files/%.o: src/%.cpp | build_files_mkdir
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MMD -c $< -o $@

$(OBJECTS):

.PHONY: clean build_files_mkdir

build_files_mkdir:
	mkdir -p $(BUILD_DIR)/build_files

clean:
	rm -rf $(BUILD_DIR)
