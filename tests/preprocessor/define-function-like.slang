//TEST(smoke):SIMPLE(diag=diag):
// support for function-like macros

#define FOO(x) 1.0 + x

float foo(float y) { return FOO(y) * 2.0; }

// simple token pasting

#define PASTE(a,b) a##b

PASTE(flo,at) bar() { return 0.0; }

// no space before parens? not a function-like macro

#define M (x) - (x)
//diag:    ^ undefined identifier
//diag:    ^ undefined identifier 'x'.
//diag:          ^ undefined identifier
//diag:          ^ undefined identifier 'x'.

// Error: undefined identifier `x`
float bar(float a) { return M(a); }
