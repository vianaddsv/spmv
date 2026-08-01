BUILDDIR := build
BUILDDIR_NOOPT := build-noopt
BUILDDIR_TEST := build-test

.PHONY: all noopt test format clean

all: $(BUILDDIR)/Makefile
	cmake --build $(BUILDDIR)

noopt: $(BUILDDIR_NOOPT)/Makefile
	cmake --build $(BUILDDIR_NOOPT)

test: $(BUILDDIR_TEST)/Makefile
	cmake --build $(BUILDDIR_TEST)
	ctest --test-dir $(BUILDDIR_TEST) --output-on-failure

$(BUILDDIR)/Makefile: CMakeLists.txt csr/csr_matrix.cpp csr/csr_matrix.hpp
	cmake -S . -B $(BUILDDIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-march=native" -DBUILD_TESTS=OFF
	ln -sf $(BUILDDIR)/compile_commands.json compile_commands.json

$(BUILDDIR_NOOPT)/Makefile: CMakeLists.txt csr/csr_matrix.cpp csr/csr_matrix.hpp
	cmake -S . -B $(BUILDDIR_NOOPT) -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-O0 -g" -DBUILD_TESTS=OFF
	ln -sf $(BUILDDIR_NOOPT)/compile_commands.json compile_commands.json

$(BUILDDIR_TEST)/Makefile: CMakeLists.txt csr/csr_matrix.cpp csr/csr_matrix.hpp test/test_csr_matrix.cpp
	cmake -S . -B $(BUILDDIR_TEST) -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-march=native" -DBUILD_TESTS=ON
	ln -sf $(BUILDDIR_TEST)/compile_commands.json compile_commands.json

format:
	clang-format -i csr/*.cpp csr/*.hpp test/*.cpp

clean:
	rm -rf $(BUILDDIR) $(BUILDDIR_NOOPT) $(BUILDDIR_TEST)
