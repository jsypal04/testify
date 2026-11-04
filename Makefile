CC = gcc
CFLAGS = -Wall -Wextra -g

MAJOR = 0
MINOR = 1
PATCH = 0

BUILD_DIR = build

LIB_NAME      = libcalenter.so
LIB_REAL_NAME = $(LIB_NAME).$(MAJOR).$(MINOR).$(PATCH)
SONAME        = $(LIB_NAME).$(MAJOR)

LIB = $(BUILD_DIR)/$(LIB_REAL_NAME)

all: $(LIB)

clean:
	rm -rf $(BUILD_DIR)/*

$(LIB): $(BUILD_DIR)/testify.o
	$(CC) $(CFLAGS) -shared -fPIC $< -o $@
	ln -sf $(LIB_REAL_NAME) $(BUILD_DIR)/$(SONAME)
	ln -sf $(SONAME) $(BUILD_DIR)/$(LIB_NAME)

$(BUILD_DIR)/testify.o: testify.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: all, clean
