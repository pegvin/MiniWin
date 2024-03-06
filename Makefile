CC:=gcc
CFLAGS:=-Isrc/ -std=c99 -Wall -MMD -MP
LFLAGS:=

BUILD      := build
BUILD_TYPE := Debug
BIN        := $(BUILD)/miniwin

SOURCES := src/miniwin.c
OBJECTS := $(SOURCES:.c=.c.o)
OBJECTS := $(patsubst %,$(BUILD)/%,$(OBJECTS))
DEPENDS := $(OBJECTS:.o=.d)

# Check if `bear` command is available, Bear is used to generate
# `compile_commands.json` for your LSP to use, but can be disabled
# in command line by `make all BEAR=''`
# URL: github.com/rizsotto/Bear
# Note: Using multiple jobs with bear is not supported, i.e.
#       `make all -j4` won't work with bear enabled
BEAR :=
ifneq (, $(shell which bear))
	BEAR:=bear --append --output $(BUILD)/compile_commands.json --
endif

ifeq ($(BUILD_TYPE),Debug)
	CFLAGS+=-O0 -g
else
	ifeq ($(BUILD_TYPE),Release)
		CFLAGS+=-O3
	else
$(error Unknown Build Type "$(BUILD_TYPE)")
	endif
endif

all: $(BIN)

-include $(DEPENDS)

$(BUILD)/%.c.o: %.c
	@echo "CC -" $<
	@mkdir -p "$$(dirname "$@")"
	@$(BEAR) $(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OBJECTS)
	@echo "LD -" $@
	@$(CC) $(OBJECTS) $(LFLAGS) -o $@

.PHONY: run
.PHONY: clean

run: all
	@./$(BIN)

clean:
	@$(RM) -rv $(BIN) $(BUILD)

