#ifndef TEST_TARGET_UTIL_H
#define TEST_TARGET_UTIL_H

#include "core/slang-list.h"
#include "core/slang-render-api-util.h"
#include "core/slang-string.h"
#include "slang.h"

namespace Slang
{

// A target that a `COMPARE_COMPUTE` test line can name in its `targets=` option. There is one bit
// for each target, so that a list of targets is a set of bits: `all-vk-cuda` is
// `AllOf & ~(Vk | Cuda)`.
//
// `Vk` and `Glsl` use the same render API (Vulkan). The Slang compiler takes a different path for
// each of them, so they are two targets, and `RenderApiFlag` cannot tell them apart.
//
// Use a struct wrapped Enum instead of enum class, as `RenderApiFlag` does, so that the values can
// be manipulated as integrals.
struct TestTargetFlag
{
    enum Enum
    {
        Vk = 1 << 0,
        Glsl = 1 << 1,
        Dx11 = 1 << 2,
        Dx12 = 1 << 3,
        Metal = 1 << 4,
        Cuda = 1 << 5,
        Cpu = 1 << 6,
        Wgpu = 1 << 7,
        Llvm = 1 << 8,
        AllOf = (1 << 9) - 1, ///< All bits set
    };
};
typedef uint32_t TestTargetFlags;

// The description of one target.
struct TestTarget
{
    // The bit of the target.
    TestTargetFlag::Enum flag;

    // The name that the author writes in `targets=`, for example `glsl`. It has no hyphen and no
    // comma, because `all-<name>` and the list of options use them.
    const char* label;

    // The render API that the target runs on.
    RenderApiType api;

    // The arguments that select the target in `render-test`. The list ends with a null pointer.
    const char* renderTestArgs[3];
};

// Returns the number of targets, and the target at `index`. The targets have the order of the bits.
Index getTestTargetCount();
const TestTarget& getTestTargetAt(Index index);

// Returns the target that has the bit `flag`. `flag` must have exactly one bit of `AllOf`.
const TestTarget& getTestTarget(TestTargetFlags flag);

// Returns true if the test command accepts the `targets=` option.
//
// The commands that run `render-test` and compare a buffer accept it, because the buffer is the
// same on every target. Commands that check the text from `slangc` do not accept it yet, because
// that text is different on every target.
bool isTargetsOptionSupportedByCommand(const UnownedStringSlice& command);

// Appends the `render-test` arguments of `target` to `ioArgs`.
void appendTestTargetArgs(const TestTarget& target, List<String>& ioArgs);

// Returns the macro definition that `slang-test` passes to the Slang compiler (as
// `-Xslang <definition>`) for the test of `target`. The name of the macro is `SLANG_TEST_` and the
// label in capital letters. For example, `metal` gives `-DSLANG_TEST_METAL` and `glsl` gives
// `-DSLANG_TEST_GLSL`. A shader uses the macro to pick the code for one target:
//
//     #if defined(SLANG_TEST_METAL)
//
// A label has only letters and digits, so the result is a valid macro name.
String getTestTargetDefinition(const TestTarget& target);

// Returns true if `name` is `all` or the label of a target.
bool isTestTargetName(const UnownedStringSlice& name);

// Turns the value of a `targets=` option into a set of targets.
//
// The value is a list of tokens that a plus sign separates, for example `vk+cpu`. A comma does
// not work as a separator, because it separates the options of a test line. A token is one of
// these:
//
//   all         every target;
//   <name>      the target with that label;
//   all-<name>  every target except the target with that label.
//
// An `all-<name>` token can remove more than one target: a hyphen between the names adds one. For
// example, `all-vk-cuda` is every target except `vk` and `cuda`, and `vk+cpu` is these two
// targets. There is no other way to remove a target: a token such as `-vk` is an unknown name.
//
// The result is the union (the OR) of the tokens.
//
// Returns `SLANG_E_INVALID_ARG` and sets `outError` for an unknown name, an empty token, or an
// empty result. A name that is wrong must be an error, because a silent skip would run the test
// on fewer targets than the author wants.
SlangResult parseTestTargets(
    const UnownedStringSlice& value,
    TestTargetFlags& outTargets,
    String& outError);

} // namespace Slang

#endif
