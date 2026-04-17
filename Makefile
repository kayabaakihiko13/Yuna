CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -I include

DIST_DIR = dist

ifeq ($(OS),Windows_NT)
    TARGET = $(DIST_DIR)/test_window.exe
else
    TARGET = $(DIST_DIR)/test_linux
endif

SRC = src/main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(DIST_DIR)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
ifeq ($(OS),Windows_NT)
	rmdir /s /q $(DIST_DIR)
else
	rm -rf $(DIST_DIR)
endif

rebuild: clean all

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean rebuild run