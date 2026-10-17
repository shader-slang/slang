# Change Log

# v2.0.1
- Update to Slang v2025.14.3.

# v2.0.0
- Update to Slang v2025.13.2.
- Add Slang Playground Featuers
  - Run shaders
  - Compile shaders
  - Get reflection information
- Added browser support

# v1.10.0
- Fixes highlighting, hover, and signature info for constructor calls.
- Signature help now automatically selects the best candidate.
- Go-to-definition can now display core module source.
- Auto completion now sorts the candidate based on relevance to context.
- Adds a new setting to turn on/off format-on-type.
- Auto completion now supports completing implementing method signatures after typing `override`.
- Fixes highlighting/hover for associated type declref exprs
- Fix various decl signature printing.

# v1.9.12
- Update to Slang v2025.11.

# v1.9.11
- Update to Slang v2025.10.4.

# v1.9.10
- Update to Slang v2025.10.1.

# v1.9.9
- Update to Slang v2025.6.1.

# v1.9.6
- Update to Slang v2025.4.

# v1.9.5
- Update to Slang v2025.3.3, with support to `CoopVec` type.

## v1.9.4
- Update to Slang v2025.2.

## v1.9.3
- Update to Slang v2024.17.

## v1.9.1
- Update to Slang v2024.16.

## v1.9.0
- Update to Slang v2024.15.

## v1.8.19
- Update to Slang v2024.14.3.

## v1.8.18
- Update to Slang v2024.11.

## v1.8.17
- Update to Slang v2024.10

## v1.8.16
- Update to Slang v2024.1.34.

## V1.8.15
- Update to Slang v2024.1.30.

## v1.8.14
- Update to Slang v2024.1.28.
- Fix performance regression.

## v1.8.13
- Update to Slang v2024.1.20.
- Add syntax support for `if (let x = expr)`.

## v1.8.12
- Update to Slang v2024.1.6
- Fixes some parsing and lookup rules.

## v1.8.11
- Update to Slang v2024.1.5
- Parser fixes.

## v1.8.10
- Update to Slang v2024.1.4
- Type system fixes.

## v1.8.9
- Update to Slang v2024.1.2.
- Allow angle-bracket #include path.
- Support unscoped enum types through the [UnscopedEnum] attribute.
- Add diagnostic on invalid generic constraint type.

## v1.8.8
- Update to Slang v2024.1.1.
- Fix a compiler crash during constant folding.

## v1.8.7
- Update to Slang v2024.1.0.
- Add highlighting for `dynamic_uniform` keyword.

## v1.8.6
- Update to Slang v2024.0.14.
- Add support for bitfields.

## v1.8.5
- Update to Slang v2024.0.10.
- Robustness fixes.

## v1.8.4
- Update to Slang v2024.0.5.
- Pointer support.
- Type checking fix around generic arrays.

## v1.8.3
- Correctly highlight half literal suffix.

## v1.8.2
- Update to Slang v2024.0.2.
- Add support for capabilities.

## v1.8.1
- Update to Slang v2023.5.4.
- Add support for Source Engine shader files.

## v1.8.0
- Update to Slang v2023.5.3.

## v1.7.26
- Update to Slang v2023.5.2.
- Improve namespace highlighting.
- Add diagnostic for invalid modifier use.

## v1.7.25
- Update to Slang v2023.5.1.
- Namespace and module fixes.

## v1.7.24
- Update to Slang v2023.5.0.
- Add support for __include and access control.

## v1.7.23
- Update to Slang v2023.4.10.
- Fixes an issue where signature help fails to appear.

## v1.7.21
- Update to Slang v2023.4.8.
- Stability fixes.
- Improves highlighting/hinting support around constructors.

## v1.7.20
- Update to Slang v2023.4.6.

## v1.7.19
- Switch to x64 on Windows.

## v1.7.18
- Update to Slang v2023.4.5.

## v1.7.17
- Update to Slang v2023.4.3.

## v1.7.16
- Update to Slang v2023.3.20.

## v1.7.15
- Update to Slang v2023.3.19.

## v1.7.13
- Update grammar for `__target_switch`.

## v1.7.12
- Update to Slang v2023.3.18
- Add highlighting support for `spirv_asm`.

## v1.7.11
- Update to Slang v2023.3.16

## v1.7.10
- Update to Slang v2023.3.10

## v1.7.9
- Update to Slang v2023.3.7
- Highlighting `__target_switch`, `__intrinsic_asm`, `spirv_asm` keywords.

## v1.7.8
- Update to Slang v2023.3.6
- Fixes regressions.

## v1.7.6
- Update to Slang v2023.3.5
- Fix a crash during type checking.
- Fix incorrect highlighting of attribute names with scope qualifiers.

## v1.7.5
- Update to Slang v2023.3.4

## v1.7.4
- Update to Slang v2023.1.0

## v1.7.3
- Update to Slang v0.28.3

## v1.7.2
- Update to Slang v0.28.1

## v1.7.1
- Update to Slang v0.28.0

## v1.7.0
- Update to Slang v0.27.21
- Fix a crash when checking `for` loops.
- Add support for finalized autodiff keywords.

## v1.6.10
- Update to Slang v0.27.18
- Improved intellisense for attribute arguments.

## v1.6.8
- Update to Slang v0.27.16
- Add hover info for attributes.
- Show variable scope in hover info.
- Show documentation for builtin decls.
## v1.6.7
- Update to Slang v0.27.14
- Fixed a crash when checking incomplete `typealias` declaration.

## v1.6.6
- Update to Slang v0.27.9

## v1.6.5
- Update to Slang v0.27.8
- Added more diagnostics.

## v1.6.4
- Update to Slang v0.27.7

