//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -std legacy -no-codegen
//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -std 2025 -no-codegen
//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -std 2026 -no-codegen
//TEST:SIMPLE(filecheck=CHECK,exit-code=-1): -std 202c -no-codegen

// These diagnostics record a current parser implementation detail, not a language guarantee about
// declaration order. Initializers and parameter defaults are parsed before later declarations are
// added to their scope. Function bodies are saved as `UnparsedStmt` and parsed later, as exercised
// by the positive fixture in `tests/language-feature/types/dyn-keyword-parse-timing.slang`.
// We should revisit these expectations if how or when syntax declarations are looked up changes,
// or if such names become reserved words.
namespace InitializerBeforeBinding
{
    // The later function is not yet visible, so the parser reads `dyn(input)` as a box type
    // expression. `input` denotes a value rather than a type, so checking the box operand fails.
    static int input = 1;
    static int value = dyn(input);

    int dyn(int argument)
    {
        return argument;
    }
}

namespace DefaultBeforeBinding
{
    // The parameter default is parsed immediately and has the same syntax-declaration lookup.
    static int input = 2;
    int value(int argument = dyn(input));

    int dyn(int argument)
    {
        return argument;
    }
}

// CHECK-NOT: internal error
// CHECK: error[E30060]
// CHECK-NOT: internal error
// CHECK: error[E30060]
// CHECK-NOT: internal error
