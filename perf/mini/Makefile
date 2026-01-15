# Makefile for mini examples - assembly analysis

CXX = g++
CXXFLAGS = -O2 -std=c++17 -I../../include
SRC = itoa_u16.cpp itoa_u32.cpp builder.cpp
EXE = $(SRC:.cpp=_linux.exe)
ASM = $(SRC:.cpp=.s)

.PHONY: all clean run asm

all: $(EXE) $(ASM)

asm: $(ASM)

run: $(EXE)
	@echo "=== itoa_u16 ==="
	./itoa_u16_linux.exe
	@echo "=== itoa_u32 ==="
	./itoa_u32_linux.exe
	@echo "=== builder ==="
	./builder_linux.exe

%_linux.exe: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

%.s: %_linux.exe
	objdump -d -S $< > $@

clean:
	rm -f $(EXE) $(ASM)
