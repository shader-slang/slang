// test-context.h

#ifndef TEST_CONTEXT_H_INCLUDED
#define TEST_CONTEXT_H_INCLUDED

#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-platform.h"
#include "../../source/core/slang-std-writers.h"
#include "../../source/core/slang-dictionary.h"
#include "../../source/core/slang-test-tool-util.h"
#include "../../source/core/slang-render-api-util.h"
#include "../../source/core/slang-cpp-compiler.h"

#include "options.h"

enum class BackendType
{
    Unknown = -1,
    Dxc,
    Fxc,
    Glslang,
    CountOf,
};

typedef uint32_t BackendFlags;
struct BackendFlag
{
    enum Enum : uint32_t
    {
        Dxc = 1 << int(BackendType::Dxc),
        Fxc = 1 << int(BackendType::Fxc),
        Glslang = 1 << int(BackendType::Glslang),
    };
};

/// Structure that describes requirements needs to run - such as rendering APIs or
/// back-end availability 
struct TestRequirements
{
    TestRequirements& addUsed(BackendType type)
    {
        if (type != BackendType::Unknown)
        {
            usedBackendFlags |= BackendFlags(1) << int(type);
        }
        return *this;
    }
    TestRequirements& addUsed(Slang::RenderApiType type)
    {
        using namespace Slang;
        if (type != RenderApiType::Unknown)
        {
            usedRenderApiFlags |=RenderApiFlags(1) << int(type);
        }
        return *this;
    }
    TestRequirements& addUsedBackends(BackendFlags flags)
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
    BackendFlags usedBackendFlags = 0;                                          ///< Used backends
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

        /// Get compiler factory
    Slang::CPPCompilerSystem* getCPPCompilerSystem();

        /// Ctor
    TestContext();
        /// Dtor
    ~TestContext();

    Options options;
    TestReporter* reporter = nullptr;
    TestCategorySet categorySet;

        /// If set then tests are not run, but their requirements are set 
    TestRequirements* testRequirements = nullptr;

    BackendFlags availableBackendFlags = 0;
    Slang::RenderApiFlags availableRenderApiFlags = 0;
    bool isAvailableRenderApiFlagsValid = false;

    Slang::RefPtr<Slang::CPPCompilerSystem> cppCompilerSystem;

protected:
    struct SharedLibraryTool
    {
        Slang::SharedLibrary::Handle m_sharedLibrary;
        InnerMainFunc m_func;
    };

    SlangSession* m_session;

    Slang::Dictionary<Slang::String, SharedLibraryTool> m_sharedLibTools;
};

#endif // TEST_CONTEXT_H_INCLUDED
