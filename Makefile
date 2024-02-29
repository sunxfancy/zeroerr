.PHONY: all linux windows build test doc-build doc-dev doc copy clean

all: linux windows

linux:
	mkdir -p build-linux
	cd build-linux && cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_EXAMPLES=ON -DBUILD_DOC=ON && cmake --build . -j `nproc`

windows:
	mkdir -p build-windows
	cd build-windows && cmake.exe -DBUILD_EXAMPLES=ON -DBUILD_DOC=ON -T host=x64 -A x64 .. && cmake.exe --build . --config Debug -j `nproc`

build:
	mkdir -p build
	cd build && cmake.exe -DBUILD_EXAMPLES=ON -DBUILD_DOC=ON -T host=x64 -A x64 .. && cmake.exe --build . --config Debug -j `nproc`

test: 
	cd build-linux && ./unittest -x
	cd build-windows && ./Debug/unittest.exe

doc-dev:
	yarn run cmake:dev

doc-build:
	mkdir -p build-linux-doc
	cd build-linux-doc && cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_EXAMPLES=ON -DBUILD_DOC=ON && cmake --build . --target doxy -j `nproc`

doc: doc-build
	yarn run cmake:docs

copy:
	cp -r docs/.vuepress  build-linux/docs/
	cp -r docs/.vuepress  build-windows/docs/

clean:
	rm -rf build-linux build-windows