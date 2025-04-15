EXE = mvmdbg
IMGUI_DIR = imgui
INCLUDE_DIRS = -I$(IMGUI_DIR) -I../src
CXXFLAGS = -std=c++11 -pedantic -Wall -MMD -MP $(INCLUDE_DIRS) `sdl2-config --cflags` -g
LDFLAGS = -ldl `sdl2-config --libs` -lGL
SRCS = $(shell find src -name *.cpp) $(shell find imgui -name *.cpp)
OBJS = $(SRCS:%=build/%.o)
DEPS = $(OBJS:.o=.d)

all: bin/$(EXE)

bin/$(EXE): $(OBJS)
	mkdir -p bin
	$(CXX) $^ -o $@ $(LDFLAGS)

build/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: run clean

run: bin/$(EXE)
	./bin/$(EXE)

clean:
	rm -rf bin build

-include $(DEPS)
