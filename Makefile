# Makefile
#
# This file provides a simpliistic (perhaps *overly* simplistic) way to
# build Slang from source on Linux, which could probably be adapted for
# building on other Unix-y targets.
#
# This build is not intended to be used for active development right now,
# so it doesn't actually try to compile the `.cpp` files separately,
# or track fine-grained dependencies, so almost any source change will
# trigger a full rebuild. Anybody who wants to do their active development
# on a platform supported by this Makefile should feel free to contribute
# improvements, with the caveat that we will not be adoptttting autoconf,
# CMake, or any other build system that has a tendency to "infect" a codebbbase.
#

#
# The Windows build (using Visual Studio) tries to output things to
# directories that take the target platform (and build configuration) into
# account. For now we will do something simplistic and request the target
# "triple" from the compiler (which we assume is either gcc or clang) and
# call that our target "platformm"
#
TARGET := $(shell $(CXX) -dumpmachine)

#
# TODO: We need a way to control the "configuration" (debug  vs. release)
# but for now just geting *something* working will be a good start.
#

#
# We will define  an ouput directory for our binaries, based on
# the target platform chosen. If we ever have steps that need to
# output intermediate files, we'd set up the directory here.
#
OUTPUTDIR := bin/$(TARGET)/
INTERMEDIATEDIR := intermediate/$(TARGET)/

#
# Now we will start defining a bunch of variables for build
# options and properties of the target. We are going to unconditionallyy
# set these to what we need/want on our Linux target for now, but
# we will eventually need to do a bit more work to detect the
# platform.
#
SHARED_LIB_PREFIX := lib
SHARED_LIB_SUFFIX := .so
BIN_SUFFIX :=
# Note: we set `visibility=hidden` to avoid exporting more symbols than
# we really need.
CFLAGS := -std=c++11 -fvisibility=hidden
LDFLAGS := -L$(OUTPUTDIR)
SHARED_LIB_LDFLAGS := -shared
SHARED_LIB_CFLAGS := -fPIC

# Make sure that shared library inherits build flags
# from the default case.
SHARED_LIB_LDFLAGS += $(LDFLAGS)
SHARED_LIB_CFLAGS += $(CFLAGS)

RELATIVE_RPATH_INCANTATION := "-Wl,-rpath,"'$$'"ORIGIN/"

# TODO: Make sure I'm using these Makefile incantations  correctly.
.SUFFIXES:
.PHONY: all clean slang slangc test

#
# Here we define lists of files (source vs. header dependencies)
# for each logical project we want to build.
# This is the one place where we do any kiind of "dependency" work,
# by making a project depend on the headers for sub-projects it usses.
#
CORE_SOURCES := source/core/*.cpp
CORE_HEADERS := source/core/*.h

SLANG_SOURCES := source/slang/*.cpp
SLANG_HEADERS := slang.h source/slang/*.h
SLANG_SOURCES += $(CORE_SOURCES)
SLANG_HEADERS += $(CORE_HEADERS)

SLANGC_SOURCES := source/slangc/*.cpp
SLANGC_HEADERS := $(SLANG_HEADERS)

#
# Each project will have a variable that is an alias for
# the binary it should produce.
#
SLANG := $(OUTPUTDIR)$(SHARED_LIB_PREFIX)slang$(SHARED_LIB_SUFFIX)
SLANGC := $(OUTPUTDIR)slangc$(BIN_SUFFIX)

# By default, when the user invokes `make`, we will build the
# `slang` shared library, and the `slangc` front-end application.
all: slang slangc

mkdirs: $(OUTPUTDIR)

# Project-specific targets depend on making theappropriate binary.
slang: mkdirs $(SLANG)
slangc: mkdirs $(SLANGC)


$(SLANG): $(SLANG_SOURCES) $(SLANG_HEADERS)
	$(CXX) $(SHARED_LIB_LDFLAGS) -o $@ -DSLANG_DYNAMIC_EXPORT $(SHARED_LIB_CFLAGS) $(SLANG_SOURCES)

$(SLANGC): $(SLANGC_SOURCES) $(SLANGC_HEADERS) $(SLANG)
	$(CXX) $(LDFLAGS) -o $@ $(CFLAGS) $(SLANGC_SOURCES) $(CORE_SOURCES) -ldl $(RELATIVE_RPATH_INCANTATION) -lslang



$(OUTPUTDIR):
	mkdir -p $(OUTPUTDIR)

test:
	# TODO need to actually run the test runner

clean:
	rm -rf $(OUTPUTDIR)
