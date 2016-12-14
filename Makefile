BINARY := build/byteboy
SRCS := $(shell find . -iname "*.cpp")
OBJS := $(addprefix build/,$(SRCS:.cpp=.o))

CXX := clang++
LD := $(CXX)
override CXXFLAGS += -std=c++11
override LDFLAGS += $(CXXFLAGS)

all:$(BINARY)

$(BINARY):$(OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

build/%.o:%.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

run:$(BINARY)
	@$(BINARY) roms/bootrom.bin roms/blue.gb

clean:
	rm -rf build/*
	mkdir -p build/modules
