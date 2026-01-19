# Makefile for Mino Compiler
CC = gcc
CFLAGS = -Wall -I./include -I./lib/minolib
TARGET = bin/minoc
BUILD_DIR = build

SRC_DIR = src
LEXER_SRC = $(SRC_DIR)/lexer/lexer.c
PARSER_SRC = $(SRC_DIR)/parser/parser.c
AST_SRC = $(SRC_DIR)/ast/ast.c
MAIN_SRC = $(SRC_DIR)/main.c

INCLUDE_DIR = include
LEXER_H = $(INCLUDE_DIR)/lexer.h
TOKENS_H = $(INCLUDE_DIR)/tokens.h
AST_H = $(INCLUDE_DIR)/ast.h

OBJS = $(BUILD_DIR)/lexer.o $(BUILD_DIR)/parser.o \
       $(BUILD_DIR)/ast.o $(BUILD_DIR)/main.o

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p bin

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/lexer.o: $(LEXER_SRC) $(LEXER_H) $(TOKENS_H)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/parser.o: $(PARSER_SRC) $(LEXER_H) $(AST_H)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ast.o: $(AST_SRC) $(AST_H)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: $(MAIN_SRC) $(LEXER_H) $(AST_H)
	$(CC) $(CFLAGS) -c $< -o $@

setup-includes:
	@echo "Setting up include symlinks..."
	mkdir -p /tmp/mino-includes
	ln -sf $(shell pwd)/include/* /tmp/mino-includes/ 2>/dev/null || true
	@echo "Add -I/tmp/mino-includes to CFLAGS for <xxx.h> includes"

test: $(TARGET)
	@echo "=== Testing Lexer ==="
	./$(TARGET) --test "func main() { return 0; }"
	@echo ""
	@echo "=== Testing with example ==="
	./$(TARGET) examples/simple.mino

run: $(TARGET)
	./$(TARGET) examples/simple.mino

clean:
	rm -rf $(BUILD_DIR) bin

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/minoc
	@echo "Compiler installed to /usr/local/bin/minoc"

.PHONY: all test run clean install setup-includes