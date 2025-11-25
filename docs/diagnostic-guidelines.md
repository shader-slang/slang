# Slang Compiler Diagnostic Guidelines

## Overview

The Slang compiler aims to provide clear, actionable, and user-friendly diagnostics that help developers quickly understand and fix issues in their code. These guidelines draw from best practices established by Rust, Clang, and Swift compilers while adapting them for Slang's specific needs.

## Diagnostic Structure

A complete diagnostic in Slang consists of:

```
error[E0000]: main error message
  --> file.slang:LL:CC
   |
LL | <code>
   |  ^^^^ primary label
   |
LL | <related code>
   | -------------- secondary label
   |
   = note: additional context without a span
   = help: suggestion for fixing the issue
```

### Core Components

- **Level**: `error`, `warning`, `lint`, `remark` (plus attached `note`, `help`)
- **Error Code**: Optional identifier (e.g., `E0308`) for detailed documentation lookup
- **Message**: Concise description of the problem
- **Source Location**: File path, line, and column information
- **Code Snippet**: The affected code with visual indicators
- **Labels**: Primary and secondary spans with explanatory text
- **Sub-diagnostics**: Additional notes and suggestions
- **Documentation Links**: References to relevant language guide chapters

## Diagnostic Levels

### Error

Emitted when the compiler cannot proceed with compilation:

- Syntax errors
- Type mismatches that prevent code generation
- Unresolved symbols
- Constraint violations
- Missing interface implementations

### Warning

Emitted for problematic but compilable code:

- Deprecated feature usage
- Unused variables or imports
- Potentially incorrect but syntactically valid code
- Code that may behave unexpectedly
- Can be turned into errors with `-werror`

### Lint

Off-by-default style or clarity guidelines:

- Extraneous parentheses
- Style violations
- Code clarity improvements

### Note

Provides additional context for errors and warnings:

- Related code locations
- Explanations of why something failed
- References to relevant language rules

### Help

Offers actionable suggestions:

- How to fix the problem
- Alternative approaches
- Links to documentation

### Remark

Off-by-default informational messages:

- Optimization hints
- Compilation progress information
- Performance suggestions
- Code generation notes

## Writing Style Guidelines

### Message Content

1. **Be concise and precise**

   - ❌ "The compiler failed to find a matching type"
   - ✅ "type mismatch: expected `int`, found `string`"

2. **Use plain language**

   - Avoid compiler jargon when possible
   - Define technical terms when necessary
   - Write for developers who may be new to the language

3. **Include relevant context**

```
error[E0277]: interface `IAddable` is not implemented for type `String`
  --> file.slang:7:22
   |
4  | interface IAddable { This add(This other); }
   |                      ---------------------- required by this interface
5  |     String s1 = "hello";
6  |     String s2 = "world";
7  |     String result = add(s1, s2);
   |                      ^^^ `add` requires `IAddable` interface
```

### Grammar and Formatting

1. **No ending punctuation** for single-sentence messages

   - ✅ ``cannot find type `Foo` in this scope``
   - ❌ ``cannot find type `Foo` in this scope.``

2. **Use backticks** for code elements

   - Types: `` `float4` ``, `` `Texture2D<float4>` ``
   - Identifiers: `` `myVariable` ``
   - Keywords: `` `interface` ``, `` `struct` ``

3. **Lowercase start** for messages

   - ✅ `missing semicolon`
   - ❌ `Missing semicolon`

4. **Active voice** when describing problems

   - ✅ ``function `foo` takes 2 arguments but 3 were provided``
   - ❌ ``3 arguments were provided but function `foo` takes 2``

5. **Use Oxford comma** in lists

   - ✅ `` expected one of `int`, `float`, or `double` ``
   - ❌ `` expected one of `int`, `float` or `double` ``

6. **Use correct articles** (a vs. an)
   - ✅ `an interface`
   - ✅ `a struct`
   - ✅ ``an `IFoo` implementation``
   - ❌ `a interface`

### Type Aliases and Underlying Types

When type aliases are involved, show the underlying type when it helps clarify the error:

```
error[E0308]: type mismatch
  --> file.slang:10:23
   |
10 |     ColorRGBA color = 0.5;
   |                       ^^^ expected `ColorRGBA` (aka `float4`), found `float`
```

Display options for controlling type alias expansion:

- `-show-type-aliases=always`: Always show "aka" annotations
- `-show-type-aliases=helpful`: Show only when it clarifies (default)
- `-show-type-aliases=never`: Never expand type aliases

## Error Codes

### Format

- Use a letter prefix followed by 5 digits: `E00001`, `W00001`
- Group related errors in ranges:
  - **TBD**

### Documentation

**Each error code needs:**

- Brief description
- Links to documentation

**Optionally:**

- Common causes
- Example code that triggers the error
- Suggested fixes

