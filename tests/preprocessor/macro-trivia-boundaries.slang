//TEST:SIMPLE(filecheck=CHECK): -E

#define VALUE 7
#define ID(x) x
#define JOIN(a, b) a ## b
#define STRINGIFY(x) #x
#define OBJECT (VALUE)

// Whitespace before the opening parenthesis makes OBJECT object-like.
#if OBJECT != 7
#error The object-like macro was parsed as function-like.
#endif

// Comments separate stringified tokens, while nested expansion and pasting keep
// the macro recursion and argument rules independent of attached trivia.
JOIN(pre, fix)
STRINGIFY(foo/* separator */bar)
ID(ID(VALUE))
OBJECT

//CHECK: prefix
//CHECK: "foo bar"
//CHECK: 7
//CHECK: ({{[ \t]*}}7{{[ \t]*}})
