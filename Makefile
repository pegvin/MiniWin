CC:=gcc
AR:=ar
CFLAGS:=-Isrc/ -std=c99 -Wall -MMD -MP
LFLAGS:=-lX11

BUILD      := build
BUILD_TYPE := Debug
LIB        := $(BUILD)/miniwin.a

EXAMPLE_SRC := src/example.c
EXAMPLE_BIN := $(BUILD)/miniwin_example

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

ifeq ($(OS),Windows_NT)
	CFLAGS += -DMWIN_BACKEND_WIN32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Linux)
		CFLAGS += -DMWIN_BACKEND_X11=1
	endif
	ifeq ($(UNAME_S),Darwin)
		CFLAGS += -DMWIN_BACKEND_COCOA=1
	endif
endif

all: $(LIB) $(EXAMPLE_BIN)

-include $(DEPENDS)

$(BUILD)/%.c.o: %.c
	@echo "CC -" $<
	@mkdir -p "$$(dirname "$@")"
	@$(BEAR) $(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJECTS)
	@echo "AR -" $@
	@$(AR) rcs $@ $<

$(EXAMPLE_BIN): $(LIB) $(EXAMPLE_SRC)
	@echo "LD -" $@
	@$(CC) $(EXAMPLE_SRC) $< -o $@ $(LFLAGS)

.PHONY: run
.PHONY: clean

run: all
	@./$(EXAMPLE_BIN)

clean:
	@$(RM) -rv $(EXAMPLE_BIN) $(LIB) $(BUILD)

