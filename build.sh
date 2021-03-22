rm -rf build/*
mkdir -p build

cd build
cmake .. -DCMAKE_BUILD_FLAG=DEBUG
make -j4
cd ..
