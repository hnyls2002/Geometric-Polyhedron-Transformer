# get the name of the test file
name=$1
cp ./testcases/$name.c ./testspace/input.c
cp ./testcases/$name.c.clay.scop ./testspace/result.scop
./build/transformer ./testspace/input.c
./build/codegen ./testspace/result.scop ./testspace/result.c