## Suggestions and Fix-its

### Applicability Levels

1. **MachineApplicable**: Can be automatically applied

```
help: add missing semicolon
   |
5  |     return value;
   |                 +
```

2. **HasPlaceholders**: Requires user input

```
help: specify the type explicitly
   |
5  |     let color: <type> = value;
   |              +++++++++
```

3. **MaybeIncorrect**: Suggestion might not be appropriate

```
help: consider adding the `[shader("compute")]` attribute
   |
5  | [shader("compute")]
   | +++++++++++++++++++
6  | void main() {
```

### Guidelines for Suggestions

- Provide fix-its only when confidence is high
- Show the exact change needed
- Use placeholders (`<type>`, `<name>`) when user input is required
- Prefer showing code transformations over textual descriptions

## Span and Location Information

### Primary Spans

- Point to the exact location of the error
- Keep spans as small as possible while remaining meaningful
- For multi-token constructs, highlight the most relevant part

### Secondary Spans

- Show related code that contributes to the error
- Use different labels to distinguish multiple spans
- Order spans by relevance, not just by source location

### Example

```
error[E0308]: type mismatch in function call
  --> file.slang:10:11
   |
8  | void expectInt(int x) { }
   |                ----- expected `int` here
9  |
10 |     expectInt("hello");
   |               ^^^^^^^ found `string`
```

## Error Cascading Prevention

We shouldn't be generating many dependent errors from a single mistake.

We should at least be checking that there are no additional error messages in all our diagnostic tests. At the moment we generally only check for the presence of the tested diagnostic.

To avoid overwhelming users with follow-on errors:

1. **Stop type-checking** in a scope after critical type errors
2. **Mark symbols as poisoned** when their definition has errors
3. **Limit error propagation** from generic instantiation failures
4. **Track error origins** to suppress duplicate reports

Example:

```
error[E0412]: the type `MyTexture` is not defined
  --> file.slang:5:5
   |
5  |     MyTexture tex;
   |     ^^^^^^^^^ type not found
   |
   = note: subsequent errors involving `tex` have been suppressed
```

## Diagnostic Priority and Limits

### Priority System

When multiple errors exist, show them in this order:

TBD

1. Syntax errors
2. Import/module errors
3. Type definition errors
4. Interface implementation errors
5. Type mismatch errors
6. Other semantic errors
7. Warnings
8. Remarks

### Error Limits

- Configurable via `-max-errors=N`
- Show message when limit reached:

```
  error: aborting due to 20 previous errors; use `-max-errors=N` to see more
```

## Lint System

Lints are a good opportunity to attach fix-its for a LSP or LLM.

### Lint Naming

- Use snake_case
- Name should make sense with "allow": `allow unused_variables`
- Be specific about what is being checked
- Group related lints with common prefixes

### Lint Levels

1. **allow**: Off by default
2. **warn**: On by default, produces warnings
3. **deny**: On by default, produces errors

### Lint Groups

Define logical groups:

- **style**: Code formatting and naming conventions
  - NON_CAMEL_CASE_NAMES
  - NON_UPPER_CASE_CONSTANTS
  - INCONSISTENT_SPACING
- **correctness**: Potential bugs or incorrect usage
- **performance**: Performance-related suggestions

## Special Diagnostic Features

### Generic Type Diffing

When dealing with complex generic types, highlight differences:

```
error[E0308]: type mismatch
  = note: expected `RWStructuredBuffer<float4>`
          found    `RWStructuredBuffer<float3>`
                           ^^^^^^ types differ here
```

### Macro Expansion Context

Show the expansion chain for errors in macros:

```
error[E0369]: invalid operation
  --> file.slang:20:5
   |
20 |     MY_MACRO!(x + y);
   |     ^^^^^^^^^^^^^^^^^ in this macro invocation
   |
  ::: macros.slang:5:10
   |
5  |     $left + $right
   |           ^ cannot add these types
```

### Similar Name Suggestions

```
error[E0425]: cannot find `printn` in scope
  --> file.slang:5:5
   |
5  |     printn("hello");
   |     ^^^^^^ not found
   |
   = help: a similar function exists: `println`
help: did you mean `println`?
   |
5  |     println("hello");
   |     ~~~~~~~
```

## IDE Integration

### LSP-Specific Formatting

Optimize diagnostics for Language Server Protocol:

- Include `DiagnosticRelatedInformation` for secondary spans
- Provide `CodeAction` items for fix-its
- Support incremental diagnostic updates
- Include diagnostic tags (deprecated, unnecessary)

### Inline Error Markup

Specifications for IDE display:

```json
{
  "severity": "error",
  "range": {
    "start": { "line": 10, "character": 5 },
    "end": { "line": 10, "character": 10 }
  },
  "message": "undefined variable `count`",
  "code": "E00123",
  "codeDescription": { "href": "https://docs.shader-slang.org/errors/E00123" }
}
```

