.PHONY: all linux windows test doc-build doc-dev doc copy clean

all: linux windows

build/linux/Makefile:
	mkdir -p build/linux
	cmake -B build/linux -S . -DCMAKE_BUILD_TYPE=Debug -DBUILD_EXAMPLES=ON -DBUILD_TEST=ON -DUSE_MOLD=ON

linux: build/linux/Makefile
	cmake --build build/linux -j `nproc`

build/windows/ZeroErr.sln:
	mkdir -p build/windows
	cmake.exe -B build/windows -S . -DBUILD_EXAMPLES=ON -DBUILD_TEST=ON -T host=x64 -A x64

windows: build/windows/ZeroErr.sln
	cmake.exe --build build/windows --config Debug -j `nproc`

test: linux windows
	cd build/linux/test && ./unittest
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