#include "slang-test/test-target-util.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Expands `value` and returns the labels of the targets, joined with a space.
static String expandLabels(const char* value)
{
    List<const TestTarget*> targets;
    String error;
    SLANG_CHECK(SLANG_SUCCEEDED(expandTestTargetList(UnownedStringSlice(value), targets, error)));

    StringBuilder labels;
    for (Index i = 0; i < targets.getCount(); ++i)
    {
        if (i != 0)
            labels << " ";
        labels << targets[i]->label;
    }
    return labels.produceString();
}

// Returns the error text for a `value` that must not expand.
static String expandError(const char* value)
{
    List<const TestTarget*> targets;
    String error;
    SLANG_CHECK(SLANG_FAILED(expandTestTargetList(UnownedStringSlice(value), targets, error)));
    SLANG_CHECK(targets.getCount() == 0);
    return error;
}

SLANG_UNIT_TEST(slangTestTargetListExpansion)
{
    // `all` leaves out the targets that are not in `all`.
    SLANG_CHECK(expandLabels("all") == "vk glsl dx11 dx12 metal cuda cpu wgpu llvm");

    // `all-<name>` removes one target, and a hyphen between names adds one.
    SLANG_CHECK(expandLabels("all-wgpu") == "vk glsl dx11 dx12 metal cuda cpu llvm");
    SLANG_CHECK(expandLabels("all-vk-cuda") == "glsl dx11 dx12 metal cpu wgpu llvm");
    SLANG_CHECK(expandLabels("all-vk-glsl") == "dx11 dx12 metal cuda cpu wgpu llvm");

    SLANG_CHECK(expandLabels("all-llvm") == "vk glsl dx11 dx12 metal cuda cpu wgpu");

    // A target that `all` already has changes nothing.
    SLANG_CHECK(expandLabels("all+glsl") == expandLabels("all"));

    // The result has the order of the table, has no duplicates, and ignores white space.
    SLANG_CHECK(expandLabels("cpu + vk+cpu") == "vk cpu");

    // The result is the union of the tokens, so the order of the tokens does not matter.
    SLANG_CHECK(expandLabels("cuda+all-cuda") == expandLabels("all"));
    SLANG_CHECK(expandLabels("all-cuda+cuda") == expandLabels("all"));
    SLANG_CHECK(expandLabels("all-cuda+all-vk") == expandLabels("all"));
}

SLANG_UNIT_TEST(slangTestTargetListErrors)
{
    // A wrong name is an error, and the text names it. A silent skip would run the test on fewer
    // targets than the author wants.
    SLANG_CHECK(expandError("vk+gls").indexOf("'gls'") >= 0);
    SLANG_CHECK(expandError("all-wgsl").indexOf("'wgsl'") >= 0);
    SLANG_CHECK(expandError("all-vk-nothing").indexOf("'nothing'") >= 0);

    // A name cannot be removed on its own. Only `all-<name>` removes targets. The text that does
    // not work is an unknown name, so a typo of the form never selects the wrong targets.
    SLANG_CHECK(expandError("-vk").indexOf("'-vk'") >= 0);
    SLANG_CHECK(expandError("all+-vk").indexOf("'-vk'") >= 0);
    SLANG_CHECK(expandError("vk+-cpu").indexOf("'-cpu'") >= 0);

    // The text lists the valid names.
    SLANG_CHECK(expandError("gls").indexOf(", glsl,") >= 0);

    // A comma does not separate the names, so the whole text is one unknown name.
    SLANG_CHECK(expandError("vk,cpu").indexOf("'vk,cpu'") >= 0);

    // An empty token, an empty list and an empty result are errors.
    SLANG_CHECK(expandError("").indexOf("empty") >= 0);
    SLANG_CHECK(expandError("vk+").indexOf("empty") >= 0);
    SLANG_CHECK(
        expandError("all-vk-glsl-dx11-dx12-metal-cuda-cpu-wgpu-llvm").indexOf("no target") >= 0);
}

SLANG_UNIT_TEST(slangTestTargetLabels)
{
    // A hyphen in a label would be ambiguous with `all-<name>`, and a comma would split the label.
    // Every label, including the ones that are not in `all`, has neither.
    List<const TestTarget*> targets;
    String error;
    SLANG_CHECK(
        SLANG_SUCCEEDED(expandTestTargetList(UnownedStringSlice("all+glsl"), targets, error)));
    SLANG_CHECK(targets.getCount() > 0);
    for (const TestTarget* target : targets)
    {
        const UnownedStringSlice label(target->label);
        SLANG_CHECK(label.indexOf('-') < 0);
        SLANG_CHECK(label.indexOf(',') < 0);
    }
}

