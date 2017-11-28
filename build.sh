
echo ""
echo "Compiling Program on: "
date

DIR_PATH="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )";
cd $DIR_PATH

clang++ -arch x86_64 -std=c++14 -rpath @loader_path/../libs -I libs/include -L libs -framework OpenGL -l glfw33 -l freetyped2_7 src/main.cpp src/shader.cpp -o build/main

echo ""