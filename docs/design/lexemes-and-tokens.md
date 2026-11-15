# Lexemes and Tokens

Parsing needs the tokens in a program, but tools that inspect or reproduce the program also need its comments and whitespace.
Consider this input, which ends immediately after `// end`:

```text
first /* comment */

  second

// end
```

A parser reads the identifiers `first` and `second`, followed by EOF.
Reproducing the input also requires the space and comment after `first`, the blank lines, the indentation before `second`, and the final comment.
The lexer preserves that text as individual lexemes and attaches it to the tokens that the parser reads.

## Reading Tokens and Trivia

A `Lexeme` is one recognized occurrence of a raw token or trivia.
Raw tokens include identifiers, literals, punctuation, and EOF.
Trivia includes whitespace runs, line endings, and comments.
`Lexer::readLexeme()` reads either category; `RawToken` and `Trivia` are the corresponding base types.
A block comment is one lexeme even when it spans several lines.
The bytes consumed for a lexeme are its *physical spelling*.

A `Token` combines one raw token with its leading and trailing trivia.
`Lexer::readToken()` forms that combination without recognizing the following raw token.
Its trailing trivia ends immediately after the first `NewLine` lexeme, and any remaining trivia leads the next token.
A newline inside a block comment does not divide that comment or end trailing-trivia collection.

The example above therefore produces these tokens, where `\n` denotes one newline:

| Raw Token | Leading Trivia | Trailing Trivia |
| --- | --- | --- |
| `first` | Empty | ` /* comment */\n` |
| `second` | `\n  ` | `\n` |
| EOF | `\n// end` | Empty |

The first newline belongs to `first`.
The next blank line and indentation lead `second`.
The final blank line and comment lead EOF, which is a real token with no spelling bytes.
For an input containing only trivia, all of that trivia leads EOF.

Starting at the beginning of a `SourceView`, `readAllLexemes()` reads every atomic lexeme, including EOF.
`readAllTokens()` instead reads every token with its attached trivia, also including EOF.
Both forms preserve every byte in that view exactly once.
Concatenating the physical spellings of each token's leading trivia, raw token, and trailing trivia, including EOF's trivia, reproduces the input.
A `SourceFile` may already have decoded the file or removed its byte-order mark before the lexer reads the view.

Clients choose the form they need to inspect.
Fiddle parses tokens with attached trivia.
The documentation extractor searches comments by declaration position, so it reads atomic lexemes and explicitly selects the raw tokens and comments used by that search.

## Physical and Logical Spellings

Token recognition can join characters separated in the input by an escaped newline:

```text
val\
ue
```

The lexer recognizes one identifier, `value`.
Name lookup needs those joined characters, but reproducing the input requires the backslash and newline too.
Each `LexemeObj` therefore stores two spellings:

- `getPhysicalSpelling()` returns the bytes used to represent the lexeme, including escaped newlines.
- `getLogicalSpelling()` returns the spelling after those line splices have been removed.

For a lexed object, the physical spelling is the exact byte span consumed from its `SourceView`.
`Lexeme` exposes both spellings, and `Token` exposes them for its raw token, excluding attached trivia.
`Token::getContent()` is a compatibility alias for logical spelling.

