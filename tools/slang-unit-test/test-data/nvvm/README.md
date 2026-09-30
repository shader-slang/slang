# NVVM bitcode fixture

`minimal-empty-kernel.ll` is the readable source for the byte array in
`unit-test-nvvm-bitcode-fixture.h`. The fixture was assembled by llvmlite 0.42.0, which embeds LLVM
14.0.6 with typed pointers enabled. From this directory, regenerate the header in an isolated Python
environment with:

```text
py -3.11 -m venv ..\..\..\..\build\nvvm-fixture-env
..\..\..\..\build\nvvm-fixture-env\Scripts\python.exe -m pip install llvmlite==0.42.0
..\..\..\..\build\nvvm-fixture-env\Scripts\python.exe generate.py
```

The resulting bitcode is 1,668 bytes, starts with `42 43 c0 de`, and has SHA-256
`b45e3b74a3881b178c3d45310cc74d0bed3ece46e7101e6b9ac98a66aa301f01`. CUDA 12.2 libNVVM 2.0
verifies and compiles it for `compute_75`; CUDA 12.2 `ptxas` 12.2.140 accepts the generated PTX.

`generate.py` checks the llvmlite and embedded LLVM versions, verifies the module, serializes the
bitcode, checks its size, magic, and SHA-256, and renders the C++ byte array. llvmlite is prototype
tooling and is not a Slang build or test dependency. Regenerate the header only when deliberately
updating this compatibility fixture, and preserve the exact producer version and hash here.

## Selected device-library signature controls

`library-*.ll` define small `__nv_roundf` functions for the byte arrays in
`unit-test-nvvm-library-signature-fixtures.h`. These controls use LLVM14 bitcode and are independent
of the provider's construction API. Their bodies return the input; they test linkage, calling
convention and signature admission, not the numerical round operation.

Seven modules verify as valid LLVM IR but are deliberately outside the direct-call ABI: internal,
available-externally and hidden definitions; fastcc and variadic definitions; and inreg on the
parameter or result. `library-invalid-fastcc-variadic.ll` combines fastcc with varargs: it parses
as bitcode but fails LLVM verification. This separates reader failure from unsupported signatures
and establishes the callback diagnostic lifetime without creating an output module.

The checked-in bytes were produced by LLVM14.0.6 Core/BitWriter and roundtripped through BitReader.
To regenerate from these readable sources, use the same isolated llvmlite0.42.0 / LLVM14.0.6
setup described above and run `generate-library-signatures.py`. The generator verifies the expected
valid/invalid distinction before writing the arrays. Bitcode serialization may retain producer
metadata; byte identity is not a shader ABI claim. Review readable IR and rerun the provider units
whenever regenerating. Neither llvmlite nor llvm-as is a build or test dependency.
