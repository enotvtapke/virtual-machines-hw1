cmake -B cmake-build-debug
cd cmake-build-debug
cmake --build .
./main
cd ..
python3 ./cache_size.py