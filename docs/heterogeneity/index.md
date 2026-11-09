# Overview

!!! danger "EXPERIMENTAL — unfinished API, rough edges"

    **`h-slang` is experimental work in progress.** APIs, syntax, and behavior can change.
    Expect missing features, bugs, and performance limitations. This is an experimental preview.

`h-slang` is a flavor of the [Slang](https://shader-slang.org/) language that explores heterogeneous programming.

The main goal of `h-slang` is to make it easy to write, experiment and share neural graphics programs & techniques.

Some core goals:

- **Compact & easy heterogeneous programming**
  Orchestrate compute kernels, rasterization/ray-tracing pipelines and interleave host and device code seamlessly. Take full advantage of Slang's advanced type-system (generics, modules, lambdas, etc.) to write flexible and reusable heterogeneous code.
- **Easy interoperability with general-purpose languages**
  Call host functions from Python, C++, or JavaScript, enabling seamless portability across platforms.
- **Automatic differentiation across host and device code**
  Extend Slang's auto-diff to support computations that span both host and device code, allowing gradients to propagate seamlessly across the heterogeneous program.

Additionally, `h-slang` serves as a research framework for experimenting with new language & compiler features to improve heterogeneous programming in general.

- [Install and build from source](installation.md)
- [Run your first program](invoking.md)
