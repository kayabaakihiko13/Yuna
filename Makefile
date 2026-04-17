CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2
LDFLAGS :=

# Include directories
INCLUDES := -I src/include -I test/include

# Output directory
DIST_DIR := dist

# Platform detection
ifeq ($(OS),Windows_NT)
    TARGET := $(DIST_DIR)/main.exe
    TEST_TARGET := $(DIST_DIR)/test_window.exe
    # Windows CMD syntax
    MKDIR_CMD := if not exist $(DIST_DIR) mkdir $(DIST_DIR)
    RM_CMD := rmdir /s /q
    RUN_CMD := 
else
    TARGET := $(DIST_DIR)/main
    TEST_TARGET := $(DIST_DIR)/test_linux
    # Linux/Unix syntax
    MKDIR_CMD := mkdir -p $(DIST_DIR)
    RM_CMD := rm -rf
    RUN_CMD := ./
endif

# Source files
MAIN_SRC := src/main.cc
TEST_SRC := test/main_tests.cc

# =============================================================================
# Targets
# =============================================================================

.PHONY: all clean rebuild run test test-run help dist

# Default target
all: $(TARGET)

# Target dist sekarang memanggil MKDIR_CMD dengan benar
dist:
	@$(MKDIR_CMD)

# Build main program
$(TARGET): $(MAIN_SRC) | dist
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(MAIN_SRC) $(LDFLAGS) -o $(TARGET)

# Build test program
$(TEST_TARGET): $(TEST_SRC) | dist
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TEST_SRC) $(LDFLAGS) -o $(TEST_TARGET)

# Clean build artifacts
clean:
	-$(RM_CMD) $(DIST_DIR) 2>nul || true
	-@echo ✓ Clean complete.

# Rebuild from scratch
rebuild: clean all

# Run main program
run: $(TARGET)
	$(RUN_CMD)$(TARGET)

# Build and run tests
test: $(TEST_TARGET)
	$(RUN_CMD)$(TEST_TARGET)

# Just run tests (if already built)
test-run:
	$(RUN_CMD)$(TEST_TARGET)
