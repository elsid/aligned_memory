CXXFLAGS = -Wall -std=c++11

all:
	$(CXX) $(CXXFLAGS) -o aligned_memory main.cpp

check: all
	./aligned_memory

clean:
	rm aligned_memory

.PHONY: all clean check
