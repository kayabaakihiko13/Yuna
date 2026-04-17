CXX = clang++

# Pastikan namanya CXXFLAGS dan gunakan -std=c++17
CXXFLAGS = -std=c++17 -Wall -Wextra

# Folder for output
RESULT_BUILDED_DIR = dist

ifeq ($(OS),Windows_NT)
	TARGET = $(RESULT_BUILDED_DIR)/test_window.exe
else
	TARGET = $(RESULT_BUILDED_DIR)/test_linux
endif

# File sumber (sesuaikan folder src/)
SRC = src/main.cc

all: $(TARGET)

$(TARGET): $(SRC) | $(RESULT_BUILDED_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

# make folder dist if not exits
$(DIST_DIR):
	mkdir -p $(DIST_DIR)

clean:
ifeq ($(OS),Windows_NT)
	rmdir /s /q $(DIST_DIR)
else
	rm -rf $(DIST_DIR)
endif

.PHONY: all clean
