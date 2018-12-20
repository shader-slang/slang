// options.h

#ifndef OPTIONS_H_INCLUDED
#define OPTIONS_H_INCLUDED

#include "../../source/core/dictionary.h"

#include "test-reporter.h"
#include "render-api-util.h"
#include "../../source/core/smart-pointer.h"

// A category that a test can be tagged with
struct TestCategory: public Slang::RefObject
{
    // The name of the category, from the user perspective
    Slang::String name;

    // The logical "super-category" of this category
    TestCategory* parent;
};

struct TestCategorySet
{
public:
        /// Find a category with the specified name. Returns nullptr if not found
    TestCategory * find(Slang::String const& name);
        /// Adds a category with the specified name, and parent. Returns the category object.
        /// Parent can be nullptr
    TestCategory* add(Slang::String const& name, TestCategory* parent);
        /// Finds a category by name, else reports and writes an error  
    TestCategory* findOrError(Slang::String const& name);

    Slang::RefPtr<TestCategory> defaultCategory;    ///< The default category

protected:
    Slang::Dictionary<Slang::String, Slang::RefPtr<TestCategory> > m_categoryMap;
};

struct Options
{
    char const* appName = "slang-test";

    // Directory to use when looking for binaries to run
    char const* binDir = "";

    // only run test cases with names that have this prefix. 
    char const* testPrefix = nullptr;

    // generate extra output (notably: command lines we run)
    bool shouldBeVerbose = false;

    // force generation of baselines for HLSL tests
    bool generateHLSLBaselines = false;

    // Dump expected/actual output on failures, for debugging.
    // This is especially intended for use in continuous
    // integration builds.
    bool dumpOutputOnFailure = false;

    // If set, will force using of executables (not shared library) for tests
    bool useExes = false;

    // kind of output to generate
    TestOutputMode outputMode = TestOutputMode::Default;

    // Only run tests that match one of the given categories
    Slang::Dictionary<TestCategory*, TestCategory*> includeCategories;

    // Exclude test that match one these categories
    Slang::Dictionary<TestCategory*, TestCategory*> excludeCategories;

    // By default we can test against all apis
    RenderApiFlags enabledApis = RenderApiFlag::AllOf;

    // The subCommand to execute. Will be empty if there is no subCommand 
    Slang::String subCommand;      

    // Arguments to the sub command. Note that if there is a subCommand the first parameter is always the subCommand itself.
    Slang::List<Slang::String> subCommandArgs;

    // By default we potentially synthesize test for all 
    // TODO: Vulkan is disabled by default for now as the majority as vulkan synthesized tests fail  
    RenderApiFlags synthesizedTestApis = RenderApiFlag::AllOf & ~RenderApiFlag::Vulkan;

        /// Parse the args, report any errors into stdError, and write the results into optionsOut
    static SlangResult parse(int argc, char** argv, TestCategorySet* categorySet, Slang::WriterHelper stdError, Options* optionsOut);
};

#endif // OPTIONS_H_INCLUDED
