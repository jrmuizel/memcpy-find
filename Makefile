UNAME_S:=$(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	PATH:=/usr/local/opt/llvm/bin:$(PATH)
endif
# llvm-config -cxxflags gives the flags that llvm was built
# with instead of flags needed to build against llvm
CXX=clang++
CXXFLAGS=`llvm-config --cxxflags` -g -std=c++14
LDLIBS=`llvm-config --libs core irreader object bitreader` -lcurses -lLLVMDemangle
LDFLAGS=`llvm-config --ldflags`

.PHONY: all clean

all: memcpy-find

clean:
	rm -f memcpy-find
