// test-context.h

#ifndef TEST_CONTEXT_H_INCLUDED
#define TEST_CONTEXT_H_INCLUDED

#include "../../source/core/slang-string-util.h"
#include "../../source/core/platform.h"
#include "../../source/core/slang-std-writers.h"
#include "../../source/core/dictionary.h"
#include "../../source/core/slang-test-tool-util.h"
#include "../../source/core/slang-render-api-util.h"

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

struct TestInfo
{
    TestInfo& addUsed(BackendType type)
    {
        if (type != BackendType::Unknown)
        {
            usedBackendFlags |= BackendFlags(1) << int(type);
        }
        return *this;
    }
    TestInfo& addUsed(Slang::RenderApiType type)
    {
        using namespace Slang;
        if (type != RenderApiType::Unknown)
        {
            usedRenderApiFlags |=RenderApiFlags(1) << int(type);
        }
        return *this;
    }
    TestInfo& addUsedBackends(BackendFlags flags)
    {
        usedBackendFlags |= flags;
        return *this;
    }
    TestInfo& addUsedRenderApis(Slang::RenderApiFlags flags)
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
    bool isCollectingTestInfo() const { return testInfo != nullptr; }
        /// If set, then tests are executed
    bool isExecuting() const { return testInfo == nullptr; }

        /// Ctor
    TestContext();
        /// Dtor
    ~TestContext();

    Options options;
    TestReporter* reporter = nullptr;
    TestCategorySet categorySet;

        /// If set then tests are not run, but their requirements are set 
    TestInfo* testInfo = nullptr;

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
