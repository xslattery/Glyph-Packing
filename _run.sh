
echo ""
echo "Compiling Program on: "
date
echo ""

DIR_PATH=$(dirname "$SCRIPT")
# echo $DIR_PATH

g++ -arch x86_64 -Wno-c++11-extensions -Wno-local-type-template-args -framework OpenGL -I $DIR_PATH/Include -L $DIR_PATH/Frameworks -l glfw33 -l freetyped2_7 main.cpp shader.cpp -o main

echo ""
echo "Running..."
echo ""
echo ""

./main

echo ""

# -l glfw33 