SLANG_UNIT_TEST(slangTestTargetArgs)
{
    // `vk` and `glsl` name their path of the compiler, so they are two tests. `cpu` has one
    // argument.
    {
        List<const TestTarget*> vkTargets;
        String vkError;
        SLANG_CHECK(
            SLANG_SUCCEEDED(expandTestTargetList(UnownedStringSlice("vk"), vkTargets, vkError)));
        SLANG_CHECK_ABORT(vkTargets.getCount() == 1);
        List<String> vkArgs;
        appendTestTargetArgs(*vkTargets[0], vkArgs);
        SLANG_CHECK_ABORT(vkArgs.getCount() == 2);
        SLANG_CHECK(vkArgs[0] == "-vk");
        SLANG_CHECK(vkArgs[1] == "-emit-spirv-directly");
    }

    // `glsl` adds two arguments, and `cpu` adds one.
    List<const TestTarget*> targets;
    String error;
    SLANG_CHECK(
        SLANG_SUCCEEDED(expandTestTargetList(UnownedStringSlice("glsl+cpu"), targets, error)));
    SLANG_CHECK_ABORT(targets.getCount() == 2);

    List<String> args;
    appendTestTargetArgs(*targets[0], args);
    SLANG_CHECK_ABORT(args.getCount() == 2);
    SLANG_CHECK(args[0] == "-vk");
    SLANG_CHECK(args[1] == "-emit-spirv-via-glsl");

    args.clear();
    appendTestTargetArgs(*targets[1], args);
    SLANG_CHECK_ABORT(args.getCount() == 1);
    SLANG_CHECK(args[0] == "-cpu");
}

SLANG_UNIT_TEST(slangTestTargetDefinition)
{
    // The macro name is `SLANG_TEST_` and the label in capital letters.
    SLANG_CHECK(getTestTargetDefinition(UnownedStringSlice("metal")) == "-DSLANG_TEST_METAL");
    SLANG_CHECK(getTestTargetDefinition(UnownedStringSlice("dx12")) == "-DSLANG_TEST_DX12");
    SLANG_CHECK(getTestTargetDefinition(UnownedStringSlice("glsl")) == "-DSLANG_TEST_GLSL");

    // Every label of the table gives a valid macro name.
    List<const TestTarget*> targets;
    String error;
    SLANG_CHECK(
        SLANG_SUCCEEDED(expandTestTargetList(UnownedStringSlice("all+glsl"), targets, error)));
    for (const TestTarget* target : targets)
    {
        const String definition = getTestTargetDefinition(UnownedStringSlice(target->label));
        SLANG_CHECK(definition.startsWith("-DSLANG_TEST_"));
        for (const char c : definition.getUnownedSlice().tail(2))
            SLANG_CHECK(c == '_' || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'));
    }
}

SLANG_UNIT_TEST(slangTestTargetNames)
{
    SLANG_CHECK(isTestTargetName(UnownedStringSlice("all")));
    SLANG_CHECK(isTestTargetName(UnownedStringSlice("vk")));
    SLANG_CHECK(isTestTargetName(UnownedStringSlice("llvm")));
    SLANG_CHECK(!isTestTargetName(UnownedStringSlice("non-exhaustive")));
    SLANG_CHECK(!isTestTargetName(UnownedStringSlice("filecheck-buffer")));
}

SLANG_UNIT_TEST(slangTestTargetsSupportedCommands)
{
    SLANG_CHECK(isTargetsOptionSupportedByCommand(UnownedStringSlice("COMPARE_COMPUTE")));
    SLANG_CHECK(isTargetsOptionSupportedByCommand(UnownedStringSlice("COMPARE_COMPUTE_EX")));
    SLANG_CHECK(isTargetsOptionSupportedByCommand(UnownedStringSlice("COMPARE_RENDER_COMPUTE")));
    SLANG_CHECK(!isTargetsOptionSupportedByCommand(UnownedStringSlice("SIMPLE")));
    SLANG_CHECK(!isTargetsOptionSupportedByCommand(UnownedStringSlice("COMPILE")));
}
