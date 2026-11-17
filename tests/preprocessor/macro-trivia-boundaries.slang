//TEST:SIMPLE(filecheck=CHECK): -E

// Comments separate preprocessing tokens, while escaped newlines add no whitespace.
// We check macro classification and stringification against that distinction.

#define VALUE 7
#define ID(x) x
#define JOIN(a, b) a ## b
#define STRINGIFY(x) #x
#define OBJECT (VALUE)
#define COMMENT_OBJECT/**/(VALUE)
#define MULTILINE_COMMENT_OBJECT/*
*/(VALUE)
#define SPLICED_FUNCTION\
(x) x

// Whitespace before the opening parenthesis makes OBJECT object-like.
#if OBJECT != 7
#error The object-like macro was parsed as function-like.
#endif

// Comments also separate the macro name from `(`, including a comment with a newline.
#if COMMENT_OBJECT != 7
#error A comment-separated object-like macro was parsed as function-like.
#endif
#if MULTILINE_COMMENT_OBJECT != 7
#error A multiline-comment-separated object-like macro was parsed as function-like.
#endif

// An escaped newline adds no whitespace between the macro name and `(`.
#if SPLICED_FUNCTION(9) != 9
#error The spliced function-like macro was parsed as object-like.
#endif

// Comments separate stringified tokens, while nested expansion and pasting keep
// the macro recursion and argument rules independent of attached trivia.
JOIN(pre, fix)
STRINGIFY(foo/* separator */bar)
ID(ID(VALUE))
OBJECT

// Stringification uses logical spelling and does not add spaces for escaped newlines.
STRINGIFY(na\
me)
STRINGIFY(foo\
+bar)

//CHECK: prefix
//CHECK: "foo bar"
//CHECK: 7
//CHECK: ({{[ \t]*}}7{{[ \t]*}})
//CHECK: "name"
//CHECK: "foo+bar"
