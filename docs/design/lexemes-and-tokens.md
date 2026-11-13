# Lexemes and Tokens

The lexer represents physical input as atomic lexemes and groups those lexemes into tokens with trivia.
Each occurrence stores a pointer to an immutable object, a `SourceLoc`, and preprocessing context flags.
`TokenPool` owns and interns the objects, their text, and the arrays of attached trivia.
Object identity excludes occurrence locations and flags.

## Atomic Lexemes and Typed Values

`Lexeme` is the base value type for both raw tokens and trivia.
`LexemeObj` stores a concrete kind, original spelling, logical spelling, and an optional interned `Name*`.
For lexed objects, original spelling is the exact text consumed from a `SourceView` and logical spelling removes escaped newlines.
For synthesized tokens, the text factory uses its supplied text for both spellings.
Logical spelling does not decode escapes inside string or character literals.
Both spelling forms remain available after the input buffer is released.

The hierarchy in [slang-lexeme-defs.lua](../../source/compiler-core/slang-lexeme-defs.lua) defines the unified kind enumeration, conceptual inheritance, diagnostic descriptions, and C++ value wrappers.
An abstract case classifies values but cannot be allocated as an object.
Raw token wrappers have names such as `RawIdentifier`; their counterparts with trivia have names such as `IdentifierToken`.
Both hierarchies support abstract groups, including `RawNumericLiteral` and `LiteralToken`.
The wrappers add no stored state or virtual dispatch.

A null object pointer denotes an empty value.
EOF is a concrete token with an object and zero spelling length.
Conditional token reading returns typed values:

```cpp
if (auto comma = reader.advanceIf<CommaToken>())
{
    recordComma(comma.getLoc());
}
```

A failed conditional read leaves the cursor unchanged.
Required parser reads retain their separate diagnostic and recovery behavior; an unexpected token is never presented as a successfully checked typed value.

## Trivia Attachment

`Lexer::lexLexeme()` returns one atomic lexeme, including trivia.
`Lexer::lexToken()` returns one raw token with leading and trailing trivia.
Each trivia lexeme belongs to exactly one cluster.

Leading trivia includes everything before the first raw token and any trivia left after the preceding token's first explicit newline.
Trailing trivia extends through and includes the first explicit `NewLine` lexeme.
A block comment remains atomic even when it contains newlines.
EOF owns any remaining leading trivia, including all input when a file contains only trivia.

For example, the first token in `a /*comment*/\n\n  b` owns the space, comment, and first newline as trailing trivia.
The second newline and indentation lead `b`.
Concatenating every cluster's leading spellings, raw spelling, and trailing spellings reconstructs the complete `SourceView` input.
`SourceFile` may already have decoded the file or removed a byte-order mark before the lexer receives that input.

A backslash-escaped newline within a token remains in its original spelling.
At a lexeme boundary it is represented by `EscapedNewLine` trivia with empty logical spelling.
It does not end a preprocessor directive or introduce whitespace between logical tokens.

The location on a physical `Token` identifies its raw token.
Trivia locations follow from the original spelling lengths of the contiguous cluster.
Unknown locations remain unknown when deriving trivia locations.
Attached trivia has zero context flags; the containing raw token retains the flags used by preprocessing and parsing.

## Preprocessing and Contextual Lexing

Trailing-trivia collection classifies characters without lexing the next raw token.
It also stops immediately after the first explicit newline.
The preprocessor can therefore change lexer context before the next operand or line is recognized.

For `#include<dir/file.h>`, the preprocessor enables header-name mode before requesting the operand.
The lexer recognizes `<dir/file.h>` as one `HeaderName`, including spaces or comment-like characters within the delimiters.
Macro-expanded angle operands can instead be assembled from expanded ordinary tokens.

The preprocessor exposes an attached explicit newline internally as a zero-width `EndOfDirective` token.
The newline remains trivia in its physical cluster.
Lazy input-stream caches ensure that consuming this delimiter does not recognize the next line before conditional-compilation diagnostic policy changes.

Macro occurrence locations continue to use the existing `SourceManager` and `SourceView` mapping.
Interned objects contain no macro invocation locations.
Relocating a token discards its attached trivia because the original cluster need not be contiguous at the new location.
Replacing its spelling also discards attached trivia.
The physical lexeme stream remains the representation for reconstructing the original input; preprocessor output does not promise reconstruction of directives or disabled branches.

Clang provides a precedent for keeping compact locations while distinguishing spelling and expansion origins in the source manager.
An eventual expansion mapping for transferred trivia would need to preserve origins of individual cluster segments.
That mapping is outside the current representation change.
See the [Clang location design](https://clang.llvm.org/docs/InternalsManual.html#the-sourcelocation-and-sourcemanager-classes).

The newline attachment rule matches Roslyn's ordinary trailing-trivia scan.
SwiftSyntax uses a different convention, so newline ownership is a deliberate policy rather than a universal compiler rule.
See [Roslyn's lexer](https://github.com/dotnet/roslyn/blob/main/src/Compilers/CSharp/Portable/Parser/Lexer.cs) and [SwiftSyntax trivia modes](https://github.com/swiftlang/swift-syntax/blob/main/Sources/SwiftParser/Lexer/Cursor.swift).

## Ownership and Serialization

`TokenPool` combines allocation ownership and interning policy; there is no separate factory object.
`Lexer::initialize()` uses a pool owned by `SourceManager` by default.
A caller can supply a different `TokenPool*`, which must remain alive while that lexer is used.
`ASTBuilder` retains the pools of parsed tokens and owns a pool for synthesized and deserialized tokens.
`TokenList` and `LexemeList` retain every referenced pool.
Their stored EOF is the `end()` sentinel and is excluded from ordinary iteration.
Sequence mutation uses `add()` or `setTokens()` so values cannot bypass ownership retention.

Ordinary lexical values borrow their pool.
`TokenRef<T>` retains a pool alongside a typed token when a caller needs independent lexical storage ownership.
It does not retain the source-manager records required to interpret a location.
Cross-pool token import copies all referenced lexical objects into the receiving pool.
Pools retained by owning handles must use `RefPtr` allocation.
Concurrent mutation of one pool is unsupported.

AST serialization writes original spelling, logical spelling, optional-name state, ordered trivia, location, and context flags.
Deserialization constructs and interns these values directly in the receiving builder's pool.
It does not re-lex text under an inferred preprocessing context.
Arena blocks and pointer arrays provide stable storage; serialization into a small set of flat indexed arrays remains a separate possible improvement.

## Bootstrapping the Definitions

Fiddle uses the lexer, so the lexer cannot require running fiddle before it is compiled.
The generated [definition snapshot](../../source/compiler-core/slang-lexeme-defs.h.fiddle) is checked in alongside the Lua definition.
Normal builds consume that snapshot.

After editing the Lua hierarchy, the `regenerate-lexeme-definitions` CMake target updates the snapshot.
The `check-lexeme-definitions` target generates into the build directory and compares the result with the checked-in snapshot.
Neither target is a prerequisite of `compiler-core`.
Changes to the hierarchy and regenerated snapshot belong in the same change.
