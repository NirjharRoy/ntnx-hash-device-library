# Nirjhar Roy <babuiroy02@gmail.com>

# Makefile - This file contains the rules to build and install the driver,
# build the libntnxhashlib.a static library that contains the wrapper functions
# that make the ioctl calls to the ntnx device library and also contains the
# rules to build and run the tests.

ARCH             := x86
AR               := ar
BINARY           := ntnx_hash_mod
C_FLAGS          := -Wall -msse4.1 -msse4.2
DRIVER_OBJECTS   := ntnx_hash
GCC              := gcc
INSMOD           := insmod
KERNEL           := /lib/modules/$(shell uname -r)/build
KMOD_DIR         := $(shell pwd)
MAKE             := make
RMMOD            := rmmod
RM               := rm
LIB_FLAGS        := rcs
NTNX_LIB         := ntnxhashlib
NTNX_LIB_FULL    := lib$(NTNX_LIB).a
TEST_BINARY      := ntnx_hash_test

ccflags-y += $(C_FLAGS)
obj-m += $(BINARY).o


$(BINARY)-y := $(DRIVER_OBJECTS).o

all: $(BINARY).ko $(NTNX_LIB_FULL) $(TEST_BINARY)

clean:
	$(MAKE) -C $(KERNEL) M=$(KMOD_DIR) clean
	-$(RM) *.a *.o $(TEST_BINARY)
	-$(MAKE) uninstall

device: $(BINARY).ko

# Rules for the driver
$(BINARY).ko: $(DRIVER_OBJECTS).c $(DRIVER_OBJECTS).h
	$(MAKE) -C $(KERNEL) M=$(KMOD_DIR) modules

uninstall:
	-sudo $(RMMOD) $(BINARY).ko

install: $(BINARY).ko
	-$(MAKE) uninstall
	sudo $(INSMOD) $<
	sleep 1


# rules for the libntnxhashlib.a and ntnxhashlib.o files
$(NTNX_LIB).o: $(NTNX_LIB).c $(DRIVER_OBJECTS).h
	$(GCC) -c $(NTNX_LIB).c -o $@

$(NTNX_LIB_FULL): $(NTNX_LIB).o
	$(AR) $(LIB_FLAGS) $@ $<

ntnxlib: $(NTNX_LIB_FULL)

# Rules for the test
$(TEST_BINARY): $(NTNX_LIB_FULL) $(TEST_BINARY).c $(DRIVER_OBJECTS).h
	$(GCC) -pthread $@.c -L. -l$(NTNX_LIB) -o $@

test: $(TEST_BINARY)

run-tests: $(TEST_BINARY) $(BINARY).ko
	$(MAKE) install
	./$(TEST_BINARY)
