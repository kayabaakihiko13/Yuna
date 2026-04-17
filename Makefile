CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2
LDFLAGS :=

# configuration include file project
INCLUDES := -I src/include -I test/include

DIST_DIR := dist

ifeq ($(OS),Windows_NT)
    TARGET := $(DIST_DIR)/main.exe
    TEST_TARGET := $(DIST_DIR)/test_window.exe
    MKDIR := if not exist $(DIST_DIR) mkdir $(DIST_DIR)
    RM := rmdir /s /q
    RUN_CMD := 
else
    TARGET := $(DIST_DIR)/main
    TEST_TARGET := $(DIST_DIR)/test_linux
    MKDIR := mkdir -p
    RM := rm -rf
    RUN_CMD := ./
endif

MAIN_SRC := src/main.cc
TEST_SRC := test/main_tests.cc

.PHONY: all clean rebuild run test test-run help

all: $(TARGET)

$(TARGET): $(MAIN_SRC) | dist
	@$(MKDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(MAIN_SRC) $(LDFLAGS) -o $(TARGET)

$(TEST_TARGET): $(TEST_SRC) | dist
	@$(MKDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TEST_SRC) $(LDFLAGS) -o $(TEST_TARGET)

dist:
	@$(MKDIR)

clean:
	-$(RM) $(DIST_DIR) 2>nul || true
	-$(RM) *.exe 2>nul || true
	-@echo ✓ Clean complete.

rebuild: clean all

run: $(TARGET)
	$(RUN_CMD)$(TARGET)

test: $(TEST_TARGET)
	$(RUN_CMD)$(TEST_TARGET)

test-run:
	$(RUN_CMD)$(TEST_TARGET)

help:
	@echo "Available targets: all, clean, rebuild, run, test, help"