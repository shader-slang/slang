// This translation unit deliberately contains no emitted code. Compiling it as C keeps the
// module boundary honest: the shared ABI header cannot silently acquire a C++-only dependency.

#include "compiler-core/slang-nvvm-ir-builder-api.h"

typedef SlangNVVMBuilderAPI SlangNVVMBuilderAPICCompileProbe;
typedef SlangNVVMBuilderFoundationAPI SlangNVVMBuilderFoundationAPICCompileProbe;
typedef SlangNVVMBuilderConstructionAPI SlangNVVMBuilderConstructionAPICCompileProbe;
typedef SlangNVVMBuilderValueOperationsAPI SlangNVVMBuilderValueOperationsAPICCompileProbe;
typedef SlangNVVMBuilderSurfaceOperationsAPI SlangNVVMBuilderSurfaceOperationsAPICCompileProbe;
