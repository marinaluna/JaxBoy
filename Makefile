BINARY := build/jaxboy
SRCS := $(shell find . -iname "*.cpp")
OBJS := $(addprefix build/,$(SRCS:.cpp=.o))

NEEDED_LIBS := SDL2 SDLmain pthread

CXX := g++ -flto
LD := $(CXX) $(addprefix -l,$(NEEDED_LIBS))
override CXXFLAGS += -std=c++11
override LDFLAGS += $(CXXFLAGS) -lSDL2 -lSDLmain

all:$(BINARY)

$(BINARY):$(OBJS) $(addprefix -l,$(NEEDED_LIBS))
	$(LD) $(LDFLAGS) -o $@ $^

build/%.o:%.cpp
	$(CXX) -O2 -c $(CXXFLAGS) $< -o $@

run:$(BINARY)
	@$(BINARY) pokeblue.gb bootrom.bin --scale=2

clean:
	rm -rf build/*
	mkdir -p build/src/core
	mkdir -p build/src/core/processor
	mkdir -p build/src/core/memory
	mkdir -p build/src/core/memory/mbc
	mkdir -p build/src/debug
