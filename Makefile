UNAME_S:=$(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	PATH:=/usr/local/opt/llvm/bin:$(PATH)
endif
CXX=clang++
CXXFLAGS=`llvm-config --cxxflags` -g -std=c++14
LDLIBS=`llvm-config --libs core irreader object bitreader` -lcurses -lLLVMDemangle
LDFLAGS=`llvm-config --ldflags`

.PHONY: all clean

all: memcpy-find

clean:
	rm -f memcpy-find