## v1.6.3
- Update to Slang v0.27.4

## v1.6.2
- Update to Slang v0.27.3

## v1.6.1
- Update to Slang v0.27.1

## v1.6.0
- Update to Slang v0.27.0
- Added support for new attributes and syntax for authoring PyTorch kernels.

## v1.5.13
- Update to Slang v0.25.3.
- Fixed an issue where breadcrumb AST nodes can mess up semantic highlighting and hover info.

## v1.5.12
- Update to Slang v0.25.2.
## v1.5.11
- Update to Slang v0.25.0.
- Fixed a bug that causes hover info and goto definition to stop working when switching between document.

## v1.5.10
- Update to Slang v0.24.54.

## v1.5.9
- General bug fixes and update to Slang v0.24.53.

## v1.5.8
- Update to Slang v0.24.47.

## v1.5.7
- Added semantic highlighting for attributes.
- Changed auto-format behavior to no longer insert space between `{}`.
- Fixed an highlighting issue for interface-typed values.
- Update to Slang v0.24.46.

## v1.5.6
- Update to Slang v0.24.45

## v1.5.5
- Improved parser recovery around unknown function modifiers.
- Added keyword highlighting for auto-diff feature.
- Update to Slang v0.24.44

## v1.5.4
- Update to Slang v0.24.37, which brings experimental support for auto differentiation.
- Add `slang.slangdLocation` configuration to allow the extension to use custom built language server.

## v1.5.3
- Update to Slang v0.24.35, which brings support for `[ForceInline]`.

## v1.5.2
- Update to Slang v0.24.34, which brings support for multi-level `break`.
- Add missing highlighting for `uint` type.

## v1.5.1
- Fix a crash when parsing entry-point functions that has type errors in the signature.

## v1.5.0
- Release on Win-ARM64 and MacOS-ARM64.

## v1.4.7
- Add `slang.format.clangFormatFallbackStyle` setting that will be used when `style` is `file` but a `.clang-format` file is not found.
- Changed the default value of `slang.format.clangFormatStyle` to `file`.
- The default value of `slang.format.clangFormatFallbackStyle` is set to `{BasedOnStyle: Microsoft, BreakBeforeBraces: Allman, ColumnLimit: 0}`.

## v1.4.6
- Update to Slang v0.24.23, which brings support for `intptr_t`, `printf`, raw string literals and partial inference of generic arguments.
- Add highlighting for raw string literals.

## v1.4.5
- Update to Slang v0.24.15 to support the new builtin interfaces (`IArithmetic`, `IInteger`, `IFloat` etc.)
- Add syntax highlighting for `half`.

## v1.4.4
- Update to Slang v0.24.14 to support constant folding through interfaces (associated constants).

## v1.4.3
- Update to Slang v0.24.13, which brings more powerful constant folding.
- Allow deleting and retyping "," to trigger function signature info.

## v1.4.2
- Update default Clang-format style to be more conservative on line-break changes.
- Update Slang compiler version to v0.24.12 to support new language features.

## v1.4.1
- Prevent "." and "-" from commiting `include` suggestions.
- Add settings to disallow line-break changes in auto formatting.
- Format on paste no longer changes lines after the cursor position.
- Fix auto indentation after typing `{` following `if`, `while`, and `for`.

## v1.4.0
- Support auto completion for `include` and `import` paths.
- Display formatted doxygen comments in hover info.
- Reduced package size.

## v1.3.3
- Fine tuned code completion triggering conditions.
- Add configurations to turn on/off each individual type of inlay hints.
- Fixed a regression that caused the commit character configuration to have no effect.

## v1.3.2
- Update default clang-format style setting.

## v1.3.1
- Fine tuned completion and auto formatting experience to prevent them from triggering on unexpected situations.
- Fixed clang-format discovering logic on Linux.

## v1.3.0
- Auto formatting using clang-format.
- Inlay hints: show inline hints for parameter names and auto deduced types.
- Fixed a bug where setting predefined macros without a value caused language server to crash.

## v1.2.2
- Auto completion now provides suggestions for general types, expressions and bracket attributes in addition to members.
- Add configuration to turn on/off commit characters for auto completion.
- Support highlighting macro invocations.
- Support hover info for macro invocations.
- Support goto definition for #include and import.
- Performance improvements. Reduces auto completion/semantic highlighting reponse time by 50%.

## v1.2.0
- Supports document symbol outline feature.
- Add MacOS support.
- Fixed highlighting of `extension` decl's target type.
- Add missing highlight for several HLSL keywords.
- Stability fixes.

## v1.1.2
- Add configuration for search paths.
- Fix highlighting bug.
- Improved parser recovery around undefined macro invocations.

## v1.1.1
- Fixed several scenarios where completion failed to trigger.
- Fixed an issue that caused static variable members to be missing from static member completion suggestions.

## v1.1.0
- Improved performance when processing large files.
- Coloring property accessors and constructor calls.
- Exclude instance members in a static member completion query.
- Improved signature help cursor range check.
- Support configuring predefined preprocessor macros in extension settings.
- Add auto completion suggestions for HLSL semantics.
- Improved parser robustness.

## v1.0.7
- Further improves parser stability.
- Add coloring for missing hlsl keywords.

## v1.0.4
- Add commit characters for auto completion.
- Fixed parser crashes.

## v1.0.3
- Fixed file permission issue of the language server on linux.

## v1.0.2
- Fixed bugs in files that uses `#include`.
- Reduced diagnostic message update frequency.

## v1.0.1
- Package up javascript files for faster performance.
- Add icon.
- Add hlsl file extension.

## v0.0.1
- Initial release. Basic syntax highlighting support for Slang and HLSL.
