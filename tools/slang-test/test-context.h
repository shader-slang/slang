// test-context.h

#ifndef TEST_CONTEXT_H_INCLUDED
#define TEST_CONTEXT_H_INCLUDED

#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-platform.h"
#include "../../source/core/slang-std-writers.h"
#include "../../source/core/slang-dictionary.h"
#include "../../source/core/slang-test-tool-util.h"
#include "../../source/core/slang-render-api-util.h"
#include "../../source/core/slang-downstream-compiler.h"

#include "../../slang-com-ptr.h"

#include "options.h"

typedef uint32_t PassThroughFlags;
struct PassThroughFlag
{
    enum Enum : PassThroughFlags
    {
        Dxc = 1 << int(SLANG_PASS_THROUGH_DXC),
        Fxc = 1 << int(SLANG_PASS_THROUGH_FXC),
        Glslang = 1 << int(SLANG_PASS_THROUGH_GLSLANG),
        VisualStudio = 1 << int(SLANG_PASS_THROUGH_VISUAL_STUDIO),
        GCC = 1 << int(SLANG_PASS_THROUGH_GCC),
        Clang = 1 << int(SLANG_PASS_THROUGH_CLANG),
        Generic_C_CPP = 1 << int(SLANG_PASS_THROUGH_GENERIC_C_CPP),
        NVRTC = 1 << int(SLANG_PASS_THROUGH_NVRTC)
    };
};

/// Structure that describes requirements needs to run - such as rendering APIs or
/// back-end availability 
struct TestRequirements
{
    
    TestRequirements& addUsedRenderApi(Slang::RenderApiType type)
    {
        using namespace Slang;
        if (type != RenderApiType::Unknown)
        {
            usedRenderApiFlags |=RenderApiFlags(1) << int(type);
        }
        return *this;
    }
    TestRequirements& addUsedBackEnd(SlangPassThrough type)
    {
        if (type != SLANG_PASS_THROUGH_NONE)
        {
            usedBackendFlags |= PassThroughFlags(1) << int(type);
        }
        return *this;
    }
    TestRequirements& addUsedBackends(PassThroughFlags flags)
    {
        usedBackendFlags |= flags;
        return *this;
    }
    TestRequirements& addUsedRenderApis(Slang::RenderApiFlags flags)
    {
        usedRenderApiFlags |= flags;
        return *this;
    }
        /// True if has this render api as used
    bool isUsed(Slang::RenderApiType apiType) const
    {
        return (apiType != Slang::RenderApiType::Unknown) && ((usedRenderApiFlags & (Slang::RenderApiFlags(1) << int(apiType))) != 0);
    }

    Slang::RenderApiType explicitRenderApi = Slang::RenderApiType::Unknown;     ///< The render api explicitly specified 
    PassThroughFlags usedBackendFlags = 0;                                          ///< Used backends
    Slang::RenderApiFlags usedRenderApiFlags = 0;                               ///< Used render api flags (some might be implied)
};

class TestContext
{
    public:

    typedef Slang::TestToolUtil::InnerMainFunc InnerMainFunc;

        /// Get the slang session
    SlangSession* getSession() const { return m_session;  }

    SlangResult init();

        /// Get the inner main function (from shared library)
    InnerMainFunc getInnerMainFunc(const Slang::String& dirPath, const Slang::String& name);
        /// Set the function for the shared library
    void setInnerMainFunc(const Slang::String& name, InnerMainFunc func);

        /// If true tests aren't being run just the information on testing is being accumulated
    bool isCollectingRequirements() const { return testRequirements != nullptr; }
        /// If set, then tests are executed
    bool isExecuting() const { return testRequirements == nullptr; }

        /// True if a render API filter is enabled
    bool isRenderApiFilterEnabled() const { return options.enabledApis != Slang::RenderApiFlag::AllOf && options.enabledApis != 0; }

        /// True if a test with the requiredFlags can in principal run (it may not be possible if the API is not available though)
    bool canRunTestWithRenderApiFlags(Slang::RenderApiFlags requiredFlags);

        /// True if can run unit tests
    bool canRunUnitTests() const { return options.apiOnly == false; }

        /// Get compiler set
    Slang::DownstreamCompilerSet* getCompilerSet();
    Slang::DownstreamCompiler* getDefaultCompiler(SlangSourceLanguage sourceLanguage);

        /// Ctor
    TestContext();
        /// Dtor
    ~TestContext();

    Options options;
    TestReporter* reporter = nullptr;
    TestCategorySet categorySet;

        /// If set then tests are not run, but their requirements are set 
    TestRequirements* testRequirements = nullptr;

    PassThroughFlags availableBackendFlags = 0;
    Slang::RenderApiFlags availableRenderApiFlags = 0;
    bool isAvailableRenderApiFlagsValid = false;

    Slang::RefPtr<Slang::DownstreamCompilerSet> compilerSet;

protected:
    struct SharedLibraryTool
    {
        Slang::ComPtr<ISlangSharedLibrary> m_sharedLibrary;
        InnerMainFunc m_func;
    };

    SlangSession* m_session;

    Slang::Dictionary<Slang::String, SharedLibraryTool> m_sharedLibTools;
};

#endif // TEST_CONTEXT_H_INCLUDED
