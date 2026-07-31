BUILDDIR := build
BUILDDIR_NOOPT := build-noopt

.PHONY: all noopt test format clean

all: $(BUILDDIR)/Makefile
	cmake --build $(BUILDDIR)

noopt: $(BUILDDIR_NOOPT)/Makefile
	cmake --build $(BUILDDIR_NOOPT)

$(BUILDDIR)/Makefile: CMakeLists.txt csr/csr_matrix.cpp csr/csr_matrix.hpp test/test_csr_matrix.cpp
	cmake -S . -B $(BUILDDIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-march=native"
	ln -sf $(BUILDDIR)/compile_commands.json compile_commands.json

$(BUILDDIR_NOOPT)/Makefile: CMakeLists.txt csr/csr_matrix.cpp csr/csr_matrix.hpp test/test_csr_matrix.cpp
	cmake -S . -B $(BUILDDIR_NOOPT) -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-O0 -g"
	ln -sf $(BUILDDIR_NOOPT)/compile_commands.json compile_commands.json

test: $(BUILDDIR)/Makefile
	cmake --build $(BUILDDIR)
	ctest --test-dir $(BUILDDIR) --output-on-failure

format:
	clang-format -i csr/*.cpp csr/*.hpp test/*.cpp

clean:
	rm -rf $(BUILDDIR) $(BUILDDIR_NOOPT)
