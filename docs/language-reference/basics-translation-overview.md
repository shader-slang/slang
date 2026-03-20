# Translation Overview

**TODO**

Translation phases:

1. Module translation
   1. Tokenization (incl. preprocessing)
   2. Parsing and semantic checking
   3. Translation to intermediate representation
2. Linking, specialization, and code generation
   1. Link modules
      - Creates a single module that contains all entry points and their dependent intermediate representation
        code
   2. Specialization for target (incl. generics)
      - `static const` constants must be resolved here at the latest
   3. Target code emission
3. Target compilation and linking (implementation-specified)
   1. Compile target code to target intermediate representation (optional, depends on target)
   2. Target intermediate representation to target machine code (optional, depends on target)
      - Often just-in-time compilation
