# Installation and setup

To use the heterogeneous programming features, clone and build the `h-slang` branch of the slang [repository](../../README.md).

## Build Slang

Follow Slang's [build prerequisites and platform setup](../../docs/building.md)
for the C++ toolchain, CMake, Ninja/Visual Studio, and Python. Fetch the repository and its submodules:

```sh
git clone --branch h-slang --recurse-submodules https://github.com/shader-slang/slang.git
cd slang
```

With Ninja and the compiler environment configured:

```sh
cmake --preset default -DSLANG_ENABLE_SLANGI=ON -DSLANG_ENABLE_SLANG_RHI=ON
cmake --build --preset releaseWithDebugInfo --target slangi slangc slangd
```

On Windows with Visual Studio 2022:

```powershell
cmake --preset vs2022-dev -DSLANG_ENABLE_SLANGI=ON -DSLANG_ENABLE_SLANG_RHI=ON
cmake --build --preset vs2022-dev-releaseWithDebugInfo --target slangi slangc slangd
$env:PATH = "$PWD\build\windows-vs2022-dev\RelWithDebInfo\bin;$env:PATH"
```

The default preset puts tools in `build/RelWithDebInfo/bin`. Add that directory to your
`PATH`; keep its companion libraries beside the executables. GPU examples need a
compatible device and driver.

## Debug in VS Code

Use the `h-slang` branch of
[slang-vscode-extension](https://github.com/saipraveenb25/h-slang-vscode-extension/tree/h-slang),
which includes the `slang-hgvm` debug adapter. See
[Debugging](debugging.md#vs-code-dap-setup) for extension setup, `settings.json`,
`launch.json`, and buffer/texture inspection.

Next: [run a program](invoking.md), or set up the
[Python, C++, and JavaScript bindings](interoperability.md#setting-up).