### Quick-Fix Protocol

Standardized fix communication:

```json
{
  "title": "Add missing interface implementation",
  "kind": "quickfix",
  "diagnostics": ["E00987"],
  "edit": {
    "changes": {
      "file.slang": [
        {
          "range": { "start": { "line": 15, "character": 0 } },
          "newText": "interface MyStruct : IRenderable {\n    // implementation\n}\n"
        }
      ]
    }
  }
}
```

### Diagnostic Severity Mappings

Map compiler levels to IDE severity:

- `error` → `DiagnosticSeverity.Error` (1)
- `warning` → `DiagnosticSeverity.Warning` (2)
- `remark` → `DiagnosticSeverity.Information` (3)
- `note` → `DiagnosticSeverity.Hint` (4)

## Internationalization

TBD (can we use LLMs here?)

## Testing Diagnostics

### Diagnostic Verification

TBD Test file syntax to be parsed and checked against machine readable output

Filecheck style test descriptions, but can be tested using the machine readable output.

```
void test() {
    int x = "string";
    // ERROR: type mismatch
    //      ^^^^^^^^ expected `int`, found `string`
    // HELP: change the type annotation
}
```

### Test Coverage Requirements

- Each diagnostic should have at least one test
- Test both positive and negative cases
- Verify fix-its compile successfully
- Check error recovery after applying suggestions

## Progressive Disclosure

### Beginner-Friendly Defaults

- Show simple, actionable messages by default
- Hide implementation details unless relevant
- Provide links to learn more

## Performance Considerations

1. Don't compute expensive diagnostics unless needed
2. Avoid reporting the same error multiple times
3. Cache diagnostic messages for repeated errors
4. Use error limits to prevent runaway diagnostics

## Command-Line Interface

### Display Options

- `-error-format=json`: Machine-readable output
- `-color=auto|always|never`: Control color output
- `-show-error-codes`: Display error codes
- `-explain E00001`: Show detailed error explanation
- `-verbose-diagnostics`: Show additional diagnostic information
- `-max-errors=N`: Set maximum error count
- `-show-type-aliases=always|helpful|never`: Control type alias display

### Verbose Mode

With `-verbose-diagnostics`:

- Show full type signatures including type aliases
- Include compiler passes information
- Show all possible fixes, not just the most likely
- Display internal compiler state when relevant

### Example JSON Output

```json
{
  "level": "error",
  "code": "E0308",
  "message": "type mismatch",
  "spans": [
    {
      "file": "main.slang",
      "line": 10,
      "column": 15,
      "text": "float3 color = float4(1, 0, 0, 1);",
      "label": "expected `float3`, found `float4`"
    }
  ],
  "children": [
    {
      "level": "help",
      "message": "use `.xyz` to extract the first three components",
      "spans": [
        {
          "file": "main.slang",
          "line": 10,
          "column": 35,
          "suggestion": ".xyz"
        }
      ]
    }
  ],
  "documentation_url": "https://docs.shader-slang.org/errors/E00345"
}
```

## Best Practices Checklist

Before adding a new diagnostic:

- [ ] Is the message clear and actionable?
- [ ] Is the span as precise as possible?
- [ ] Would a fix-it help?
- [ ] Error code
- [ ] Is the severity level appropriate?
- [ ] Are related locations shown with notes?
- [ ] Is the message properly capitalized and punctuated, grammar etc.
- [ ] Will this message make sense in different contexts?
- [ ] Have we considered error cascading?
- [ ] Is there a relevant documentation link?
- [ ] Does the documentation have examples?
- [ ] Have we added tests for this diagnostic?

## Examples of Good Diagnostics

### Type Mismatch

```
error[E0308]: mismatched types
  --> src/main.slang:5:16
   |
4  | float3 expectVec3(float3 v) { return v; }
   |                   ------- expected due to this parameter type
5  |     expectVec3(float4(1, 0, 0, 1));
   |                ^^^^^^^^^^^^^^^^^^^ expected `float3`, found `float4`
   |
   = help: use `.xyz` to extract the first three components
   = note: see https://docs.shader-slang.org/types/vectors for vector swizzling
```

### Missing Interface Implementation

```
error[E0277]: type `String` doesn't implement interface `IArithmetic`
  --> src/main.slang:10:24
   |
10 |     String result = s1 + s2;
   |                        ^ operator `+` requires `IArithmetic` interface
   |
   = note: the interface `IArithmetic` is not implemented for `String`
   = note: string concatenation requires explicit method calls
   = help: use `s1.concat(s2)` instead
   = note: see https://docs.shader-slang.org/interfaces/operators
```

These guidelines should be treated as living documentation that evolves with the Slang compiler's needs and user feedback. Regular reviews and updates ensure diagnostics remain helpful and relevant.
