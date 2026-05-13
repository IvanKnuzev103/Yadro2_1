CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
TARGET = task
SOURCES = main.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

run: all
	./$(TARGET) $(FILE)

run-save: all
	./$(TARGET) $(FILE) > result.txt

clean:
	rm -f $(TARGET) result.txt

.PHONY: all run run-save clean