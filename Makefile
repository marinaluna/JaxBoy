BINARY := build/jaxboy
SRCS := $(shell find . -iname "*.cpp")
OBJS := $(addprefix build/,$(SRCS:.cpp=.o))

NEEDED_LIBS := libminifb.a
NEEDED_FRAMEWORKS := CoreGraphics AppKit

CXX := clang++
LD := $(CXX) $(addprefix libs/,$(NEEDED_LIBS)) $(addprefix -framework ,$(NEEDED_FRAMEWORKS))
override CXXFLAGS += -std=c++11
override LDFLAGS += $(CXXFLAGS)

all:$(BINARY)

$(BINARY):$(OBJS) $(addprefix libs/,$(NEEDED_LIBS))
	$(LD) $(LDFLAGS) -o $@ $^

build/%.o:%.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

run:$(BINARY)
	@$(BINARY) roms/tetris.gb roms/bootrom.bin -debug

clean:
	rm -rf build/*
	mkdir -p build/src/core
	mkdir -p build/src/core/memory
	mkdir -p build/src/core/processor
	mkdir -p build/src/common
	mkdir -p build/src/debugger
