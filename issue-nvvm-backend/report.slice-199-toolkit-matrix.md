# Slice 199: Require the CUDA toolkit compilation and assembly matrix

## Motivation

The initial Linux NVVM checks produced useful local evidence, but no reusable gate required each
shader, optimization level, and architecture to compile and assemble. A successful command could
also leave an old artifact in place and accidentally make a later validation appear successful.

Consider this existing shader from `tests/cuda/nvvm-core-execution.slang`:

```slang
groupshared int sharedValues[64];

[CUDAKernel]
void computeMain(uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    int index = int(cudaThreadIdx().x);
    sharedValues[index] = index * 3 + 1;
    GroupMemoryBarrierWithGroupSync();
    destination[index] = sharedValues[63 - index];
}
```

The selected toolkit must accept the LLVM IR produced for this kernel, emit PTX for the requested
architecture, and assemble that PTX into a cubin. CUDA 13 rejects SM70, so silently retaining an SM70
default or silently substituting SM80 would invalidate the claimed matrix.

## Proposed Solution

Add `extras/validate-nvvm-toolkit.py` with explicit compiler, provider file, toolkit root, architecture,
and output arguments. Its fixed nine-shader inventory covers execution state and barriers, mixed
numeric operations, float surfaces, FP64, thread-local global context, FP16, matrices, helper
aggregates, and conventional resource bindings. Each shader runs at O0 and O3 unless the caller
explicitly selects one of those levels.

Every requested cell is mandatory. The runner records its compile command, assembly command, exit
codes, diagnostics, requested and actual PTX target, and artifacts. Missing tools, missing outputs,
wrong targets, rejected architectures, and command failures produce a failing result, never a skip.

A `workflow_dispatch` workflow builds Slang and the isolated LLVM 14 provider inside pinned NVIDIA
CUDA development containers. It then runs SM70/80/90 for CUDA 12.9 and SM80/90 for CUDA 13.4. These
jobs require no GPU, runner credentials, or external provider artifact.

## Change Summary

- `extras/validate-nvvm-toolkit.py` owns the shader inventory, strict execution gate, and JSON evidence.
- `.github/workflows/nvvm-toolkit-validation.yml` defines the two toolkit jobs, builds the provider
  through the slice-197 helper, and uploads evidence even when validation fails.
- This completed slice plan and report preserve the acceptance evidence and remaining CI limitation.
  Generated PTX, cubins, and detailed logs stay below `build/`.

## Concepts and Vocabulary

- **Provider:** the compiler-matched shared library that constructs LLVM 14 IR for direct NVVM
  compilation; it is distinct from the selected toolkit's libNVVM.
- **Required cell:** one shader, architecture, and optimization level. Its identity remains in the
  result inventory even if the compiler or toolkit is unavailable.
- **PTX and cubin:** the textual device program emitted by libNVVM and the assembled machine-code
  container produced by ptxas. Producing both is compile/assembly evidence, not execution evidence.

## Process Report

`parse_arguments` requires explicit tools and architectures and rejects duplicate or malformed
matrix inputs. `main` constructs the complete required-cell inventory before checking dependencies,
so a missing provider cannot reduce the denominator. `require_file` rejects missing or empty inputs
and outputs. `select_libnvvm` selects a concrete library inside the requested toolkit; `-nvvm-path`
then bypasses ambient library search. The provider path remains an exact file override, using the
slice-198 loader correction rather than recreating a directory workaround in the validator.

For the example above, `validate_cell` invokes `slangc -emit-cuda-via-nvvm` with that selected libNVVM,
entry point, optimization level, and capability. It deletes its named outputs before starting their
producers. After compilation, it checks the emitted `.target` and kernel entry, then invokes the
selected toolkit's `ptxas -arch=sm_...`. Only a successful assembly with a fresh, nonempty cubin can
mark the cell passed. `run` retains command failures, launch errors, and timeouts in structured
records; the top-level exit status requires all cells to pass.

The input-shape audit identified filesystem paths, compiler outputs, and requested architecture
identities as the data this layer owns. These are intentional external-tool contracts, not malformed
compiler IR. No new compiler helper, semantic fallback, or alternate AST/IR representation is
introduced. The compiler and provider remain responsible for their normal lowering and ABI checks.
`sha256` identifies the compiler executable, provider, toolkit libraries, each shader source, and
successful PTX/cubin artifacts in the evidence.

The final runner, using the corrected exact-file provider loader, passed all local cells:

| Local toolkit | Required architectures | Optimization levels | Compilation and assembly |
| ------------- | ---------------------- | ------------------- | ------------------------ |
| CUDA 12.9.2   | SM70, SM80, SM90       | O0, O3              | 54/54 passed             |
| CUDA 13.4.2   | SM80, SM90             | O0, O3              | 36/36 passed             |

The CUDA 12.9 components were extracted into an isolated directory from NVIDIA packages. They did
not replace the installed CUDA 13 toolkit. Reproduce the two local runs from the repository root:

```sh
python3 extras/validate-nvvm-toolkit.py \
    --slangc build/Debug/bin/slangc \
    --provider build/Debug/bin/libslang-llvm-nvvm.so \
    --cuda-root build/nvvm-slice199-cuda129/root/usr/local/cuda-12.9 \
    --expected-toolkit 12.9 --architectures 70 80 90 \
    --output build/nvvm-slice199-cuda129-validation

python3 extras/validate-nvvm-toolkit.py \
    --slangc build/Debug/bin/slangc \
    --provider build/Debug/bin/libslang-llvm-nvvm.so \
    --cuda-root /usr/local/cuda-13.4 \
    --expected-toolkit 13.4 --architectures 80 90 \
    --output build/nvvm-slice199
```

Negative checks selected SM80/O3, retaining all nine shader records. A missing provider, a missing
toolkit, and a mismatched expected toolkit each exited 1 with nine infrastructure failures. A fixture
compiler that exits successfully without writing PTX also failed all nine records; a pre-existing
PTX fixture was removed and could not satisfy the gate. Explicit SM70/O3 under CUDA 13.4 exited 1
with nine compile failures and libNVVM's unsupported `compute_70` diagnostic. None became skipped or
silently retargeted cases.

Python syntax, YAML parsing, repository formatting, and the repository-pinned actionlint 1.7.12 all
passed. NVIDIA's public [CUDA 12.9.1 tag metadata](https://hub.docker.com/v2/repositories/nvidia/cuda/tags/12.9.1-devel-ubuntu24.04) and [CUDA 13.4.1 tag metadata](https://hub.docker.com/v2/repositories/nvidia/cuda/tags/13.4.1-devel-ubuntu24.04) were checked before pinning the containers: the available CUDA
13.4 development image is 13.4.1, not an invented 13.4.2 tag. The workflow pins CUDA 12.9.1 and 13.4.1
images by digest. Those patch versions differ from the locally validated packages, and the workflow
has not been dispatched. The local evidence covers both toolkit families; a first remote run still
needs to verify the exact containers and hosted build environment. No GPU execution is claimed.
