CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2
LDFLAGS :=

# Include directories
INCLUDES := -I src/include -I test/include
# Output directory
DIST_DIR := dist

# Source files
MAIN_SRC := src/main.cc
TEST_SRC := test/main_tests.cc

# Platform-specific settings
ifeq ($(OS),Windows_NT)
    TARGET := $(DIST_DIR)/main.exe
    TEST_TARGET := $(DIST_DIR)/test_window.exe
    RUN_CMD := 
    RM_CMD := if exist $(DIST_DIR) rmdir /s /q $(DIST_DIR)
else
    TARGET := $(DIST_DIR)/main
    TEST_TARGET := $(DIST_DIR)/test_linux
    RUN_CMD := ./
    RM_CMD := rm -rf $(DIST_DIR)
endif

.PHONY: all clean rebuild run test test-run help dist

# Default target
all: $(TARGET)

# Create dist directory ← PASTIKAN BARIS BERIKUTNYA PAKAI TAB!
dist:
	mkdir $(DIST_DIR)

# Build main program
$(TARGET): $(MAIN_SRC) | dist
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(MAIN_SRC) $(LDFLAGS) -o $(TARGET)

# Build test program
$(TEST_TARGET): $(TEST_SRC) | dist
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TEST_SRC) $(LDFLAGS) -o $(TEST_TARGET)

# Clean build artifacts
clean:
	-@$(RM_CMD)
	-@echo Clean complete.

rebuild: clean all

# Run main program
run: $(TARGET)
	$(RUN_CMD)$(TARGET)

# Build and run tests
test: $(TEST_TARGET)
	$(RUN_CMD)$(TEST_TARGET)

# Help message (for developer)
help:
	@echo "🛠️  Available targets:"
	@echo "  all        - Build main program (default)"
	@echo "  run        - Build and run main program"
	@echo "  test       - Build and run tests"
	@echo "  clean      - Remove build artifacts"
	@echo "  rebuild    - Clean + rebuild all"
	@echo "  help       - Show this message"