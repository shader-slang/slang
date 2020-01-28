// options.h

#ifndef OPTIONS_H_INCLUDED
#define OPTIONS_H_INCLUDED

#include "../../source/core/slang-dictionary.h"

#include "test-reporter.h"
#include "../../source/core/slang-render-api-util.h"
#include "../../source/core/slang-smart-pointer.h"

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
    TestCategory* find(Slang::String const& name);
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

    // Directory to use when looking for binaries to run. If empty it's not set.
    Slang::String binDir;

    // only run test cases with names that have this prefix. 
    char const* testPrefix = nullptr;

    // generate extra output (notably: command lines we run)
    bool shouldBeVerbose = false;

    // Use verbose paths
    bool verbosePaths = false;

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
    Slang::RenderApiFlags enabledApis = Slang::RenderApiFlag::AllOf;

    // The subCommand to execute. Will be empty if there is no subCommand 
    Slang::String subCommand;      

    // Arguments to the sub command. Note that if there is a subCommand the first parameter is always the subCommand itself.
    Slang::List<Slang::String> subCommandArgs;

    // By default we potentially synthesize test for all 
    // TODO: Vulkan is disabled by default for now as the majority as vulkan synthesized tests
    // OpenGL is disabled for now
    // CPU is disabled by default
    // CUDA is disabled by default
    Slang::RenderApiFlags synthesizedTestApis = Slang::RenderApiFlag::AllOf & ~(Slang::RenderApiFlag::Vulkan | Slang::RenderApiFlag::OpenGl | Slang::RenderApiFlag::CPU); 

    // The adapter to use. If empty will match first found adapter.
    Slang::String adapter;

        /// Parse the args, report any errors into stdError, and write the results into optionsOut
    static SlangResult parse(int argc, char** argv, TestCategorySet* categorySet, Slang::WriterHelper stdError, Options* optionsOut);
};

#endif // OPTIONS_H_INCLUDED
