# Demo makefile for hello_world.cpp. Only works if placed
# in the build artifacts directory. Needs libgcc 5
# installed, see md_QUICKSTART.html in the included docs.

.PHONY: clean

CXX      = g++
LD       = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -Iinclude -Ideps/include -fopenmp -D_GLIBCXX_USE_CXX11_ABI=0 -DOPHELIB_ENABLE_EXCEPTIONS
LDFLAGS  = -fopenmp
LDLIBS   = lib/libophelib.a deps/lib/libntl.a deps/lib/libgmp.a -lpthread

BINARY   = hello_world

$(BINARY): $(BINARY).o
	$(CXX) $(LDFLAGS) $(BINARY).o -o $(BINARY) $(LDLIBS)

$(BINARY).o: $(BINARY).cpp
	$(LD) $(CXXFLAGS) -c $(BINARY).cpp

clean:
	rm -f $(BINARY) $(BINARY).o
