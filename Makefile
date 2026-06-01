# ============================================================
# Makefile – Computer Graphics Project (Linux / GTK3 + Cairo)
# ============================================================
# Usage:
#   make          – build the binary
#   make run      – build and run
#   make clean    – remove build artifacts

CXX      := g++
TARGET   := cg_project
SRC      := LinuxMain.cpp

# GTK3 flags (includes Cairo automatically)
GTK_CFLAGS  := $(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS    := $(shell pkg-config --libs   gtk+-3.0)

CXXFLAGS := -std=c++17 -Wall -Wextra -O2 $(GTK_CFLAGS)
LDFLAGS  := $(GTK_LIBS) -lm

# ─────────────────────────────────────────────
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)
	@echo "Build successful -> ./$(TARGET)"

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
