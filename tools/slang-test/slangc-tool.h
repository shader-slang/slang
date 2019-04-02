// slangc-tool.h

#ifndef SLANGC_TOOL_H_INCLUDED
#define SLANGC_TOOL_H_INCLUDED

#include "../../source/core/slang-std-writers.h"
#include "../../source/core/slang-test-tool-util.h"

/* The slangc 'tool' interface, such that slangc like functionality is available directly without invoking slangc command line tool, or
need for a dll/shared library. */
struct SlangCTestToolUtil
{
    static Slang::ITestTool* getTestTool();
};

#endif // SLANGC_TOOL_H_INCLUDED
