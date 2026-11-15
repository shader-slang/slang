# Lexemes and Tokens

The lexer must recognize the tokens used by preprocessing and parsing while preserving the text those consumers normally skip.
Consider this input, which ends immediately after `// end`:

```text
first /* comment */

  second

// end
```

A parser needs the identifiers `first` and `second` and an indication that the input has ended.
Reproducing the input also requires the comment after `first`, the blank lines, the indentation before `second`, and the final comment.
We represent every part of the input as an atomic *lexeme*, then group whitespace and comments around the tokens that preprocessing and parsing read.
The grouping lets those consumers read tokens without removing the text between them.

This chapter follows the input through that grouping, explains why spelling and location are separate, and then describes how consumers read and retain the resulting values.
The implementation lives in [slang-lexer.h](../../source/compiler-core/slang-lexer.h) and [slang-token.h](../../source/compiler-core/slang-token.h).

## From Input Text to Tokens

A `Lexeme` represents one recognized occurrence: an identifier, literal, punctuation mark, whitespace run, newline, or comment.
`Lexer::readLexeme()` reads one such occurrence, including whitespace and comments.
We call whitespace, line endings, and comments *trivia* because preprocessing and parsing generally read the surrounding tokens instead.
`RawToken` is the category of atomic lexemes that are tokens rather than trivia.

For the first line of the example, atomic reading produces an identifier, a space, a block comment, and a newline.
Each lexeme has its own kind and *physical spelling*: the exact input bytes the lexer consumed for that lexeme.
A block comment remains one lexeme even when the comment contains several lines.

`Lexer::readToken()` reads a raw token together with ordered leading and trailing trivia.
We call that combination a *token cluster*; the value representing it is a `Token`.
Leading trivia precedes the raw token, and trailing trivia follows it.
For tokens read directly by the lexer, the three parts occupy consecutive bytes of the input.

Trailing trivia ends immediately after the first explicit `NewLine` lexeme.
Any remaining trivia leads the next raw token.
The example is therefore grouped as follows; `\n` denotes one newline in this table:

| Raw Token | Leading Trivia | Trailing Trivia |
| --- | --- | --- |
| `first` | Empty | ` /* comment */\n` |
| `second` | `\n  ` | `\n` |
| EOF | `\n// end` | Empty |

The first newline belongs to `first`, while the next blank line and indentation belong to `second`.
We use the first explicit newline as the division so that each token's trailing trivia includes the end of its line, and the next token's leading trivia includes the intervening blank lines.
A newline inside a block comment does not divide that comment or end trailing-trivia collection.

EOF is a real, zero-width raw token at the end of the input.
Its leading trivia contains any text remaining after the preceding token's trailing trivia.
If the input contains only trivia, all of that trivia leads EOF.
EOF therefore participates in the same grouping rule as every other token.

Every input byte belongs to exactly one atomic lexeme and exactly one token cluster.
Concatenating the physical spellings of each cluster's leading trivia, raw token, and trailing trivia, including the EOF cluster, reproduces the input.
The input here is the text in a `SourceView`; `SourceFile` may already have decoded the file or removed a byte-order mark before lexing begins.

`Lexer::readAllLexemes()` reads the complete atomic sequence, including EOF.
`Lexer::readAllTokens()` reads the complete grouped sequence, also including EOF.
Consumers choose between those representations according to what they need to inspect.
For example, the documentation extractor searches comments by declaration position, so it reads atomic lexemes and explicitly selects the kinds used by that search.
Fiddle parses the grouped token sequence.

## Why a Lexeme Has Two Spellings

Token recognition can join characters that are separated in the input by an escaped newline.
Consider this identifier:

```text
val\
ue
```

The lexer recognizes one identifier spelled `value` after line splicing.
Identifier lookup needs that joined text, but reconstruction needs the backslash and newline as well.
Each `LexemeObj` therefore stores two spellings.
For lexed objects, `getPhysicalSpelling()` returns the exact consumed input bytes, including backslash-newline sequences.
`getLogicalSpelling()` returns the *logical spelling*, with line splices removed.
For this identifier, the physical spelling spans both lines and the logical spelling is `value`.

`Lexeme` exposes both accessors, and `Token` exposes them for its raw token, excluding attached trivia.
`Token::getContent()` is a compatibility alias for `getLogicalSpelling()`.

The lexer recognizes tokens through logical code-point reads while recording the consumed byte span for physical spelling.
Logical lookahead and consumption use the same line-splice removal and UTF-8 decoding procedure.
Lookahead copies the bounded byte cursor and leaves the lexer's position and diagnostics unchanged.
Recognition therefore tests the same characters that a later read consumes, without losing the input bytes skipped by line splicing.

