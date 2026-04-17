CXX = clang++

# Pastikan namanya CXXFLAGS dan gunakan -std=c++17
CXXFLAGS = -std=c++17 -Wall -Wextra

# Nama file output
TARGET = main

# File sumber (sesuaikan folder src/)
SRC = src/main.cc

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET).exe

clean:
	del $(TARGET).exe
