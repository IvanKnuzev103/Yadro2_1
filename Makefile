all:
	@g++ -std=c++17 main.cpp -o task

run: all
	@./task in.txt

clean:
	@rm -f task result.txt