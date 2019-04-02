// test-context.cpp
#include "slangc-tool.h"

#include "../../slang-com-ptr.h"
#include "../../slang-com-helper.h"

#include "../../source/core/slang-test-tool-util.h"

using namespace Slang;

SLANG_API void spSetCommandLineCompilerMode(SlangCompileRequest* request);

static void _diagnosticCallback(char const* message, void* /*userData*/)
{
    auto stdError = GlobalWriters::getError();
    stdError.put(message);
    stdError.flush();
}

static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_Slang_ITestTool = SLANG_UUID_Slang_ITestTool;

class SlangCTestTool: public Slang::ITestTool
{
public:
    typedef SlangCTestTool ThisType;

    SLANG_IUNKNOWN_QUERY_INTERFACE
    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() SLANG_OVERRIDE { return 1; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() SLANG_OVERRIDE { return 1; }

    // ITestTool
    SLANG_NO_THROW SlangResult SLANG_MCALL run(StdWriters* stdWriters, SlangSession* session, int argc, const char*const* argv) SLANG_OVERRIDE;
    SLANG_NO_THROW SlangResult SLANG_MCALL calcTestRequirements(Slang::StdWriters* stdWriters, SlangSession* session, int argc, const char*const* argv, TestRequirements* ioRequirements ) SLANG_OVERRIDE;

    static ThisType* getSingleton() { static ThisType s_singleton; return &s_singleton; }

private:
    SlangCTestTool() {}

    ISlangUnknown* getInterface(const Guid& guid);
};

ISlangUnknown* SlangCTestTool::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_Slang_ITestTool) ? static_cast<ITestTool*>(this) : nullptr;
}

SlangResult SlangCTestTool::run(StdWriters* stdWriters, SlangSession* session, int argc, const char*const* argv)
{
    SlangCompileRequest* compileRequest = spCreateCompileRequest(session);
    spSetDiagnosticCallback(compileRequest, &_diagnosticCallback, nullptr);

    spSetCommandLineCompilerMode(compileRequest);
    // Do any app specific configuration
    stdWriters->setRequestWriters(compileRequest);

    {
        const SlangResult res = spProcessCommandLineArguments(compileRequest, &argv[1], argc - 1);
        if (SLANG_FAILED(res))
        {
            // TODO: print usage message
            return res;
        }
    }

    SlangResult res = SLANG_OK;

#ifndef _DEBUG
    try
#endif
    {
        // Run the compiler (this will produce any diagnostics through SLANG_WRITER_TARGET_TYPE_DIAGNOSTIC).
        res = spCompile(compileRequest);
        // If the compilation failed, then get out of here...
        // Turn into an internal Result -> such that return code can be used to vary result to match previous behavior
        res = SLANG_FAILED(res) ? SLANG_E_INTERNAL_FAIL : res;
    }
#ifndef _DEBUG
    catch (Exception & e)
    {
        GlobalWriters::getOut().print("internal compiler error: %S\n", e.Message.ToWString().begin());
        res = SLANG_FAIL;
    }
#endif

    // Now that we are done, clean up after ourselves
    spDestroyCompileRequest(compileRequest);
    return res;
}

SlangResult SlangCTestTool::calcTestRequirements(Slang::StdWriters* stdWriters, SlangSession* session, int argc, const char*const* argv, TestRequirements* ioRequirements )
{
    // Ideally we could parse using spProcessCommandLineArguments - but slang interface does not give access to all the parameters set
    // First check pass through
    {
        const char* passThrough;
        if (SLANG_SUCCEEDED(TestToolUtil::extractArg(argv, argc, "-pass-through", &passThrough)))
        {
            ioRequirements->addUsed(TestToolUtil::toBackendType(UnownedStringSlice(passThrough))); 
        }
    }
    // The target if set will also imply a backend
    {
        const char* targetName;
        if (SLANG_SUCCEEDED(TestToolUtil::extractArg(argv, argc, "-target", &targetName)))
        {
            const SlangCompileTarget target = TestToolUtil::toCompileTarget(UnownedStringSlice(targetName));
            ioRequirements->addUsedBackends(TestToolUtil::getBackendFlagsForTarget(target));
        }
    }
    return SLANG_OK;
}

/* static */Slang::ITestTool* SlangCTestToolUtil::getTestTool()
{
    return SlangCTestTool::getSingleton();
}
