.PHONY: all linux windows test linux-test windows-test doc clean

all: linux windows

build/linux/Makefile: Makefile
	mkdir -p build/linux
	cmake -B build/linux -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=11 \
		-DBUILD_EXAMPLES=ON -DBUILD_TEST=ON -DUSE_MOLD=ON -DDISABLE_CUDA_BUILD=OFF -DENABLE_FUZZING=ON \
		-DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang

linux: build/linux/Makefile
	cmake --build build/linux -j `nproc`

build/windows/ZeroErr.sln:
	mkdir -p build/windows
	cmake.exe -B build/windows -S . \
		-DBUILD_EXAMPLES=ON -DBUILD_TEST=ON -DDISABLE_CUDA_BUILD=OFF -T host=x64 -A x64

windows: build/windows/ZeroErr.sln
	cmake.exe --build build/windows --config Debug -j `nproc`

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


build/linux-release/Makefile: Makefile
	mkdir -p build/linux-release
	cmake -B build/linux-release -S . -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_STANDARD=11 \
		-DBUILD_EXAMPLES=ON -DBUILD_TEST=ON -DUSE_MOLD=ON -DENABLE_FUZZING=ON \
		-DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang

linux-release: build/linux-release/Makefile
	cmake --build build/linux-release -j `nproc`

bench: linux-release
	cd build/linux-release/test && ./unittest -b --testcase=speedtest

doc:
	mkdir -p build-linux-doc
	cd build-linux-doc && cmake .. -DCMAKE_BUILD_TYPE=Debug \
		-DBUILD_EXAMPLES=ON -DBUILD_DOC=ON && \
		cmake --build . --target doxy -j `nproc`

clean:
	rm -rf build