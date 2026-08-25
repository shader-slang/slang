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
