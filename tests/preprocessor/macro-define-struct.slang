//TEST:SIMPLE:
// Test for issue: https://github.com/shader-slang/slang/issues/9494
// It demonstrates that the macro invocation `Def` will start at a new line, otherwise the compilation
// will fail due to the 'public' keyword being unexpected after the enum declaration
enum TargetEnum
{
    Target_1,
}

#define Def \
public struct TestStruct \
{ \
    public static const int Value = 42; \
} \

Def
