.PHONY: all linux windows

all: linux windows

linux:
	mkdir -p build-linux
	cd build-linux && cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_EXAMPLES=ON -DBUILD_DOC=ON && cmake --build . -j `nproc`

windows:
	mkdir -p build-windows
	cd build-windows && cmake.exe -DBUILD_EXAMPLES=ON -DBUILD_DOC=ON -T host=x64 -A x64 .. && cmake.exe --build . --config Debug -j `nproc`

doc-dev:
	yarn run cmake:dev

doc: linux
	yarn run cmake:docs

copy:
	cp -r docs/.vuepress  build-linux/docs/
	cp -r docs/.vuepress  build-windows/docs/

clean:
	rm -rf build-linux build-windows