CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude

AVR_CC = avr-gcc
AVR_CFLAGS = -mmcu=atmega328p -Os -nostartfiles -nostdlib -Wl,-e,main -Wl,--section-start=.isr=0x10

TARGET = avr_emulator
AVR_ELF = program.elf
AVR_HEX = program.hex
TEST_GPIO = test_gpio
TEST_STACK = test_stack
TEST_INSTRUCTIONS = test_instructions

# Emulator sources: exclude the AVR guest program and standalone tests.
SRC = $(filter-out src/program.c src/test_%.c, $(wildcard src/*.c))
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)

# -------------------------
# Debug / optimization
# -------------------------

DEBUG ?= 1

ifeq ($(DEBUG),1)
	CFLAGS += -DDEBUG
else
	CFLAGS += -O2
endif

.PHONY: all run clean test

all: $(TARGET) $(AVR_HEX)

# -------------------------
# Build emulator
# -------------------------

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# -------------------------
# Build AVR guest program
# -------------------------

$(AVR_ELF): src/program.c
	$(AVR_CC) $(AVR_CFLAGS) -o $@ $<

$(AVR_HEX): $(AVR_ELF)
	avr-objcopy -O ihex -R .eeprom $< $@

$(TEST_GPIO): src/test_gpio.c src/gpio.c
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_STACK): src/test_stack.c $(filter-out src/main.c, $(SRC))
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_INSTRUCTIONS): src/test_instructions.c $(filter-out src/main.c, $(SRC))
	$(CC) $(CFLAGS) -o $@ $^

test: $(TEST_GPIO) $(TEST_STACK) $(TEST_INSTRUCTIONS)
	./$(TEST_GPIO)
	./$(TEST_STACK)
	./$(TEST_INSTRUCTIONS)

# -------------------------
# Run
# -------------------------

run: all
	./$(TARGET)

# -------------------------
# Clean
# -------------------------

clean:
	rm -f $(TARGET) $(AVR_ELF) $(AVR_HEX) $(TEST_GPIO) $(TEST_STACK) $(TEST_INSTRUCTIONS) src/*.o src/*.d

-include $(DEP)