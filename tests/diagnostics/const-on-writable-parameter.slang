// `const` on an `out` or `inout` parameter contradicts its passing mode, so it is an error even
// though DXC accepts it. `const` on an `in` parameter, and `const __ref`, are not diagnosed.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

void constOut(const out int x);
//diag:       ^^^^^ 'const' conflicts with parameter passing mode 'out'
//diag:       ^^^^^ an 'out' parameter is meant to be written by its function, so 'x' cannot be 'const'

int constInOut(const inout int x);
//diag:        ^^^^^ 'const' conflicts with parameter passing mode 'inout'
//diag:        ^^^^^ an 'inout' parameter is meant to be written by its function, so 'x' cannot be 'const'

int constInSpaceOut(const in out int x);
//diag:             ^^^^^ 'const' conflicts with parameter passing mode 'inout'
//diag:             ^^^^^ an 'inout' parameter is meant to be written by its function, so 'x' cannot be 'const'

int constIn(const int x);

int constRef(const __ref int x);