Logical spelling removes escaped newlines; literal decoding is a separate operation.
For example, the logical spelling of `"a\nb"` still includes quotes and the two characters `\` and `n`.
`getStringLiteralTokenValue()` decodes that escape when a consumer needs the literal's value.

An escaped newline can also occur between raw tokens:

```text
value \
+ other
```

The space has already ended the identifier, so the backslash/newline is a separate `EscapedNewLine` trivia lexeme.
Its physical spelling preserves the backslash and line ending, and its logical spelling is empty.
The escape itself neither introduces logical whitespace nor ends a preprocessor directive.
The space before the escape still separates `value` from `+`.

When a compiler operation synthesizes text, no original input span determines the two spellings.
The convenience `TokenPool::createToken()` overloads use the supplied text or name for both forms.
The lower-level lexeme factory accepts the two spellings separately for callers preserving an already recognized value, such as deserialization.

## Shared Objects and Individual Occurrences

Consider two occurrences of `value` in one file:

```slang
int value;
value = 1;
```

Both identifiers have the same kind and spelling, but their locations differ.
Their preprocessing context can differ as well: the second identifier starts a line, while the first follows another token.
We keep those two kinds of information separate so that sharing spelling never merges occurrences.

A `Lexeme` contains a pointer to an immutable `LexemeObj`, a `SourceLoc`, and occurrence flags.
The object contains the concrete kind, both spellings, and an optional interned `Name*`.
`TokenPool` owns and interns those objects.
Within one pool, lexemes with the same concrete kind, physical spelling, logical spelling, and optional `Name*` share an object, regardless of location or occurrence flags.
Different physical spellings remain distinct even when their logical spellings are equal, as with `value` and the escaped-newline identifier above.

A `Token` follows the same division.
Its immutable `TokenObj` refers to a raw-token object and ordered arrays of leading and trailing trivia objects.
Its occurrence carries the raw token's location and flags.
Sharing a `TokenObj` requires the same raw object and the same ordered trivia arrays, including the division between leading and trailing trivia.
Every referenced object belongs to the same pool.

`TokenFlag::AtStartOfLine` and `TokenFlag::AfterWhitespace` describe the occurrence used by preprocessing.
For example, recognizing a function-like macro definition depends on whether whitespace separates its name from `(`.
Those flags belong to the occurrence rather than to an interned spelling.
Attached trivia exposes its kind, spelling, and location with zero occurrence flags; the containing raw token carries the preprocessing context.

The raw token's location and the physical spelling lengths determine the locations of attached trivia.
Leading trivia begins before the raw token by the total byte length of its leading trivia's physical spellings.
Trailing trivia begins immediately after the raw token's physical spelling.
An unknown raw-token location produces unknown trivia locations rather than invented offsets.
These calculations require a physically contiguous cluster; they do not establish the origin of text assembled by preprocessing.

The shared objects are immutable.
A client changing a token's spelling or kind constructs a replacement through its `TokenPool`, leaving other occurrences unchanged.
The replacement's producer is responsible for supplying spellings, trivia, and occurrence information that satisfy the new token's contract.
Changing only occurrence flags with `Token::withFlags()` returns a value referring to the same immutable object.

## Reading a Checked Token Category

A parser often needs to consume a token only when its kind matches a grammar rule.
For example, after reading an argument, a parser can consume an optional comma and keep its location:

```cpp
if (auto comma = reader.advanceIf<CommaToken>())
{
    recordComma(comma.getLoc());
}
```

`TokenReader::advanceIf<T>()` checks the next token's kind.
On a match, it advances and returns the checked typed value.
On a mismatch, it returns an empty value and leaves the cursor unchanged.
Required parser reads handle diagnostics and recovery separately; a mismatched token does not become a successfully checked value.

`CommaToken` is a wrapper over a `Token` occurrence.
The corresponding atomic wrapper, `RawComma`, represents a comma without attached trivia.
Wrappers also describe abstract groups, such as `LiteralToken` and `RawNumericLiteral`.
The generated hierarchy supplies kind tests and checked conversions without adding object state or virtual dispatch.
Only concrete kinds can be allocated as lexical objects.
The same checked categories apply to literal-value helpers: `getStringLiteralTokenValue()` accepts a `RawStringLiteral`, so the caller establishes the literal kind before requesting decoding.

An empty lexical value has a null object pointer and means that no value was returned.
EOF has an object, a location, and possibly leading trivia, despite having no spelling bytes.
A successful EOF read therefore remains distinguishable from a failed optional read.

A complete `TokenList` or `LexemeList` stores its actual EOF after the ordinary elements.
Its `end()` points to that stored sentinel, so ordinary iteration excludes EOF.
A consumer reconstructing the complete input also visits the sentinel to include its trivia.
`TokenReader` borrows its token sequence and EOF; it does not allocate storage, fabricate a token, or change the input token's kind.
When a producer exposes a bounded token sequence, such as a macro argument, that producer also supplies the EOF that terminates the reader's view.

## When Preprocessing Controls Recognition

Grouping trivia must leave the following raw token unread because preprocessing can change how that token is recognized.
Consider an include directive with no space after its name:

```slang
#include<dir/file.h>
```

In `LexerMode::Normal`, `<` begins an ordinary punctuation token.
After identifying the directive, the preprocessor selects `LexerMode::ExpectingAngleBracketHeaderName` before requesting the operand.
The lexer then recognizes `<dir/file.h>` as one `AngleBracketHeaderName` raw token.
Spaces and comment-like characters inside the angle brackets are part of that spelling.
A quoted operand remains a `StringLiteral`.

To preserve that ordering, `Lexer::readToken()` checks whether characters begin trivia without recognizing the next raw token.
Reading the `include` identifier may collect its trivia, but leaves the adjacent `<` for the later operand read.
The preprocessor's input-stream lookahead also defers recognition until a token is requested.
An include operand produced by macro expansion can instead consist of ordinary expanded tokens, which the include handler combines into the filename.

The same timing requirement applies at a directive's final newline:

```slang
#if 0
"unfinished
#endif
```

After the preprocessor reads `0`, it must finish evaluating the directive and suppress diagnostics in the disabled branch before the lexer recognizes the unfinished string.
Trailing-trivia collection therefore stops immediately after the directive's explicit newline.
`LexerInputStream` leaves that newline in its physical token cluster and exposes a separate zero-width `EndOfDirective` token to directive handlers.
Reading that delimiter does not recognize the next line.
The handler can change diagnostic policy before the next token is requested.

Preprocessing also changes which token occurrences reach the parser.
Consider this macro definition and invocation:

```slang
#define VALUE 7
int result = VALUE;
```

Macro playback reuses the token for `7` from the definition, including its location and attached physical trivia.
Those positions still identify the original cluster in its originating `SourceView`.
Repeated playback can therefore refer to the same physical cluster several times.

Generated spellings follow the operation that creates them.
Builtin expansions such as `__LINE__` and macro stringification create bare tokens through `MacroInvocation::_pushSingleTokenStream()`.
Builtin expansions supply the invocation location; stringification supplies the location of `#` in the macro definition.
Token pasting instead re-lexes the joined text in a new `SourceView`.
`SourceManager` and `SourceView` provide the location records used by those operations; interned lexical objects contain no invocation locations.
The representation of expansion origins is separate from the positions derived for a contiguous physical cluster.

The physical lexeme or token sequence represents the original input.
The expanded sequence omits directives and disabled branches and can repeat text from macro definitions.
Reconstructing the original input therefore uses the physical sequence.

## Keeping Lexical Values Alive

An AST node can keep a token after the parser's input list has been destroyed.
Keeping only the token handle would leave its object pointer dependent on another owner's lifetime.
We therefore distinguish borrowed lexical values from the objects that own their storage.

`TokenPool` owns copied spelling text, immutable objects, trivia arrays, and references to names.
An ordinary `Lexeme` or `Token` borrows that pool.
The pool's copy of the text permits the original input buffer to be released while lexical text remains available.
The pool itself must remain alive for every borrowed value that is still used.

`Lexer::initialize()` uses the `SourceManager`'s pool by default.
A caller can supply another pool and keep it alive while the lexer reads into it.
`TokenList` and `LexemeList` retain all pools referenced by their elements, including EOF.
Their mutation APIs retain the required owners as values are added or replaced.
The parser arranges for `ASTBuilder` to retain the pools used by tokens stored in its AST.
`ASTBuilder` also owns a pool for synthesized and deserialized values.

A consumer keeping one token independently can use `TokenRef<T>` to retain the pool with its typed value.
Pool ownership keeps lexical text and names alive.
Interpreting a `SourceLoc` additionally requires the corresponding `SourceManager` records; an owning token does not retain those records.
Pools retained by owning handles are allocated under `RefPtr` ownership.
A single pool does not support concurrent mutation.

Importing a token into another pool copies its referenced lexical state into the receiving pool.
The imported token then depends on that pool for lexical storage, including every attached trivia object.
Its location and flags remain occurrence information.

AST serialization preserves the recognized lexical state: concrete kind, physical and logical spellings, name presence, ordered trivia, location, and occurrence flags.
Deserialization constructs that state directly in the receiving builder's pool.
For example, re-lexing the spelling `<dir/file.h>` in normal mode would produce ordinary tokens rather than the original header-name token.
The serialized kind records the recognition decision, so loading a token does not require guessing a preprocessing context.

## Maintaining the Kind Definitions

The hierarchy in [slang-lexeme-defs.lua](../../source/compiler-core/slang-lexeme-defs.lua) defines the kind enumeration, conceptual parent relationships, diagnostic descriptions, and C++ wrappers.
Keeping those definitions together ensures that kind tests and typed values agree about the same categories.

Fiddle uses the lexer, so compiling the lexer cannot depend on first running Fiddle.
Normal builds consume checked-in snapshots generated from the Lua hierarchy: the [kind definitions](../../source/compiler-core/slang-lexeme-defs.h.fiddle), [raw lexeme wrappers](../../source/compiler-core/slang-raw-lexeme-types.h.fiddle), and [token wrappers](../../source/compiler-core/slang-token-types.h.fiddle).
The `regenerate-lexeme-definitions` CMake target updates those snapshots after a hierarchy edit.
The `check-lexeme-definitions` target generates separate build-directory copies and compares them with the checked-in files.
Neither target is a prerequisite of `compiler-core`, which would create the bootstrap dependency cycle.
Changes to the hierarchy and regenerated snapshots belong in the same change.
