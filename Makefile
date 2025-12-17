CC      := gcc
CFLAGS  := -O3 -Wall -Wextra
TARGET  := xshell.exe
SRC     := xshell_win.c # If needed, change to xshell_posix.c for POSIX systems

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	del /Q $(TARGET) 2>nul || exit 0
