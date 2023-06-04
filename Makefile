TEST_DIR = ./testspace
SRC_DIR = ./src
BUILD_DIR = ./build

# build the compiler from source
build: $(SRC_DIR)/transformer.cpp $(SRC_DIR)/codegen.cpp
	@g++ -DCLOOG_INT_GMP $(SRC_DIR)/transformer.cpp -lcloog-isl -lclan -fsanitize=address,undefined -losl -o $(BUILD_DIR)/transformer && \
	g++ $(SRC_DIR)/codegen.cpp -lcloog-isl -lclan -losl -o $(BUILD_DIR)/codegen

# compile the input.c file
trans-test: build
	@$(BUILD_DIR)/transformer $(TEST_DIR)/input.c
#	@echo ============================================================
#	@echo "output.c"
#	@cat $(TEST_DIR)/output.c
#	@echo ------------------------------------------------------------

# codegen result.c from result.scop
codegen-test: build
	@$(BUILD_DIR)/codegen $(TEST_DIR)/result.scop $(TEST_DIR)/result.c
#	@echo ============================================================	
#	@echo "result.c:"
#	@cat $(TEST_DIR)/result.c
#	@echo ------------------------------------------------------------

test: trans-test codegen-test

clean:
	rm -rf $(BUILD_DIR)/*

.PHONY: $(SRC_DIR)/transformer.cpp