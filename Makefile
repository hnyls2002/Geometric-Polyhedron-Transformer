TEST_DIR = ./testspace
SRC_DIR = ./src

# build the compiler from source
build:
	@g++ -DCLOOG_INT_GMP $(SRC_DIR)/transformer.cpp -lcloog-isl -lclan -losl -o $(TEST_DIR)/transformer

test: build
	@cd $(TEST_DIR); \
	./transformer test.c