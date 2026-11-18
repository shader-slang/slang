//TEST:SIMPLE(filecheck=CHECK): -E

// Parsing arguments advances the underlying stream while the expansion stream
// still caches the macro name. Each comma must be consumed from that argument
// stream before parsing the next argument.
#define PAIR(a, b) a b
#define TRIPLE(a, b, c) a b c
#define WRAPPED(a, b) PAIR(a, b)
#define VARIADIC(...) __VA_ARGS__

PAIR(first, second)
TRIPLE(one, (two, inner), three)
WRAPPED(left, right)
PAIR(, last)
PAIR(initial, )
before VARIADIC() after

//CHECK: first second
//CHECK: one ({{ *}}two{{ *}},{{ *}}inner{{ *}}) three
//CHECK: left right
//CHECK: last
//CHECK: initial
//CHECK: before after
