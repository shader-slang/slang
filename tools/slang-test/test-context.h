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

class TestContext
{
    public:

        /// Get the slang session
    SlangSession* getSession() const { return m_session;  }

    SlangResult init();

        /// Get the inner main function (from shared library)
    Slang::ITestTool* getTestTool(const Slang::String& dirPath, const Slang::String& name);
        /// Set the function for the shared library
    void setTestTool(const Slang::String& name, Slang::ITestTool* tool);

        /// If true tests aren't being run just the information on testing is being accumulated
    bool isCollectingRequirements() const { return testRequirements != nullptr; }
        /// If set, then tests are executed
    bool isExecuting() const { return testRequirements == nullptr; }

        /// Ctor
    TestContext();
        /// Dtor
    ~TestContext();

    Options options;
    TestReporter* reporter = nullptr;
    TestCategorySet categorySet;

        /// If set then tests are not run, but their requirements are set 
    Slang::TestRequirements* testRequirements = nullptr;

    Slang::BackendFlags availableBackendFlags = 0;
    Slang::RenderApiFlags availableRenderApiFlags = 0;
    bool isAvailableRenderApiFlagsValid = false;

protected:
    struct SharedLibraryTool
    {
        Slang::SharedLibrary::Handle m_sharedLibrary;
        Slang::ComPtr<Slang::ITestTool> m_testTool;
    };

    SlangSession* m_session;

    Slang::Dictionary<Slang::String, SharedLibraryTool> m_sharedLibTools;
};

#endif // TEST_CONTEXT_H_INCLUDED
