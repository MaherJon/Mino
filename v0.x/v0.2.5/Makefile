# Makefile for Mino Compiler v0.2.5
CC = gcc
CFLAGS = -Wall -Wextra -g -I./include
LDFLAGS = 
TARGET = bin/minoc
BUILD_DIR = build

# Source files
SRC_DIR = src
LEXER_SRC = $(SRC_DIR)/lexer/lexer.c
PARSER_SRC = $(SRC_DIR)/parser/parser.c
AST_SRC = $(SRC_DIR)/ast/ast.c
SEMANTIC_SRC = $(SRC_DIR)/semantic/semantic.c
MAIN_SRC = $(SRC_DIR)/main.c
CODEGEN_SRC = $(SRC_DIR)/codegen/codegen.c

# Header files
INCLUDE_DIR = include
LEXER_H = $(INCLUDE_DIR)/lexer.h
TOKENS_H = $(INCLUDE_DIR)/tokens.h
AST_H = $(INCLUDE_DIR)/ast.h
SEMANTIC_H = $(INCLUDE_DIR)/semantic.h
PARSER_H = $(INCLUDE_DIR)/parser.h

# Object files
OBJS = $(BUILD_DIR)/lexer.o $(BUILD_DIR)/parser.o \
	$(BUILD_DIR)/ast.o $(BUILD_DIR)/semantic.o \
	$(BUILD_DIR)/codegen.o $(BUILD_DIR)/main.o

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p bin

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/lexer.o: $(LEXER_SRC) $(LEXER_H) $(TOKENS_H)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/parser.o: $(PARSER_SRC) $(LEXER_H) $(AST_H) $(PARSER_H)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/ast.o: $(AST_SRC) $(AST_H) $(TOKENS_H)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/semantic.o: $(SEMANTIC_SRC) $(SEMANTIC_H) $(AST_H)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/codegen.o: $(CODEGEN_SRC) $(AST_H)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main.o: $(MAIN_SRC) $(LEXER_H) $(AST_H) $(PARSER_H) $(SEMANTIC_H)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	@echo "=== Testing Lexer ==="
	./$(TARGET) --test "func main() { let x: int = 42; return x; }"
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

.PHONY: all test run clean install