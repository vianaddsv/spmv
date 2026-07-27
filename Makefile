BUILDDIR := build

.PHONY: all test format clean

all: $(BUILDDIR)/Makefile
	cmake --build $(BUILDDIR)

$(BUILDDIR)/Makefile: CMakeLists.txt csr/csr_matrix.cpp csr/csr_matrix.hpp test/test_csr_matrix.cpp
	cmake -S . -B $(BUILDDIR)
	ln -sf $(BUILDDIR)/compile_commands.json compile_commands.json

test: $(BUILDDIR)/Makefile
	cmake --build $(BUILDDIR)
	ctest --test-dir $(BUILDDIR) --output-on-failure

format:
	clang-format -i csr/*.cpp csr/*.hpp test/*.cpp

clean:
	rm -rf $(BUILDDIR)
