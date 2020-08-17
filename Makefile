CXX      := -g++
CXXFLAGS := -pedantic-errors -Wall -Wextra -g -std=c++11 -fpermissive
LDFLAGS  := -L/usr/lib -lstdc++ -lm `sdl2-config --libs`
BUILD    := ./build
OBJ_DIR  := $(BUILD)/objects
APP_DIR  := $(BUILD)/
TARGET   := SGBE
INCLUDE  := -I /usr/include/SDL2
SRC      :=                      \
   $(wildcard src/common/*.cpp) \
   $(wildcard src/gb/*.cpp) \
   $(wildcard src/sgbe_app/*.cpp) \

OBJECTS  := $(SRC:%.cpp=$(OBJ_DIR)/%.o)

all: build $(APP_DIR)/$(TARGET)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@ $(LDFLAGS)

$(APP_DIR)/$(TARGET): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(APP_DIR)/$(TARGET) $^ $(LDFLAGS)

build:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/$(TARGET)