// `const` on an `out` or `inout` parameter makes the parameter read-only in the body, which
// contradicts its passing mode. HLSL accepts these declarations, so we warn rather than reject them.
// `const` on an `in` parameter, and `const __ref`, are not diagnosed.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

void constOut(const out int x);
//diag:       ^^^^^ 'const' conflicts with parameter passing mode 'out'
//diag:       ^^^^^ an 'out' parameter is meant to be written by its function, but 'const' makes 'x' read-only in the function body

int constInOut(const inout int x);
//diag:        ^^^^^ 'const' conflicts with parameter passing mode 'inout'
//diag:        ^^^^^ an 'inout' parameter is meant to be written by its function, but 'const' makes 'x' read-only in the function body

int constInSpaceOut(const in out int x);
//diag:             ^^^^^ 'const' conflicts with parameter passing mode 'inout'
//diag:             ^^^^^ an 'inout' parameter is meant to be written by its function, but 'const' makes 'x' read-only in the function body

int constIn(const int x);

int constRef(const __ref int x);
