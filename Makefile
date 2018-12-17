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
# improvements, with the caveat that we will not be adopting autoconf,
# CMake, or any other build system that has a tendency to "infect" a codebase.
#

PLATFORM := $(shell uname -s | tr '[:upper:]' '[:lower:]')
ARCHITECTURE := $(shell uname -p)

ifeq (,$(CONFIGURATION))
	CONFIGURATION := release
endif

ifeq (,$(SLANG_TEST_CATEGORY))
	SLANG_TEST_CATEGORY := full
endif

#
# The Windows build (using Visual Studio) tries to output things to
# directories that take the target platform (and build configuration) into
# account. For now we will do something simplistic and request the target
# "triple" from the compiler (which we assume is either gcc or clang) and
# call that our target "platformm"
#
TARGET := $(PLATFORM)-$(ARCHITECTURE)
#
# TODO: We need a way to control the "configuration" (debug  vs. release)
# but for now just geting *something* working will be a good start.
#

#
# We will define  an ouput directory for our binaries, based on
# the target platform chosen. If we ever have steps that need to
# output intermediate files, we'd set up the directory here.
#
OUTPUTDIR := bin/$(TARGET)/$(CONFIGURATION)/
INTERMEDIATEDIR := intermediate/$(TARGET)/$(CONFIGURATION)/

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
CFLAGS := -std=c++11 -fvisibility=hidden -fno-delete-null-pointer-checks
CFLAGS += -I.
LDFLAGS := -L$(OUTPUTDIR)
SHARED_LIB_LDFLAGS := -shared
SHARED_LIB_CFLAGS := -fPIC

ifeq (debug,$(CONFIGURATION))
CFLAGS += -g
else
CFLAGS += -O2
endif

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
#
SLANG_SOURCES += $(CORE_SOURCES)
SLANG_HEADERS += $(CORE_HEADERS)

SLANGC_SOURCES := source/slangc/*.cpp
SLANGC_HEADERS := $(SLANG_HEADERS)
#
SLANGC_SOURCES += $(CORE_SOURCES)

SLANG_GLSLANG_SOURCES := source/slang-glslang/*.cpp
SLANG_GLSLANG_HEADERS := source/slang-glslang/*.h

SLANG_REFLECTION_TEST_SOURCES := tools/slang-reflection-test/*.cpp
SLANG_REFLECTION_TEST_HEADERS :=

# Add `glslang` sources to the build or `slang-glslang`
#
# Note: We aren't going to wasttte time trying to work with
# the existing CMake-based build for `glslang`.
#
SLANG_GLSLANG_SOURCES += \
	external/glslang/OGLCompilersDLL/*.cpp \
	external/glslang/SPIRV/*.cpp \
	external/glslang/glslang/GenericCodeGen/*.cpp \
	external/glslang/glslang/MachineIndependent/*.cpp \
	external/glslang/glslang/MachineIndependent/preprocessor/*.cpp \
	external/glslang/glslang/OSDependent/Unix/*.cpp


SLANG_TEST_SOURCES := tools/slang-test/*.cpp
SLANG_TEST_HEADERS := tools/slang-test/*.h
#
SLANG_TEST_SOURCES += $(CORE_SOURCES)
SLANG_TEST_HEADERS += $(CORE_HEADERS)

#
# Each project will have a variable that is an alias for
# the binary it should produce.
#
SLANG := $(OUTPUTDIR)$(SHARED_LIB_PREFIX)slang$(SHARED_LIB_SUFFIX)
SLANGC := $(OUTPUTDIR)slangc$(BIN_SUFFIX)
SLANG_GLSLANG := $(OUTPUTDIR)$(SHARED_LIB_PREFIX)slang-glslang$(SHARED_LIB_SUFFIX)
SLANG_TEST := $(OUTPUTDIR)slang-test$(BIN_SUFFIX)
SLANG_REFLECTION_TEST := $(OUTPUTDIR)slang-reflection-test$(BIN_SUFFIX)

# By default, when the user invokes `make`, we will build the
# `slang` shared library, and the `slangc` front-end application.
all: slang slang-glslang slangc slang-test slang-reflection-test

mkdirs: $(OUTPUTDIR)

# Project-specific targets depend on making theappropriate binary.
slang: mkdirs $(SLANG)
slangc: mkdirs $(SLANGC)
slang-glslang: mkdirs $(SLANG_GLSLANG)
slang-test: mkdirs $(SLANG_TEST)
slang-reflection-test: mkdirs $(SLANG_REFLECTION_TEST)

$(SLANG): $(SLANG_SOURCES) $(SLANG_HEADERS)
	$(CXX) $(SHARED_LIB_LDFLAGS) -o $@ -DSLANG_DYNAMIC_EXPORT $(SHARED_LIB_CFLAGS) $(SLANG_SOURCES) -ldl $(RELATIVE_RPATH_INCANTATION)

$(SLANGC): $(SLANGC_SOURCES) $(SLANGC_HEADERS) $(SLANG)
	$(CXX) $(LDFLAGS) -o $@ $(CFLAGS) $(SLANGC_SOURCES) -ldl $(RELATIVE_RPATH_INCANTATION) -lslang

$(SLANG_GLSLANG): $(SLANG_GLSLANG_SOURCES) $(SLANG_GLSLANG_HEADERS)
	$(CXX) $(SHARED_LIB_LDFLAGS) -pthread -o $@ -Iexternal/glslang/ $(SHARED_LIB_CFLAGS) -DAMD_EXTENSIONS -DNV_EXTENSIONS $(SLANG_GLSLANG_SOURCES)

$(SLANG_TEST): $(SLANG_TEST_SOURCES) $(SLANG_TEST_HEADERS) $(SLANG)
	$(CXX) $(LDFLAGS) -o $@ $(CFLAGS) $(SLANG_TEST_SOURCES) -ldl $(RELATIVE_RPATH_INCANTATION) -lslang

$(SLANG_REFLECTION_TEST): $(SLANG_REFLECTION_TEST_SOURCES) $(SLANG)
	$(CXX) $(LDFLAGS) -o $@ $(CFLAGS) $(SLANG_REFLECTION_TEST_SOURCES) $(RELATIVE_RPATH_INCANTATION) -lslang

$(OUTPUTDIR):
	mkdir -p $(OUTPUTDIR)

test: $(SLANG_TEST) $(SLANG_REFLECTION_TEST)
	$(SLANG_TEST) -bindir $(OUTPUTDIR) -travis -category $(SLANG_TEST_CATEGORY) $(SLANG_TEST_FLAGS)

clean:
	rm -rf $(OUTPUTDIR)
