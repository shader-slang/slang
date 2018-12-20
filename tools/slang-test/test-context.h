// test-context.h

#ifndef TEST_CONTEXT_H_INCLUDED
#define TEST_CONTEXT_H_INCLUDED

#include "../../source/core/slang-string-util.h"
#include "../../source/core/platform.h"
#include "../../source/core/slang-std-writers.h"
#include "../../source/core/dictionary.h"
#include "../../source/core/slang-test-tool-util.h"

#include "options.h"

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

        /// Ctor
    TestContext();
        /// Dtor
    ~TestContext();

    Options options;
    TestReporter* reporter = nullptr;
    TestCategorySet categorySet;

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
