A := http_server/src/logger.cpp
B := http_server/src/http_server.cpp
MAIN := main.cpp
TARGET := app

all: $(TARGET)

$(TARGET): $(A) $(B) $(MAIN)
	@g++ -std=c++23 -g -Wall -Wextra -pedantic \
		-I./http_server/include \
		$(A) \
		$(B) \
		$(MAIN) -o $(TARGET)

.PHONY: clean run

clean:
	@rm -rf $(TARGET)

run: app
	@./$(TARGET)
	@rm -rf $(TARGET)
