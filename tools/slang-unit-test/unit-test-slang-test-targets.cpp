#include "slang-test/test-target-util.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Returns the labels of the targets in `targets`, joined with a space.
static String labelsOf(TestTargetFlags targets)
{
    StringBuilder labels;
    for (Index i = 0; i < getTestTargetCount(); ++i)
    {
        const TestTarget& target = getTestTargetAt(i);
        if (!(targets & target.flag))
            continue;
        if (labels.getLength() != 0)
            labels << " ";
        labels << target.label;
    }
    return labels.produceString();
}

// Parses `value` and returns the labels of the targets, joined with a space.
static String expandLabels(const char* value)
{
    TestTargetFlags targets = 0;
    String error;
    SLANG_CHECK(SLANG_SUCCEEDED(parseTestTargets(UnownedStringSlice(value), targets, error)));
    return labelsOf(targets);
}

// Returns the error text for a `value` that must not parse.
static String parseError(const char* value)
{
    TestTargetFlags targets = 0;
    String error;
    SLANG_CHECK(SLANG_FAILED(parseTestTargets(UnownedStringSlice(value), targets, error)));
    SLANG_CHECK(targets == 0);
    return error;
}

SLANG_UNIT_TEST(slangTestTargetListParsing)
{
    // `all` has every target.
    SLANG_CHECK(expandLabels("all") == "vk glsl dx11 dx12 metal cuda cpu wgpu llvm");

    // The bits of the targets are the bits of `all`, and a target is one bit.
    TestTargetFlags all = 0;
    for (Index i = 0; i < getTestTargetCount(); ++i)
    {
        const TestTarget& target = getTestTargetAt(i);
        SLANG_CHECK((target.flag & (target.flag - 1)) == 0);
        SLANG_CHECK((all & target.flag) == 0);
        all |= target.flag;
    }
    SLANG_CHECK(all == TestTargetFlag::AllOf);

    // `all-<name>` removes one target, and a hyphen between names adds one.
    SLANG_CHECK(expandLabels("all-wgpu") == "vk glsl dx11 dx12 metal cuda cpu llvm");
    SLANG_CHECK(expandLabels("all-vk-cuda") == "glsl dx11 dx12 metal cpu wgpu llvm");
    SLANG_CHECK(expandLabels("all-vk-glsl") == "dx11 dx12 metal cuda cpu wgpu llvm");
    SLANG_CHECK(expandLabels("all-llvm") == "vk glsl dx11 dx12 metal cuda cpu wgpu");

    // A target that `all` already has changes nothing.
    SLANG_CHECK(expandLabels("all+glsl") == expandLabels("all"));

    // The result has the order of the bits, has no duplicates, and ignores white space.
    SLANG_CHECK(expandLabels("cpu + vk+cpu") == "vk cpu");

    // The result is the union of the tokens, so the order of the tokens does not matter.
    SLANG_CHECK(expandLabels("cuda+all-cuda") == expandLabels("all"));
    SLANG_CHECK(expandLabels("all-cuda+cuda") == expandLabels("all"));
    SLANG_CHECK(expandLabels("all-cuda+all-vk") == expandLabels("all"));

    // `none` selects no target. It is a valid result, and it has no bit.
    TestTargetFlags none = TestTargetFlag::AllOf;
    String noneError;
    SLANG_CHECK(SLANG_SUCCEEDED(parseTestTargets(UnownedStringSlice("none"), none, noneError)));
    SLANG_CHECK(none == 0);
    SLANG_CHECK(SLANG_SUCCEEDED(parseTestTargets(UnownedStringSlice(" none "), none, noneError)));
    SLANG_CHECK(none == 0);

    // `vk` and `glsl` are two targets with one render API.
    TestTargetFlags vkAndGlsl = 0;
    String error;
    SLANG_CHECK(SLANG_SUCCEEDED(parseTestTargets(UnownedStringSlice("vk+glsl"), vkAndGlsl, error)));
    SLANG_CHECK(vkAndGlsl == (TestTargetFlag::Vk | TestTargetFlag::Glsl));
    SLANG_CHECK(getTestTarget(TestTargetFlag::Vk).api == RenderApiType::Vulkan);
    SLANG_CHECK(getTestTarget(TestTargetFlag::Glsl).api == RenderApiType::Vulkan);
    SLANG_CHECK(getTestTarget(TestTargetFlag::Cpu).api == RenderApiType::CPU);
}

