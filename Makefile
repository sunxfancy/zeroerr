.PHONY: all linux windows test linux-test windows-test doc-build doc-dev doc copy clean

all: linux windows

build/linux/Makefile:
	mkdir -p build/linux
	cmake -B build/linux -S . -DCMAKE_BUILD_TYPE=Debug -DBUILD_EXAMPLES=ON -DBUILD_TEST=ON -DUSE_MOLD=ON -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang

linux: build/linux/Makefile
	cmake --build build/linux -j `nproc`

build/windows/ZeroErr.sln:
	mkdir -p build/windows
	cmake.exe -B build/windows -S . -DBUILD_EXAMPLES=ON -DBUILD_TEST=ON -T host=x64 -A x64

windows: build/windows/ZeroErr.sln
	cmake.exe --build build/windows --config Debug -j `nproc`

test: linux-test windows-test fuzz-test

fuzz-test: linux
	cd build/linux/test && ./unittest -f --testcase=fuzz_.*

linux-test: linux
	cd build/linux/test && ./unittest

windows-test: windows
	cd build/windows/test && ./Debug/unittest.exe

doc-dev:
	yarn run cmake:dev

doc-build:
	mkdir -p build-linux-doc
	cd build-linux-doc && cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_EXAMPLES=ON -DBUILD_DOC=ON && cmake --build . --target doxy -j `nproc`

doc: doc-build
	yarn run cmake:docs

copy:
	cp -r docs/.vuepress  build-linux-doc/docs/

clean:
	rm -rf build