# Compiler
CC = /mingw64/bin/gcc

# Temporary directories for MSYS2
export TMP=/c/Users/HP/AppData/Local/Temp
export TEMP=/c/Users/HP/AppData/Local/Temp

# Compiler flags
CFLAGS = -Wall -Wextra -I"C:/msys64/mingw64/include" -I"./"

# Linker flags
LDFLAGS = -L/mingw64/lib -lws2_32 -lpaho-mqtt3c

# Source files
SRC = main.c \
      modbus_functions.c \
      drive_feedback.c \
      drive_parameters.c \
      drive_command.c \
      telemetry.c \
      heartbeat.c \
      lcu_comm.c \
      mqtt_client.c \
      ini.c \
      command_parser.c \
      command_handler.c \
      cJSON.c

# Directories
OBJDIR = obj
BINDIR = bin

# Object files
OBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

# Target executable
TARGET = $(BINDIR)/drive_control.exe

# Default rule
all: $(TARGET)

# Link executable
$(TARGET): $(OBJ)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

# Compile C source files
$(OBJDIR)/%.o: %.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf $(OBJDIR) $(BINDIR)
	@echo "Clean complete"

# Phony targets
.PHONY: all clean
