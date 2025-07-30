# Debugging Slang

This document gives examples showing how to run debuggers in the Slang codebase.
Follow the [Building Slang From Source](/docs/building.md) instructions first.

## Visual Studio

This repo includes multiple `*.natvis` files which Visual Studio picks up
automatically; no extra configuration is required.

## LLDB

If you use [LLDB][], we provide a `.lldbinit` file which enables data formatters
for types in the Slang codebase. You can use this with LLDB in your terminal via
the [`--local-lldbinit`][] flag; for example:

```
$ cmake --build --preset debug
$ lldb --local-lldbinit build/Debug/bin/slangc -- tests/byte-code/hello.slang -dump-ir
(lldb) breakpoint set --name dumpIR
(lldb) run
```

LLDB can be used with either GCC or Clang, but Clang seems to behave better
about respecting breakpoint locations and not having missing variables.

### VS Code

If instead you prefer to debug within VS Code, you can run LLDB via the
[CodeLLDB][] extension. For example, to recreate the same debugging session as
above, create a `.vscode/tasks.json` file with these contents:

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Debug build",
      "type": "shell",
      "command": "cmake",
      "args": ["--build", "--preset", "debug"]
    }
  ]
}
```

Then create a `.vscode/launch.json` file with these contents:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "LLDB",
      "preLaunchTask": "Debug build",
      "type": "lldb",
      "request": "launch",
      "initCommands": ["command source .lldbinit"],
      "program": "build/Debug/bin/slangc",
      "args": ["tests/byte-code/hello.slang", "-dump-ir"]
    }
  ]
}
```

Finally, place any breakpoints you want, and hit F5.

[`--local-lldbinit`]: https://lldb.llvm.org/man/lldb.html#cmdoption-lldb-local-lldbinit
[codelldb]: https://marketplace.visualstudio.com/items?itemName=vadimcn.vscode-lldb
[lldb]: https://lldb.llvm.org/index.html
