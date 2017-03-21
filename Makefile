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
	@$(BINARY) roms/bootrom.bin roms/tetris.gb

clean:
	rm -rf build/*
	mkdir -p build/memory
