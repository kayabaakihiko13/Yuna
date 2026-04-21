CXX := clang++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2
LDFLAGS :=
INCLUDES := -I src/include -I test/include

DIST_DIR := dist
APP_DIR := $(DIST_DIR)/app
TEST_DIR := $(DIST_DIR)/test

MAIN_SRC := src/main.cc
TEST_SRC := test/main_tests.cc

# DETEKSI OS
ifeq ($(OS),Windows_NT)
    TARGET := $(APP_DIR)/main.exe
    TEST_TARGET := $(TEST_DIR)/test_window.exe
    RUN_CMD := 
    # Pakai shell bawaan Windows yang simpel
    MKDIR_CMD := if not exist
    RM_CMD := if exist
    RM_OPTS := /s /q
    KILL_CMD := taskkill /F /IM main.exe /T >nul 2>&1 || exit 0
else
    TARGET := $(APP_DIR)/main
    TEST_TARGET := $(TEST_DIR)/test_linux
    RUN_CMD := ./
    MKDIR_CMD := mkdir -p
    RM_CMD := rm -rf
    RM_OPTS := 
    KILL_CMD := pkill -f main || true
endif

.PHONY: all clean rebuild run test help dist

all: $(TARGET)

# Rule Buat Folder (Handle Windows vs Linux manual)
$(APP_DIR):
ifeq ($(OS),Windows_NT)
	@if not exist "$(DIST_DIR)" mkdir "$(DIST_DIR)"
	@if not exist "$(APP_DIR)" mkdir "$(subst /,\,$(APP_DIR))"
else
	@mkdir -p $(APP_DIR)
endif

$(TEST_DIR):
ifeq ($(OS),Windows_NT)
	@if not exist "$(DIST_DIR)" mkdir "$(DIST_DIR)"
	@if not exist "$(TEST_DIR)" mkdir "$(subst /,\,$(TEST_DIR))"
else
	@mkdir -p $(TEST_DIR)
endif

# Build Program Utama
$(TARGET): $(MAIN_SRC) | $(APP_DIR)
	-@$(KILL_CMD)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(MAIN_SRC) $(LDFLAGS) -o $(TARGET)
	@echo "App build complete: $(TARGET)"

# Build Test
$(TEST_TARGET): $(TEST_SRC) | $(TEST_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(TEST_SRC) $(LDFLAGS) -o $(TEST_TARGET)
	@echo "Test build complete: $(TEST_TARGET)"

clean:
ifeq ($(OS),Windows_NT)
	@if exist "$(DIST_DIR)" rmdir /s /q "$(DIST_DIR)"
else
	@rm -rf $(DIST_DIR)
endif
	@echo "Clean complete."

run: $(TARGET)
	@$(RUN_CMD)$(TARGET)

test: $(TEST_TARGET)
	@$(RUN_CMD)$(TEST_TARGET)

rebuild: clean all
