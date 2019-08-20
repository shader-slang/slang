
#include "slang-test-tool-util.h"

#include "../../slang-com-helper.h"

#include "slang-io.h"
#include "slang-string-util.h"

namespace Slang
{

/* static */ToolReturnCode TestToolUtil::getReturnCode(SlangResult res)
{
    switch (res)
    {
        case SLANG_OK:              return ToolReturnCode::Success;
        case SLANG_E_INTERNAL_FAIL: return ToolReturnCode::CompilationFailed;
        case SLANG_FAIL:            return ToolReturnCode::Failed;
        case SLANG_E_NOT_AVAILABLE: return ToolReturnCode::Ignored;
        default:
        {
            return (SLANG_SUCCEEDED(res)) ? ToolReturnCode::Success : ToolReturnCode::Failed;
        }
    }
}

/* static */ToolReturnCode TestToolUtil::getReturnCodeFromInt(int code)
{
    if (code >= int(ToolReturnCodeSpan::First) && code <= int(ToolReturnCodeSpan::Last))
    {
        return ToolReturnCode(code);
    }
    else
    {
        SLANG_ASSERT(!"Invalid integral code");
        return ToolReturnCode::Failed;
    }
}

/* static */SlangResult TestToolUtil::setSessionDefaultPrelude(const char* exePath, slang::IGlobalSession* session)
{
    // Set the prelude to a path
    String canonicalPath;
    if (SLANG_SUCCEEDED(Path::getCanonical(exePath, canonicalPath)))
    {
        // Get the directory
        canonicalPath = Path::getParentDirectory(canonicalPath);

        String path = Path::combine(canonicalPath, "../../../prelude/slang-cpp-prelude.h");
        if (SLANG_SUCCEEDED(Path::getCanonical(path, canonicalPath)))
        {
            // Use forward slashes, to avoid escaping the path
            canonicalPath = StringUtil::calcCharReplaced(canonicalPath, '\\', '/');

            // It must exist!
            if (!File::exists(canonicalPath))
            {
                SLANG_ASSERT(!"Couldn't find the prelude relative to the executable");
                return SLANG_FAIL;
            }

            StringBuilder prelude;
            prelude << "#include \"" << canonicalPath << "\"\n\n";
            const SlangPassThrough downstreamCompilers[] = {
                SLANG_PASS_THROUGH_CLANG,                   ///< Clang C/C++ compiler 
                SLANG_PASS_THROUGH_VISUAL_STUDIO,           ///< Visual studio C/C++ compiler
                SLANG_PASS_THROUGH_GCC,                     ///< GCC C/C++ compiler
                SLANG_PASS_THROUGH_GENERIC_C_CPP,
            };
            for (auto downstreamCompiler : downstreamCompilers)
            {
                session->setDownstreamCompilerPrelude(downstreamCompiler, prelude.getBuffer());
            }
        }
    }

    return SLANG_OK;
}


}

