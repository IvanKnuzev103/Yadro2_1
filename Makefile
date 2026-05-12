CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = bot

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET)

run: $(TARGET)
	./$(TARGET) $(FILE)

clean:
	rm -f $(TARGET) result.txt result2.txt result3.txt compare.txt
