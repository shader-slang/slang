#ifndef TEST_TARGET_UTIL_H
#define TEST_TARGET_UTIL_H

#include "core/slang-list.h"
#include "core/slang-string.h"
#include "slang.h"

namespace Slang
{

// A backend that a `COMPARE_COMPUTE` test line can name in its `targets=` option.
struct TestTarget
{
    // The name that the author writes in `targets=`, for example `glsl`.
    const char* label;

    // The arguments that select the backend in `render-test`. The list ends with a null pointer.
    const char* renderTestArgs[3];

    // True if `targets=all` includes this target.
    bool isInAll;
};

// Returns true if the test command accepts the `targets=` option.
//
// The commands that run `render-test` and compare a buffer accept it, because the buffer is the
// same on every target. Commands that check the text from `slangc` do not accept it yet, because
// that text is different on every target.
bool isTargetsOptionSupportedByCommand(const UnownedStringSlice& command);

// Appends the `render-test` arguments of `target` to `ioArgs`.
void appendTestTargetArgs(const TestTarget& target, List<String>& ioArgs);

// Returns the macro definition that `slang-test` passes to the Slang compiler (as
// `-Xslang <definition>`) for the test of the target with the label `label`. The name of the macro
// is `SLANG_TEST_` and the label in capital letters. For example, `metal` gives
// `-DSLANG_TEST_METAL` and `glsl` gives `-DSLANG_TEST_GLSL`. A shader uses the macro to pick the
// code for one target:
//
//     #if defined(SLANG_TEST_METAL)
//
// A label has only letters and digits, so the result is a valid macro name.
String getTestTargetDefinition(const UnownedStringSlice& label);

// Turns the value of a `targets=` option into a list of targets.
//
// The value is a list of tokens that a plus sign separates, for example `vk+cpu`. A comma does
// not work as a separator, because it separates the options of a test line. A token is one of
// these:
//
//   all         every target that has `isInAll` set;
//   <name>      the target with that label;
//   all-<name>  every target of `all` except the target with that label.
//
// An `all-<name>` token can remove more than one target: a hyphen between the names adds one. For
// example, `all-vk-cuda` is every target of `all` except `vk` and `cuda`, and `vk+cpu` is these two
// targets. There is no other way to remove a target: a token such as `-vk` is an unknown name.
//
// The result is the union of the tokens. `outTargets` has no duplicates and uses the order of the
// table, not the order of the tokens, so that the test names are stable.
//
// Returns true if `name` is `all` or the label of a target.
bool isTestTargetName(const UnownedStringSlice& name);

// Returns `SLANG_E_INVALID_ARG` and sets `outError` for an unknown name, an empty token, or an
// empty result. A name that is wrong must be an error, because a silent skip would run the test
// on fewer targets than the author wants.
SlangResult expandTestTargetList(
    const UnownedStringSlice& value,
    List<const TestTarget*>& outTargets,
    String& outError);

} // namespace Slang

#endif
