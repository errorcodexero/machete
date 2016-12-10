#
# Makefile for running style checks, unit tests and cross-compiling the
#   FRCUserProgram to be loaded on the robot.
#

OBJDIR := obj
CXX := arm-frc-linux-gnueabi-g++
ifeq ($(OS),Windows_NT)
WPILIB := $(USERPROFILE)/wpilib/cpp/current
else
WPILIB := /usr/local/wpilib/cpp/current
endif
CXXSRC := $(wildcard */*.cpp)
SRCDIRS := $(sort $(dir $(CXXSRC)))
space := $(eval) $(eval)
VPATH := $(subst $(space),:,$(SRCDIRS))
CXXOBJ := $(addprefix $(OBJDIR)/,$(patsubst %.cpp,%.o,$(CXXSRC)))
CXXFLAGS := -Wall -Wextra -Werror -O2 -g -std=c++14 -I$(WPILIB)/include -fmessage-length=0
LDFLAGS := -L$(WPILIB)/lib/

.PHONY: default
default: check_style test build

.PHONY: all
all: clean check_style test build

.PHONY: check_style check
check_style check:
	@echo "=== make check_style ==="
	./check_style

.PHONY: test
test:
	@echo "=== make test ==="
	bazel test --run_under='valgrind --error-exitcode=1' \
	  --cxxopt=-std=c++14 --cxxopt=-Wall --cxxopt=-Werror \
	  --test_verbose_timeout_warnings ...

.PHONY: build
build:
	@echo "=== make FRCUserProgram ==="
	@$(MAKE) FRCUserProgram

FRCUserProgram: $(CXXOBJ)
	$(CXX) -o $@ $(CXXOBJ) $(LDFLAGS) -lwpi

$(OBJDIR):
	mkdir -p $(sort $(dir $(CXXOBJ)))

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(COMPILE.cpp) $(OUTPUT_OPTION) $<

.PHONY: clean
clean:
	@echo "=== make clean ==="
	-bazel clean
	-$(RM) -rf FRCUserProgram $(OBJDIR) f1.tmp

# for Makefile debugging
.PHONY: verbose
verbose:
	@echo "=== make verbose ==="
	@echo ''
	@echo OBJDIR = \'$(OBJDIR)\'
	@echo ''
	@echo CXXSRC = \'$(CXXSRC)\'
	@echo ''
	@echo CXXOBJ = \'$(CXXOBJ)\'
	@echo ''
	@echo SRCDIRS = \'$(SRCDIRS)\'
	@echo ''
	@echo VPATH = \'$(VPATH)\'
	@echo ''
	
