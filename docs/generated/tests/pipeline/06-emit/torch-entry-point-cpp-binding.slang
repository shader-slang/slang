//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#torch
//META: doc_section_digest=ef5cfd163a728cc46ca4433bc278ab9ed4d15e8a9bf778e3f189b3818e2edee9
//META: purpose=The Torch backend emits the PyTorch C++ glue for a `[TorchEntryPoint]` function — including a `SLANG_PRELUDE_EXPORT` declaration and a `pybind11`-style `m.def(...)` binding — and references the slang-torch-prelude.h header.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's `## Backends > Torch` section names slang-emit-torch.{h,cpp}
// as emitting "the PyTorch C++ glue used to bind Slang shaders into
// PyTorch extensions" and points at prelude/slang-torch-prelude.h.
// Pin the canonical text artifacts the doc implies:
//   - the prelude header is referenced;
//   - the `[TorchEntryPoint]` function is exported via
//     `SLANG_PRELUDE_EXPORT`;
//   - a pybind11-style `m.def(...)` binding for the entry point is
//     emitted so the host can call the Slang function.

//TEST:SIMPLE(filecheck=TORCH):-target torch -line-directive-mode none

[TorchEntryPoint]
float scaleValue(float x, float k)
{
    return x * k;
}

// The torch backend inlines slang-torch-prelude.h verbatim rather than
// emitting an `#include "slang-torch-prelude.h"` line, so the canonical
// marker for "prelude is present" is the prelude's own opening comment.
// TORCH-DAG: Prelude for PyTorch cpp binding.

// The entry point is declared with SLANG_PRELUDE_EXPORT on its own line.
// TORCH-DAG: {{^SLANG_PRELUDE_EXPORT$}}

// A pybind11-style binding registers the entry point under its source name.
// TORCH-DAG: m.def("scaleValue", &scaleValue, "scaleValue");
