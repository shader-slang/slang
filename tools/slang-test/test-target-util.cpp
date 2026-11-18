#include "test-target-util.h"

#include "core/slang-char-util.h"

namespace Slang
{

// A test line that has no render API argument asks `slang-test` to make the tests for the targets
// of `all`. A test that cannot run on a target writes `targets=all-<name>`, for example
// `targets=all-vk-cuda`.
//
// `vk` and `glsl` use the Vulkan API, but the Slang compiler takes a different path for each of
// them: `vk` emits SPIR-V directly, and `glsl` emits GLSL and then compiles it to SPIR-V. They are
// two targets. Each one has its path in its arguments, so a run that changes the default path of
// the suite (`-emit-spirv-via-glsl`) does not make them the same test.
static const TestTarget kTestTargets[] = {
    {"vk", {"-vk", "-emit-spirv-directly", nullptr}, true},
    {"glsl", {"-vk", "-emit-spirv-via-glsl", nullptr}, true},
    {"dx11", {"-dx11", nullptr, nullptr}, true},
    {"dx12", {"-dx12", nullptr, nullptr}, true},
    {"metal", {"-metal", nullptr, nullptr}, true},
    {"cuda", {"-cuda", nullptr, nullptr}, true},
    {"cpu", {"-cpu", nullptr, nullptr}, true},
    {"wgpu", {"-wgpu", nullptr, nullptr}, true},
    {"llvm", {"-llvm", nullptr, nullptr}, true},
};

bool isTargetsOptionSupportedByCommand(const UnownedStringSlice& command)
{
    return command == "COMPARE_COMPUTE" || command == "COMPARE_COMPUTE_EX" ||
           command == "COMPARE_RENDER_COMPUTE";
}

void appendTestTargetArgs(const TestTarget& target, List<String>& ioArgs)
{
    for (const char* const* arg = target.renderTestArgs; *arg; ++arg)
        ioArgs.add(*arg);
}

String getTestTargetDefinition(const UnownedStringSlice& label)
{
    StringBuilder definition;
    definition << "-DSLANG_TEST_";
    for (const char c : label)
        definition.appendChar(CharUtil::toUpper(c));
    return definition.produceString();
}

static Index findTargetIndex(const UnownedStringSlice& label)
{
    for (Index i = 0; i < SLANG_COUNT_OF(kTestTargets); ++i)
    {
        if (label == kTestTargets[i].label)
            return i;
    }
    return -1;
}

bool isTestTargetName(const UnownedStringSlice& name)
{
    return name == "all" || findTargetIndex(name) >= 0;
}

static String getValidNames()
{
    StringBuilder names;
    names << "all";
    for (const auto& target : kTestTargets)
        names << ", " << target.label;
    return names.produceString();
}

static SlangResult reportUnknownTarget(
    const UnownedStringSlice& token,
    const UnownedStringSlice& name,
    String& outError)
{
    StringBuilder error;
    error << "unknown target '" << name << "'";
    if (token != name)
        error << " in '" << token << "'";
    error << ". Valid names are: " << getValidNames();
    outError = error.produceString();
    return SLANG_E_INVALID_ARG;
}

SlangResult expandTestTargetList(
    const UnownedStringSlice& value,
    List<const TestTarget*>& outTargets,
    String& outError)
{
    outTargets.clear();

    bool isSelected[SLANG_COUNT_OF(kTestTargets)] = {};

    auto selectAll = [&]()
    {
        for (Index i = 0; i < SLANG_COUNT_OF(kTestTargets); ++i)
        {
            if (kTestTargets[i].isInAll)
                isSelected[i] = true;
        }
    };

    // A split that drops the empty segments would hide an empty token, which is an error here.
    UnownedStringSlice rest = value;
    for (bool isLast = false; !isLast;)
    {
        const Index plus = rest.indexOf('+');
        isLast = (plus < 0);
        const UnownedStringSlice token = (isLast ? rest : rest.head(plus)).trim();
        if (!isLast)
            rest = rest.tail(plus + 1);

        if (token.getLength() == 0)
        {
            outError = "'targets=' has an empty name. Add a name, or remove the extra '+'.";
            return SLANG_E_INVALID_ARG;
        }

        if (token == "all")
        {
            selectAll();
        }
        else if (token.startsWith(UnownedStringSlice("all-")))
        {
            // `all-name` is every target of `all` without the named targets. A label has no
            // hyphen, so a hyphen between two names, as in `all-vk-cuda`, adds a name.
            bool isRemoved[SLANG_COUNT_OF(kTestTargets)] = {};

            UnownedStringSlice names = token.tail(4);
            for (bool isLastName = false; !isLastName;)
            {
                const Index hyphen = names.indexOf('-');
                isLastName = (hyphen < 0);
                const UnownedStringSlice name = isLastName ? names : names.head(hyphen);
                if (!isLastName)
                    names = names.tail(hyphen + 1);

                const Index index = findTargetIndex(name);
                if (index < 0)
                    return reportUnknownTarget(token, name, outError);
                isRemoved[index] = true;
            }

            for (Index i = 0; i < SLANG_COUNT_OF(kTestTargets); ++i)
            {
                if (kTestTargets[i].isInAll && !isRemoved[i])
                    isSelected[i] = true;
            }
        }
        else
        {
            const Index index = findTargetIndex(token);
            if (index < 0)
                return reportUnknownTarget(token, token, outError);
            isSelected[index] = true;
        }
    }

    for (Index i = 0; i < SLANG_COUNT_OF(kTestTargets); ++i)
    {
        if (isSelected[i])
            outTargets.add(&kTestTargets[i]);
    }

    if (outTargets.getCount() == 0)
    {
        outError = "'targets=' selects no target.";
        return SLANG_E_INVALID_ARG;
    }
    return SLANG_OK;
}

} // namespace Slang