Only escaped physical newlines have been removed from a lexeme's logical spelling.
A literal's quotes and escape sequences remain.
For example, the logical spelling of `"a\nb"` still includes the two quotes and the two characters `\` and `n`.
`getStringLiteralTokenValue()` decodes that spelling to produce a string containing `a`, a newline, and `b`.

An escaped newline can also occur between raw tokens:

```text
value \
+ other
```

The space has already ended the identifier, so the backslash and line ending form an `EscapedNewLine` trivia lexeme.
Its physical spelling contains those bytes, and its logical spelling is empty.
The line splice neither adds logical whitespace nor ends a preprocessor directive.
The space before the splice still separates `value` from `+`.

Logical lookahead and consumption use the same line-splice removal and UTF-8 decoding procedure.
Lookahead copies the bounded byte cursor, leaving the lexer's position and diagnostics unchanged.
Consumption advances that cursor and records any line splices needed to construct the current lexeme's logical spelling.
That record is temporary scanner state; the returned lexeme already contains both completed spellings.

When an operation synthesizes token text, that text need not correspond to an input span.
The single-spelling `TokenPool::createToken()` overloads use their supplied text or name for both forms.
The lower-level lexeme factory accepts physical and logical spellings separately, including when deserialization restores a recognized lexeme.

## Shared Objects and Located Occurrences

Two identical identifiers can occur at different locations or in different preprocessing contexts.
We share their immutable lexical data without merging those occurrences.
A `Lexeme` contains a `LexemeObj` pointer, a `SourceLoc`, and occurrence flags.
The object stores the concrete type tag, both spellings, and an optional interned `Name*`.
Within one `TokenPool`, occurrences with the same object data share that object.
The location and flags remain in each occurrence.

Physical spelling is part of object identity.
The spliced identifier above and an unspliced `value` therefore use different objects even though their logical spellings match.
A `TokenObj` additionally stores the raw-token object and ordered arrays of leading and trailing trivia objects.
Sharing a token object requires those same objects in the same order and on the same side of the raw token.
Every object referenced by a token object belongs to its pool.

The raw token's location and the physical spelling lengths determine the locations of attached trivia.
Leading trivia begins before the raw token by the combined physical length of its lexemes.
Trailing trivia begins immediately after the raw token's physical spelling.
Unknown raw-token locations yield unknown trivia locations.
These offsets describe the consecutive input bytes represented by a token and its trivia.

`AtStartOfLine` and `AfterWhitespace` are occurrence flags used by preprocessing.
For example, a function-like macro definition depends on whether whitespace separates the macro name from `(`.
Macro expansion can also change the first token's line context without changing its spelling.
Those facts belong to the occurrence rather than to a shared object.
Attached trivia exposes both spellings and its location with zero occurrence flags; the raw-token occurrence carries the preprocessing context.

Objects are immutable after creation.
Splitting `>>` or changing lexical spelling requires creating the resulting objects through the pool.
Other occurrences still refer to the original objects.
Changing only occurrence flags with `Token::withFlags()` produces another value referring to the same object.

## Choosing the Token Pool

The compiler chooses one token pool with the `ASTBuilder` that will own the resulting AST.
It passes that pool to preprocessing, and preprocessing passes it to every lexer it creates.
Input files, included files, pasted macro text, and generated macro tokens therefore allocate their lexical objects in that same pool.
Tokens synthesized later by the builder use that pool as well.
The parser borrows the completed input sequence; its builder already keeps the pool alive.

Deserialization uses the receiving builder's pool for all restored lexical objects.
The pool is selected with the loading state, so reading an individual token does not need to find or retain another owner.

A standalone client chooses its pool when it initializes a lexer.
The lexer borrows that pool during scanning, so the client keeps it alive with a `RefPtr<TokenPool>`.
Token and lexeme lists are constructed with that pool and retain it for the whole sequence.
Every value added to a list must belong to its chosen pool.
Pool factories likewise require any raw-token and trivia objects they combine to belong to that pool.
These requirements put allocation and lifetime decisions at the start of the operation, rather than at each token use.

A pool owns copied spelling text, immutable objects, trivia arrays, and references to names.
Keeping the builder or owning list alive keeps that lexical storage alive, even after the input buffer is released.
`TokenRef<T>` can retain the pool when a client needs one independently owned token.
Interpreting a `SourceLoc` additionally requires the corresponding `SourceManager` records; keeping lexical storage alive does not retain those records.
A pool is not safe for concurrent mutation.

## Type Tags and Checked Values

The hierarchy in [lexeme-defs.lua](../../source/compiler-core/lexeme-defs.lua) defines concrete lexeme types and abstract groups such as `NumericLiteral`.
Only concrete types have `LexemeTypeTag` values.
The generator first builds the hierarchy, then visits it depth-first so that every group's concrete descendants have consecutive tags.
For example, the tags for `IntegerLiteral` and `FloatingPointLiteral` are adjacent, and together make up `NumericLiteral`.
An abstract definition with no concrete descendants is rejected.

A `LexemeType` describes a half-open range of those tags.
A concrete type's range contains its one tag; an abstract type's range contains all tags below it in the hierarchy.
Each C++ wrapper exposes its descriptor as `T::type`.
The descriptors are also named in `LexemeTypes`, and carry descriptions used by diagnostics.

Membership uses one unsigned subtraction and one comparison.
Subtracting the range's first tag gives an offset; that offset must be smaller than the number of tags in the range.
A tag below the first tag wraps to a large unsigned offset and fails the same comparison.
The query does not walk parent types at runtime.
Descriptions for concrete tags come from a table indexed by the tag.

`lexeme.isInstanceOf(type)` tests a descriptor supplied at runtime.
`lexeme.isInstanceOf<T>()` uses `T::type` for the same check.
The concrete atomic wrapper for a comma is `RawComma`; its corresponding token-with-trivia wrapper is `CommaToken`.
Abstract wrappers, such as `RawNumericLiteral` and `NumericLiteralToken`, use the ranges for their groups.
The wrappers add no stored state or virtual dispatch.

`Token::is<T>()` tests a token category, and `Token::as<T>()` returns its checked typed value or an empty value.
Casting does not consume input.
A parser that wants to consume an optional comma can instead use the reader:

```cpp
if (auto comma = reader.advanceIf<CommaToken>())
{
    recordComma(comma.getLoc());
}
```

On a mismatch, `advanceIf<T>()` returns an empty value and leaves the reader's cursor unchanged.
An empty lexical value has a null object pointer.
EOF has an object, so a successful EOF read remains distinct from a failed optional read even though both have empty spelling.

A complete token or lexeme list stores EOF after its ordinary elements.
Its `end()` points at that stored sentinel, so ordinary iteration excludes EOF.
A client reconstructing the input also visits EOF to include its attached trivia.
A `TokenReader` borrows the sequence and repeats its EOF after reaching the end.
For a bounded sequence, such as a macro argument, the producer supplies an explicit EOF for the reader's view.

## Preprocessing Controls Recognition

The preprocessor sometimes needs to choose how the next raw token is recognized.
Consider an include directive without a space after its name:

```slang
#include<dir/file.h>
```

In normal mode, `<` begins a punctuation token.
After identifying the directive, the preprocessor selects `LexerMode::ExpectingAngleBracketHeaderName` before requesting its operand.
The lexer then recognizes `<dir/file.h>` as one angle-bracket header name, including any spaces or comment-like characters inside the brackets.
A quoted operand remains a string literal.
The mode applies only while reading the operand's first raw token.
If that token names a function-like macro, the preprocessor resets the mode before reading its arguments, where `<` can be an ordinary operator.
If macro expansion supplies the operand as ordinary tokens instead, the include handler combines their logical spellings into the filename.

Reading `include` must therefore leave the operand unrecognized.
Trailing-trivia collection checks whether the next characters begin trivia without asking the raw-token recognizer to classify them.
The preprocessor's lookahead also defers the next recognition until a token is requested.
Together, those rules let the directive handler choose the mode before the lexer recognizes `<`.

Diagnostic suppression needs the same control at the end of a directive:

```slang
#if 0
"unfinished
#endif
```

The preprocessor first needs to finish evaluating `0` and decide that the branch is disabled.
Only then can it suppress diagnostics for the unfinished string.
Trailing-trivia collection stops after the directive's `NewLine` lexeme.
`LexerInputStream` leaves that newline attached to the physical token and returns a separate zero-width `EndOfDirective` token before reading the next line.
The directive handler can change diagnostic policy at that point.

Macro playback can reuse tokens from the macro definition, including their physical trivia and locations.
Those locations still identify the original bytes in the definition's `SourceView`, even when playback repeats a token.
Builtin expansions such as `__LINE__` and stringification create bare tokens in the same chosen pool.
Builtins use the invocation location; stringification uses the location of `#` in the definition.
Token pasting re-lexes the joined text in a new `SourceView`, also using that pool.

The physical input sequence preserves directives, disabled branches, and the text used to write macro invocations.
The expanded sequence can omit that text and repeat tokens from macro definitions.
Reconstructing the original input therefore uses the physical sequence.

## Serializing Lexical State

AST serialization writes each token's concrete type tag, both spellings, name presence, ordered trivia, location, and occurrence flags.
Deserialization creates those lexical objects directly in its chosen pool.
For example, reading `<dir/file.h>` in normal lexer mode would produce several punctuation and identifier tokens instead of the original angle-bracket header name.
The serialized tag preserves that recognition decision, so loading a token does not require repeating context-sensitive lexing.

## Regenerating the Type Definitions

Fiddle generates the type tags, ranges, descriptions, and C++ wrappers from the Lua hierarchy.
Fiddle itself uses the lexer, so compiling the lexer cannot require running Fiddle first.
Normal builds use checked-in snapshots for the [type definitions](../../source/compiler-core/slang-lexeme-defs.h.fiddle), [atomic wrappers](../../source/compiler-core/slang-raw-lexeme-types.h.fiddle), and [token wrappers](../../source/compiler-core/slang-token-types.h.fiddle).

The `regenerate-lexeme-definitions` CMake target updates those snapshots after a hierarchy edit.
The `check-lexeme-definitions` target generates separate build-directory copies and compares them with the checked-in files.
Neither target is a prerequisite of `compiler-core`, which would create the bootstrap cycle.
The definitions and their regenerated snapshots are checked in together.
