CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2
LDFLAGS :=

# Include directories
INCLUDES := -I src/include -I test/include

# Output directory
DIST_DIR := dist
APP_DIR := $(DIST_DIR)/app
TEST_DIR := $(DIST_DIR)/test

# Source files
MAIN_SRC := src/main.cc
TEST_SRC := test/main_tests.cc

# Platform-specific settings
ifeq ($(OS),Windows_NT)
    # Windows settings
    TARGET := $(APP_DIR)/main.exe
    TEST_TARGET := $(TEST_DIR)/test_window.exe
    RUN_CMD := 
    MKDIR_APP_CMD := if not exist $(subst /,\,$(APP_DIR)) mkdir $(subst /,\,$(APP_DIR))
    MKDIR_TEST_CMD := if not exist $(subst /,\,$(TEST_DIR)) mkdir $(subst /,\,$(TEST_DIR))
    RM_ALL_CMD := if exist $(subst /,\,$(DIST_DIR)) rmdir /s /q $(subst /,\,$(DIST_DIR))
    RM_TEST_CMD := if exist $(subst /,\,$(TEST_DIR)) rmdir /s /q $(subst /,\,$(TEST_DIR))
    # Perintah untuk mematikan proses jika masih berjalan (mencegah LNK1104)
    KILL_CMD := taskkill /F /IM main.exe /T >nul 2>&1 || (exit 0)
    KILL_TEST_CMD := taskkill /F /IM test_window.exe /T >nul 2>&1 || (exit 0)
else
    # Linux/macOS settings
    TARGET := $(APP_DIR)/main
    TEST_TARGET := $(TEST_DIR)/test_linux
    RUN_CMD := ./
    MKDIR_APP_CMD := mkdir -p $(APP_DIR)
    MKDIR_TEST_CMD := mkdir -p $(TEST_DIR)
    RM_ALL_CMD := rm -rf $(DIST_DIR)
    RM_TEST_CMD := rm -rf $(TEST_DIR)
    KILL_CMD := pkill -f $(TARGET) || true
    KILL_TEST_CMD := pkill -f $(TEST_TARGET) || true
    SHELL := /bin/bash
endif

.PHONY: all clean rebuild run test help dist clean-test

# Default target
all: $(TARGET)

# Rule untuk membuat direktori
$(APP_DIR):
	@$(MKDIR_APP_CMD)

$(TEST_DIR):
	@$(MKDIR_TEST_CMD)

# Helper untuk memicu pembuatan folder
dist: $(APP_DIR) $(TEST_DIR)

# Build main program
$(TARGET): $(MAIN_SRC) | $(APP_DIR)
	@echo "Checking for running instances..."
	-@$(KILL_CMD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(MAIN_SRC) $(LDFLAGS) -o $(TARGET)
	@echo "App build complete: $(TARGET)"

# Build test program
$(TEST_TARGET): $(TEST_SRC) | $(TEST_DIR)
	@echo "Checking for running test instances..."
	-@$(KILL_TEST_CMD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TEST_SRC) $(LDFLAGS) -o $(TEST_TARGET)
	@echo "Test build complete: $(TEST_TARGET)"

# Clean build artifacts
clean:
	-@$(RM_ALL_CMD)
	@echo "Clean complete."

clean-test:
	-@$(RM_TEST_CMD)
	@echo "Test Clean Complete"

# Run main program
run: $(TARGET)
	@echo "Running app..."
	@$(RUN_CMD)$(TARGET)

# Build and run tests
test: $(TEST_TARGET)
	@echo "Running tests..."
	@$(RUN_CMD)$(TEST_TARGET)

rebuild: clean all

# Help message
help:
	@echo "Available targets:"
	@echo "  all        - Build main program (default)"
	@echo "  run        - Build and run main program"
	@echo "  test       - Build and run tests"
	@echo "  clean      - Remove all build artifacts"
	@echo "  clean-test - Remove only test artifacts"
	@echo "  rebuild    - Clean + rebuild all"
