#include "test-target-util.h"

#include "core/slang-char-util.h"

namespace Slang
{

// `vk` and `glsl` use the Vulkan API, but the Slang compiler takes a different path for each of
// them: `vk` emits SPIR-V directly, and `glsl` emits GLSL and then compiles it to SPIR-V. They are
// two targets. Each one has its path in its arguments, so a run that changes the default path of
// the suite (`-emit-spirv-via-glsl`) does not make them the same test.
static const TestTarget kTestTargets[] = {
    {TestTargetFlag::Vk, "vk", RenderApiType::Vulkan, {"-vk", "-emit-spirv-directly", nullptr}},
    {TestTargetFlag::Glsl, "glsl", RenderApiType::Vulkan, {"-vk", "-emit-spirv-via-glsl", nullptr}},
    {TestTargetFlag::Dx11, "dx11", RenderApiType::D3D11, {"-dx11", nullptr, nullptr}},
    {TestTargetFlag::Dx12, "dx12", RenderApiType::D3D12, {"-dx12", nullptr, nullptr}},
    {TestTargetFlag::Metal, "metal", RenderApiType::Metal, {"-metal", nullptr, nullptr}},
    {TestTargetFlag::Cuda, "cuda", RenderApiType::CUDA, {"-cuda", nullptr, nullptr}},
    {TestTargetFlag::Cpu, "cpu", RenderApiType::CPU, {"-cpu", nullptr, nullptr}},
    {TestTargetFlag::Wgpu, "wgpu", RenderApiType::WebGPU, {"-wgpu", nullptr, nullptr}},
    {TestTargetFlag::Llvm, "llvm", RenderApiType::LLVM, {"-llvm", nullptr, nullptr}},
};

Index getTestTargetCount()
{
    return SLANG_COUNT_OF(kTestTargets);
}

const TestTarget& getTestTargetAt(Index index)
{
    SLANG_ASSERT(index >= 0 && index < getTestTargetCount());
    return kTestTargets[index];
}

const TestTarget& getTestTarget(TestTargetFlags flag)
{
    for (const auto& target : kTestTargets)
    {
        if (TestTargetFlags(target.flag) == flag)
            return target;
    }
    SLANG_ASSERT_FAILURE("The flag is not the bit of one target.");
    return kTestTargets[0];
}

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

String getTestTargetDefinition(const TestTarget& target)
{
    StringBuilder definition;
    definition << "-DSLANG_TEST_";
    for (const char* c = target.label; *c; ++c)
        definition.appendChar(CharUtil::toUpper(*c));
    return definition.produceString();
}

// Returns the bit of the target that has the label `label`, or 0 if there is none.
static TestTargetFlags findTargetFlag(const UnownedStringSlice& label)
{
    for (const auto& target : kTestTargets)
    {
        if (label == target.label)
            return target.flag;
    }
    return 0;
}

bool isTestTargetName(const UnownedStringSlice& name)
{
    return name == "all" || findTargetFlag(name) != 0;
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

SlangResult parseTestTargets(
    const UnownedStringSlice& value,
    TestTargetFlags& outTargets,
    String& outError)
{
    outTargets = 0;
    TestTargetFlags result = 0;

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
            result |= TestTargetFlag::AllOf;
        }
        else if (token.startsWith(UnownedStringSlice("all-")))
        {
            // `all-name` is every target without the named targets. A label has no hyphen, so a
            // hyphen between two names, as in `all-vk-cuda`, adds a name.
            TestTargetFlags removed = 0;

            UnownedStringSlice names = token.tail(4);
            for (bool isLastName = false; !isLastName;)
            {
                const Index hyphen = names.indexOf('-');
                isLastName = (hyphen < 0);
                const UnownedStringSlice name = isLastName ? names : names.head(hyphen);
                if (!isLastName)
                    names = names.tail(hyphen + 1);

                const TestTargetFlags flag = findTargetFlag(name);
                if (flag == 0)
                    return reportUnknownTarget(token, name, outError);
                removed |= flag;
            }

            result |= TestTargetFlag::AllOf & ~removed;
        }
        else
        {
            const TestTargetFlags flag = findTargetFlag(token);
            if (flag == 0)
                return reportUnknownTarget(token, token, outError);
            result |= flag;
        }
    }

    if (result == 0)
    {
        outError = "'targets=' selects no target.";
        return SLANG_E_INVALID_ARG;
    }
    outTargets = result;
    return SLANG_OK;
}

} // namespace Slang
