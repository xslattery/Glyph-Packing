
echo ""
echo "Compiling Program on: "
date

DIR_PATH=$(dirname "$SCRIPT")
# echo $DIR_PATH

g++ -arch x86_64 -Wno-c++11-extensions -framework OpenGL -I $DIR_PATH/Include -L $DIR_PATH/Frameworks -l glfw33 -l freetyped2_7 main.cpp shader.cpp -o main

echo ""