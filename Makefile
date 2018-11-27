#PATH:=/usr/local/Cellar/llvm/7.0.0_1/bin:$(PATH)
CXXFLAGS=`llvm-config --cxxflags` -g -std=c++14
LDLIBS=`llvm-config --libs core irreader object bitreader` -lcurses -lLLVMDemangle
LDFLAGS=`llvm-config --ldflags`
all: memcpy-find

clean:
	rm -f memcpy-find
