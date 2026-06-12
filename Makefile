# Makefile pour le compilateur Scalin
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -Iinclude
SRC_DIR = src
BUILD_DIR = build
TARGET = scalin

SOURCES = \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/lexer/lexer.cpp \
	$(SRC_DIR)/parser/parser.cpp \
	$(SRC_DIR)/semantic/semantic.cpp \
	$(SRC_DIR)/codegen/codegen.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/$(TARGET) $^

%.out: %.sc
	./build/$(TARGET) $< -o $@
	@echo "Compilation réussie : $@"

clean:
	rm -rf $(BUILD_DIR)/*

test_lexer: tests/test_lexer.cpp $(SOURCES)
	$(CXX) $(CXXFLAGS) -o tests/test_lexer tests/test_lexer.cpp $(SOURCES)
	./tests/test_lexer

.PHONY: all clean test_lexer