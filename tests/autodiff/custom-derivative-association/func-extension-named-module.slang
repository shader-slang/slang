//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -experimental-feature

// A backward-derivative extension or custom forward pass in a module with a `module` declaration
// is rejected as a custom derivative that uses a non-exported import, whichever module declares
// the function it extends and however this module imports that one, and even when this module
// imports nothing. The error names `function` rather than the function. That is not intended
// behavior, but we record it here so that a change to it is deliberate. The same extensions are
// accepted in a module without a `module` declaration, as `func-extension-cross-module.slang`
// shows.

module "func-extension-named-module";

import "cross-module-primal";
__exported import "cross-module-exported-primal";

public struct ApplyExtendedContext
{
    public float a;
    public float x;
    public void operator()(__none_t da, out float dx, float dOut) { dx = dOut * 2.0 * a * x; }
};

// --- A function declared in this module ---

public float libraryBackwardExtended(no_diff float a, float x) { return a * x * x; }

__func_extension bwd_diff(libraryBackwardExtended)(no_diff float a, inout DifferentialPair<float> x, float dOut) -> void
//CHECK:         ^^^^^^^^ error E31162
//CHECK:         ^^^^^^^^ a publicly visible custom derivative cannot reference 'function' from a module that is imported but not re-exported with '__exported import'.
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

public float libraryApplyExtended(no_diff float a, float x) { return a * x * x; }

__func_extension __apply(libraryApplyExtended)(no_diff float a, float x) -> Tuple<float, ApplyExtendedContext>
//CHECK:         ^^^^^^^ error E31162
//CHECK:         ^^^^^^^ a publicly visible custom derivative cannot reference 'function' from a module that is imported but not re-exported with '__exported import'.
{
    return makeTuple(a * x * x, ApplyExtendedContext(a, x));
}

// --- A function from a module imported with a plain `import` ---

// A backward-derivative extension is also rejected for the import, as an extension in a module
// without a `module` declaration is in `func-extension-non-exported-import.slang`. A custom
// forward pass is not.

__func_extension bwd_diff(splitBackwardExtended)(no_diff float a, inout DifferentialPair<float> x, float dOut) -> void
//CHECK:         ^^^^^^^^ error E31162
//CHECK:         ^^^^^^^^ a publicly visible custom derivative cannot reference 'function' from a module that is imported but not re-exported with '__exported import'.
//CHECK:         ^^^^^^^^ error E31162
//CHECK:         ^^^^^^^^ a publicly visible custom derivative cannot reference 'splitBackwardExtended' from a module that is imported but not re-exported with '__exported import'.
//CHECK:         ^^^^^^^^ error E31162
//CHECK:         ^^^^^^^^ a publicly visible custom derivative cannot reference 'splitBackwardExtended' from a module that is imported but not re-exported with '__exported import'.
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

__func_extension __apply(splitApplyExtended)(no_diff float a, float x) -> Tuple<float, ApplyExtendedContext>
//CHECK:         ^^^^^^^ error E31162
//CHECK:         ^^^^^^^ a publicly visible custom derivative cannot reference 'function' from a module that is imported but not re-exported with '__exported import'.
{
    return makeTuple(a * x * x, ApplyExtendedContext(a, x));
}

// --- A function from a module imported with `__exported import` ---

__func_extension bwd_diff(exportedSplitBackwardExtended)(no_diff float a, inout DifferentialPair<float> x, float dOut) -> void
//CHECK:         ^^^^^^^^ error E31162
//CHECK:         ^^^^^^^^ a publicly visible custom derivative cannot reference 'function' from a module that is imported but not re-exported with '__exported import'.
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

__func_extension __apply(exportedSplitApplyExtended)(no_diff float a, float x) -> Tuple<float, ApplyExtendedContext>
//CHECK:         ^^^^^^^ error E31162
//CHECK:         ^^^^^^^ a publicly visible custom derivative cannot reference 'function' from a module that is imported but not re-exported with '__exported import'.
{
    return makeTuple(a * x * x, ApplyExtendedContext(a, x));
}

// The error for the import is also reported at the core-module declaration of the requirement
// that the derivative satisfies. This annotation comes last because an annotation without a
// position matches the first unmatched diagnostic with the same message.
//
//CHECK: custom derivative uses non-exported import
//CHECK: a publicly visible custom derivative cannot reference 'splitBackwardExtended' from a module that is imported but not re-exported with '__exported import'.