SLANG_UNIT_TEST(slangTestTargetListErrors)
{
    // A wrong name is an error, and the text names it. A silent skip would run the test on fewer
    // targets than the author wants.
    SLANG_CHECK(parseError("vk+gls").indexOf("'gls'") >= 0);
    SLANG_CHECK(parseError("all-wgsl").indexOf("'wgsl'") >= 0);
    SLANG_CHECK(parseError("all-vk-nothing").indexOf("'nothing'") >= 0);

    // A name cannot be removed on its own. Only `all-<name>` removes targets. The text that does
    // not work is an unknown name, so a typo of the form never selects the wrong targets.
    SLANG_CHECK(parseError("-vk").indexOf("'-vk'") >= 0);
    SLANG_CHECK(parseError("all+-vk").indexOf("'-vk'") >= 0);
    SLANG_CHECK(parseError("vk+-cpu").indexOf("'-cpu'") >= 0);

    // `none` is the whole value. A list that has it and another name is an error, and so is a
    // list that removes every target without `none`.
    SLANG_CHECK(parseError("none+vk").indexOf("'targets=none'") >= 0);
    SLANG_CHECK(parseError("vk+none").indexOf("'targets=none'") >= 0);
    SLANG_CHECK(parseError("all-none").indexOf("'none'") >= 0);
    SLANG_CHECK(
        parseError("all-vk-glsl-dx11-dx12-metal-cuda-cpu-wgpu-llvm").indexOf("targets=none") >= 0);

    // The text lists the valid names.
    SLANG_CHECK(parseError("gls").indexOf(", glsl,") >= 0);

    // A comma does not separate the names, so the whole text is one unknown name.
    SLANG_CHECK(parseError("vk,cpu").indexOf("'vk,cpu'") >= 0);

    // An empty token, an empty list and an empty result are errors.
    SLANG_CHECK(parseError("").indexOf("empty") >= 0);
    SLANG_CHECK(parseError("vk+").indexOf("empty") >= 0);
    SLANG_CHECK(
        parseError("all-vk-glsl-dx11-dx12-metal-cuda-cpu-wgpu-llvm").indexOf("no target") >= 0);
}

SLANG_UNIT_TEST(slangTestTargetLabels)
{
    // A hyphen in a label would be ambiguous with `all-<name>`, and a comma would split the label.
    // Every label has neither.
    SLANG_CHECK(getTestTargetCount() > 0);
    for (Index i = 0; i < getTestTargetCount(); ++i)
    {
        const UnownedStringSlice label(getTestTargetAt(i).label);
        SLANG_CHECK(label.indexOf('-') < 0);
        SLANG_CHECK(label.indexOf(',') < 0);
    }
}

SLANG_UNIT_TEST(slangTestTargetArgs)
{
    // `vk` and `glsl` name their path of the compiler, so they are two tests. `cpu` has one
    // argument.
    List<String> args;
    appendTestTargetArgs(getTestTarget(TestTargetFlag::Vk), args);
    SLANG_CHECK_ABORT(args.getCount() == 2);
    SLANG_CHECK(args[0] == "-vk");
    SLANG_CHECK(args[1] == "-emit-spirv-directly");

    args.clear();
    appendTestTargetArgs(getTestTarget(TestTargetFlag::Glsl), args);
    SLANG_CHECK_ABORT(args.getCount() == 2);
    SLANG_CHECK(args[0] == "-vk");
    SLANG_CHECK(args[1] == "-emit-spirv-via-glsl");

    args.clear();
    appendTestTargetArgs(getTestTarget(TestTargetFlag::Cpu), args);
    SLANG_CHECK_ABORT(args.getCount() == 1);
    SLANG_CHECK(args[0] == "-cpu");
}

SLANG_UNIT_TEST(slangTestTargetDefinition)
{
    // The macro name is `SLANG_TEST_` and the label in capital letters.
    SLANG_CHECK(
        getTestTargetDefinition(getTestTarget(TestTargetFlag::Metal)) == "-DSLANG_TEST_METAL");
    SLANG_CHECK(
        getTestTargetDefinition(getTestTarget(TestTargetFlag::Dx12)) == "-DSLANG_TEST_DX12");
    SLANG_CHECK(
        getTestTargetDefinition(getTestTarget(TestTargetFlag::Glsl)) == "-DSLANG_TEST_GLSL");

    // Every target gives a valid macro name.
    for (Index i = 0; i < getTestTargetCount(); ++i)
    {
        const String definition = getTestTargetDefinition(getTestTargetAt(i));
        SLANG_CHECK(definition.startsWith("-DSLANG_TEST_"));
        for (const char c : definition.getUnownedSlice().tail(2))
            SLANG_CHECK(c == '_' || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'));
    }
}

SLANG_UNIT_TEST(slangTestTargetNames)
{
    SLANG_CHECK(isTestTargetName(UnownedStringSlice("all")));
    SLANG_CHECK(isTestTargetName(UnownedStringSlice("none")));
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
