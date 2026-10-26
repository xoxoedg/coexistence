CC = gcc
CFLAGS = -Wall -Wextra -Wconversion -std=gnu11
DEPFLAGS = -MMD -MP
BUILD = build
SRC = $(wildcard src/*.c)
OBJ = $(SRC:src/%.c=$(BUILD)/%.o)
DEP = $(OBJ:.o=.d)
.PHONY: clean

$(BUILD)/main: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@ 


$(BUILD)/%.o: src/%.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $<  -o $@


clean:
	rm -rf $(BUILD) 


-include $(DEP)
