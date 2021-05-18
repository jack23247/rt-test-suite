# Makefile (https://stackoverflow.com/a/2484343)

CC = gcc
LD = gcc

BFLAGS = -O3 -mrdrnd # -O0 -ggdb #-O3
CFLAGS = -I. -std=gnu99 -Wall -Wextra -pedantic
WITH = -pthread -lrt -l:libcpuid.a # -lm

MODULES   := src
SRC_DIR   := $(addprefix ./,$(MODULES))
BUILD_DIR := $(addprefix .build/,$(MODULES))

EXE_NAME := rt-test-suite.x86_64

################################################################################

SRC       := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
OBJ       := $(patsubst ./%.c,.build/%.o,$(SRC))
INCLUDES  := $(addprefix -I,$(SRC_DIR))

vpath %.c $(SRC_DIR)

define make-goal
$1/%.o: %.c
	$(CC) $(BFLAGS) $(CFLAGS) $(INCLUDES) -c $$< -o $$@
endef

.PHONY: all checkdirs clean

all: checkdirs $(EXE_NAME)

$(EXE_NAME): $(OBJ)
	$(LD) $^ -o $@ $(WITH)

checkdirs: $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(BUILD_DIR) html $(EXE_NAME)

$(foreach bdir,$(BUILD_DIR),$(eval $(call make-goal,$(bdir))))