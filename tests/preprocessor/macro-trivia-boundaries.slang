//TEST:SIMPLE(filecheck=CHECK): -E

// We check spacing between preprocessing tokens before and after macro expansion.
// Macro classification, stringification, and directive expressions expose incorrect separators.

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

// Whitespace before the opening parenthesis makes `OBJECT` object-like.
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

// We expand before stringification so the quoted text exposes logical separators.
// Whitespace before replacement text or at an argument's ends must not split a name.
#define EXPAND_STRINGIFY(x) STRINGIFY(x)
#define COMPONENT part
#define FUNCTION_COMPONENT() part
EXPAND_STRINGIFY(left.COMPONENT.right)
EXPAND_STRINGIFY(left.FUNCTION_COMPONENT().right)
EXPAND_STRINGIFY(left.ID( part ).right)
//CHECK: "left.part.right"
//CHECK: "left.part.right"
//CHECK: "left.part.right"

// A substituted argument uses the separator written before the parameter.
#define SEPARATED_PARAM(x) left x.right
EXPAND_STRINGIFY(SEPARATED_PARAM(part))
//CHECK: "left part.right"

// Stringized results also inherit the separator before `#` in the replacement list.
#define STRINGIZED_PARAM(x) left #x
EXPAND_STRINGIFY(STRINGIZED_PARAM(part))
//CHECK: "left \"part\""

// Builtin expansion keeps the separators written around each invocation.
EXPAND_STRINGIFY(left.__LINE__.right)
EXPAND_STRINGIFY(left __LINE__.right)
//CHECK: "left.{{[0-9]+}}.right"
//CHECK: "left {{[0-9]+}}.right"

// Pasting joins its operands without adding a separator before the pasted token.
// Whitespace before the invocation still separates the result from preceding text.
EXPAND_STRINGIFY(left.JOIN(pre, fix).right)
EXPAND_STRINGIFY(left JOIN(pre, fix).right)
EXPAND_STRINGIFY(left.JOIN(, part).right)
EXPAND_STRINGIFY(left.JOIN(part, ).right)
#define PASTED_COMPONENT(a, b) left.a ## b.right
#define SPACED_PASTED_COMPONENT(a, b) left. a##b.right
EXPAND_STRINGIFY(PASTED_COMPONENT(, part))
EXPAND_STRINGIFY(SPACED_PASTED_COMPONENT(, part))
//CHECK: "left.prefix.right"
//CHECK: "left prefix.right"
//CHECK: "left.part.right"
//CHECK: "left.part.right"
//CHECK: "left.part.right"
//CHECK: "left. part.right"

// Chained pastes retain the left boundary even when both initial operands are empty.
#define CHAINED_COMPONENT(a, b, c) left.a ## b ## c.right
EXPAND_STRINGIFY(CHAINED_COMPONENT(, , part))
//CHECK: "left.part.right"

// When the first parameter expands to nothing, we use the invocation's spacing.
// An empty argument at a later parameter preserves spacing written at that position.
#define INITIAL_EMPTY_PARAM(x) x part
EXPAND_STRINGIFY(left.INITIAL_EMPTY_PARAM().right)
EXPAND_STRINGIFY(SEPARATED_PARAM())
//CHECK: "left.part.right"
//CHECK: "left .right"

// We keep surrounding separators when object-like or function-like macros expand to nothing.
// Adjacent punctuation stays adjacent when neither side supplies a separator.
#define EMPTY
#define EMPTY_FUNCTION()
EXPAND_STRINGIFY(left EMPTY.right)
EXPAND_STRINGIFY(left.EMPTY right)
EXPAND_STRINGIFY(left.EMPTY.right)
EXPAND_STRINGIFY(left EMPTY_FUNCTION().right)
EXPAND_STRINGIFY(left.EMPTY_FUNCTION().right)
//CHECK: "left .right"
//CHECK: "left. right"
//CHECK: "left..right"
//CHECK: "left .right"
//CHECK: "left..right"

// Whitespace from an expanded argument's trailing empty macro is discarded.
// When a trailing parameter expands to nothing, spacing before that parameter remains.
#define TRAILING_PARAM(x) left x
#define EMBEDDED_PARAM(x) left+x+right
EXPAND_STRINGIFY(ID(left EMPTY).right)
EXPAND_STRINGIFY(TRAILING_PARAM(EMPTY)+right)
EXPAND_STRINGIFY(EMBEDDED_PARAM(EMPTY EMPTY))
//CHECK: "left.right"
//CHECK: "left +right"
//CHECK: "left++right"

// Expression parsing treats line-start flags as directive boundaries.
// Pasted and substituted operators must belong to the expanded directive's line.
#define INFIX(x) 1 x 0
#if 1 JOIN(>, =) 0 != 1
#error The pasted comparison did not remain in the expression.
#endif
#if INFIX(
>=) != 1
#error The argument's physical newline split the expanded expression.
#endif
