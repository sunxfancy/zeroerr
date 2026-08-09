.PHONY: all linux windows test linux-test windows-test doc clean

# Tool overrides (optional): CLANGXX=... CLANG=... CMAKE=...
CLANGXX ?= clang++
CLANG   ?= clang
CMAKE   ?= cmake

# Discover clang compiler-rt / libFuzzer link directory.
# Prefer explicit override, then clang++ -print-runtime-dir, then dirname of fuzzer_no_main.
CLANG_RUNTIME_DIR ?= $(shell $(CLANGXX) -print-runtime-dir 2>/dev/null)
ifeq ($(strip $(CLANG_RUNTIME_DIR)),)
CLANG_RUNTIME_DIR := $(shell dirname $$($(CLANGXX) -print-file-name=libclang_rt.fuzzer_no_main-x86_64.a 2>/dev/null) 2>/dev/null)
endif

# Only pass -L when the directory really exists.
ifneq ($(wildcard $(CLANG_RUNTIME_DIR)/.),)
FUZZ_LINK_FLAGS := -L$(CLANG_RUNTIME_DIR)
ENABLE_FUZZING  ?= ON
else
FUZZ_LINK_FLAGS :=
ENABLE_FUZZING  ?= OFF
$(info [zeroerr] clang runtime dir not found; ENABLE_FUZZING=$(ENABLE_FUZZING))
endif

# Linker flags for fuzzing builds (override with FUZZ_LINK_FLAGS=...).
LINUX_FUZZ_CMAKE_FLAGS := -DENABLE_FUZZING=$(ENABLE_FUZZING)
ifneq ($(strip $(FUZZ_LINK_FLAGS)),)
LINUX_FUZZ_CMAKE_FLAGS += -DCMAKE_EXE_LINKER_FLAGS=$(FUZZ_LINK_FLAGS) -DCMAKE_CXX_FLAGS=$(FUZZ_LINK_FLAGS)
endif

all: linux windows

build/linux/Makefile: Makefile
	mkdir -p build/linux
	$(CMAKE) -B build/linux -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=11 \
		-DBUILD_EXAMPLES=ON -DBUILD_TEST=ON -DUSE_MOLD=OFF -DDISABLE_CUDA_BUILD=OFF \
		-DCMAKE_CXX_COMPILER=$(CLANGXX) -DCMAKE_C_COMPILER=$(CLANG) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		$(LINUX_FUZZ_CMAKE_FLAGS)

linux: build/linux/Makefile
	$(CMAKE) --build build/linux -j `nproc`

build/windows/ZeroErr.sln: Makefile
	mkdir -p build/windows
	cmake.exe -B build/windows -S . \
		-DBUILD_EXAMPLES=ON -DBUILD_TEST=ON -DDISABLE_CUDA_BUILD=OFF -T host=x64 -A x64

windows: build/windows/ZeroErr.sln
	cmake.exe --build build/windows --config Debug -j `nproc`

build/macosx/Makefile: Makefile
	mkdir -p build/macosx
	$(CMAKE) -B build/macosx -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=11 \
		-DBUILD_EXAMPLES=ON -DBUILD_TEST=ON -DUSE_MOLD=ON -DDISABLE_CUDA_BUILD=OFF -DENABLE_FUZZING=OFF \
		-DCMAKE_CXX_COMPILER=$(CLANGXX) -DCMAKE_C_COMPILER=$(CLANG)

macosx: build/macosx/Makefile
	$(CMAKE) --build build/macosx -j 4

test: linux-test windows-test fuzz-test

cuda-test: windows
	cd build/windows/test && ./Debug/cudatest.exe

fuzz-test: linux
	cd build/linux/test && ./unittest --testcase=fuzz_serialize.*

fuzz: linux
	cd build/linux/test && ./unittest -f --testcase=presentation

linux-test: linux
	cd build/linux/test && ./unittest

reporter: linux
	cd build/linux/test && ./unittest --no-color --log-to-report --reporters=xml

windows-test: windows
	cd build/windows/test && ../examples/Debug/2_log.exe --testcase="parsing test"

macosx-test: macosx
	cd build/macosx/test && ./unittest

build/linux-release/Makefile: Makefile
	mkdir -p build/linux-release
	$(CMAKE) -B build/linux-release -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=11 \
		-DBUILD_EXAMPLES=ON -DBUILD_TEST=ON -DUSE_MOLD=ON \
		-DCMAKE_CXX_COMPILER=$(CLANGXX) -DCMAKE_C_COMPILER=$(CLANG) \
		$(LINUX_FUZZ_CMAKE_FLAGS)

linux-release: build/linux-release/Makefile
	$(CMAKE) --build build/linux-release -j `nproc`

bench: linux-release
	cd build/linux-release/test && ./unittest -b --testcase=speedtest

doc:
	mkdir -p build-linux-doc
	cd build-linux-doc && $(CMAKE) .. -DCMAKE_BUILD_TYPE=Debug \
		-DBUILD_EXAMPLES=ON -DBUILD_DOC=ON && \
		$(CMAKE) --build . --target doxy -j `nproc`

clean:
	rm -rf build
