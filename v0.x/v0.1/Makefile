# Makefile for Mino v0.1
CC = gcc
CFLAGS = -Wall -I./System -I.
TARGET = mino_test
BUILD_DIR = build

# 源文件路径
SYSTEM_DIR = System
EXAMPLES_DIR = examples

# 源文件
SOURCES = $(SYSTEM_DIR)/System.c \
          $(EXAMPLES_DIR)/test.c

# 目标文件
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR) $(TARGET)

# 创建构建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/$(SYSTEM_DIR)
	mkdir -p $(BUILD_DIR)/$(EXAMPLES_DIR)

# 链接可执行文件
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@

# 编译System.c
$(BUILD_DIR)/$(SYSTEM_DIR)/System.o: $(SYSTEM_DIR)/System.c $(SYSTEM_DIR)/System.h
	$(CC) $(CFLAGS) -c $< -o $@

# 编译test.c
$(BUILD_DIR)/$(EXAMPLES_DIR)/test.o: $(EXAMPLES_DIR)/test.c
	$(CC) $(CFLAGS) -c $< -o $@

# 运行测试
run: $(TARGET)
	@echo "Running $(TARGET)..."
	@./$(TARGET)

# 清理
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

# 安装头文件到系统目录
install:
	cp $(SYSTEM_DIR)/System.h /usr/local/include/
	@echo "System.h installed to /usr/local/include/"

# 卸载
uninstall:
	rm -f /usr/local/include/System.h
	@echo "System.h removed from /usr/local/include/"

.PHONY: all run clean install uninstall