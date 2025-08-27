# Example programs Makefile
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread
SRCDIR = .
BUILDDIR = ../build/examples

# Create build directory
$(shell mkdir -p $(BUILDDIR))

# Example targets
EXAMPLES = cpu_bound_example memory_bound_example load_imbalance_example

all: $(EXAMPLES)

cpu_bound_example: $(SRCDIR)/cpu_bound_example.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)/$@ $<

memory_bound_example: $(SRCDIR)/memory_bound_example.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)/$@ $<

load_imbalance_example: $(SRCDIR)/load_imbalance_example.cpp
	$(CXX) $(CXXFLAGS) -o $(BUILDDIR)/$@ $<

clean:
	rm -rf $(BUILDDIR)

.PHONY: all clean