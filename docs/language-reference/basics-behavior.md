# Program Behavior

## Observable behavior {#observable}

TODO

## Classification of Behavior {#classification}

Slang classifies the observable behavior of a program as follows:

1. **Precisely defined.** The observable behavior is defined precisely for all targets. Examples of precisely
   defined behavior:
   - Basic [unsigned integer](types-fundamental.md#integer) operations such as addition, subtraction,
     multiplication.
2. **Implementation-defined.** The observable behavior is defined by the target and it is documented. The target
   consists of the shader compilation target, the declared extensions, and the target device with
   drivers. Examples of implementation-defined behavior:
   - Size of [bool](types-fundamental.md#boolean)
   - Evaluation of [floating point](types-fundamental.md#floating) numbers. For example, whether the target
     implements [IEEE 754-2019](https://doi.org/10.1109/IEEESTD.2019.8766229) standard or something else.
   - Memory layout when composed from fundamental types
   - Target capabilities
   - Available texture types and operations
3. **Unspecified.** The observable behavior is defined by the target but documentation is not
   required. Examples of unspecified behavior:
   - The bit-exact formulae for texture sampling algorithms
   - Memory layouts of opaque types and their underlying data
4. **Undefined.** The program behavior is undefined. No guarantees are made. Possible results include a
   program crash; data corruption; and differing computational results depending on optimization level, target
   language/driver/device, or timing. Examples of undefined behavior:
   - Data race
   - Out-of-bounds memory access
   - Application use of Slang internal language features
