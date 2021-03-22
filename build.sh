rm -rf build/*
mkdir -p build

cd build
cmake .. -CMAKE_BUILD_FLAG=DEBUG
make -j4
cd ..
