#ifndef SLANG_TEST_TOOL_UTIL_H
#define SLANG_TEST_TOOL_UTIL_H

#include "slang-std-writers.h"

namespace Slang {

#ifdef SLANG_SHARED_LIBRARY_TOOL
#   define SLANG_TEST_TOOL_API SLANG_EXTERN_C SLANG_DLL_EXPORT 
#else
#   define SLANG_TEST_TOOL_API
#endif

/* Utility functions for 'test tools' */
struct TestToolUtil
{
    typedef SlangResult(*InnerMainFunc)(Slang::StdWriters* stdWriters, SlangSession* session, int argc, const char*const* argv);

        /// Given a slang result, returns a return code that can be returned from an executable
    static int getReturnCode(SlangResult res);
};

} // namespace Slang

#endif // SLANG_TEST_TOOL_H
