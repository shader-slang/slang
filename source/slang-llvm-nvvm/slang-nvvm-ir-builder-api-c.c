// This translation unit deliberately contains no emitted code. Compiling it as C keeps the
// module boundary honest: the shared ABI header cannot silently acquire a C++-only dependency.

#include "compiler-core/slang-nvvm-ir-builder-api.h"

typedef SlangNVVMBuilderAPI_V1 SlangNVVMBuilderAPIV1CCompileProbe;
typedef SlangNVVMBuilderAPI_V2 SlangNVVMBuilderAPIV2CCompileProbe;
typedef char SlangNVVMBuilderAPIV2MinimumSizeCCompileProbe
    [(SLANG_NVVM_BUILDER_API_V2_MIN_SIZE <= sizeof(SlangNVVMBuilderAPI_V2)) ? 1 : -1];
typedef char SlangNVVMBuilderAPIV2ScalarMinimumSizeCCompileProbe
    [(SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE <= sizeof(SlangNVVMBuilderAPI_V2)) ? 1 : -1];
typedef char SlangNVVMBuilderAPIV2CapabilityOrderCCompileProbe
    [(SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE > SLANG_NVVM_BUILDER_API_V2_MIN_SIZE) ? 1 : -1];
typedef char SlangNVVMBuilderAPIV2ScalarControlFlowMinimumSizeCCompileProbe
    [(SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE <= sizeof(SlangNVVMBuilderAPI_V2))
         ? 1
         : -1];
typedef char SlangNVVMBuilderAPIV2ScalarControlFlowCapabilityOrderCCompileProbe
    [(SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE >
      SLANG_NVVM_BUILDER_API_V2_SCALAR_MIN_SIZE)
         ? 1
         : -1];
typedef char SlangNVVMBuilderAPIV2ScalarSSAMinimumSizeCCompileProbe
    [(SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE <= sizeof(SlangNVVMBuilderAPI_V2)) ? 1 : -1];
typedef char SlangNVVMBuilderAPIV2ScalarSSACapabilityOrderCCompileProbe
    [(SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE >
      SLANG_NVVM_BUILDER_API_V2_SCALAR_CONTROL_FLOW_MIN_SIZE)
         ? 1
         : -1];
typedef char SlangNVVMBuilderAPIV2ScalarFunctionMinimumSizeCCompileProbe
    [(SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE <= sizeof(SlangNVVMBuilderAPI_V2)) ? 1
                                                                                            : -1];
typedef char SlangNVVMBuilderAPIV2ScalarFunctionCapabilityOrderCCompileProbe
    [(SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE >
      SLANG_NVVM_BUILDER_API_V2_SCALAR_SSA_MIN_SIZE)
         ? 1
         : -1];
typedef char SlangNVVMBuilderAPIV2ScalarPointerArithmeticMinimumSizeCCompileProbe
    [(SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE <=
      sizeof(SlangNVVMBuilderAPI_V2))
         ? 1
         : -1];
typedef char SlangNVVMBuilderAPIV2ScalarPointerArithmeticCapabilityOrderCCompileProbe
    [(SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE >
      SLANG_NVVM_BUILDER_API_V2_SCALAR_FUNCTION_MIN_SIZE)
         ? 1
         : -1];
typedef char SlangNVVMBuilderAPIV2ScalarArrayMinimumSizeCCompileProbe
    [(SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE <= sizeof(SlangNVVMBuilderAPI_V2)) ? 1 : -1];
typedef char SlangNVVMBuilderAPIV2ScalarArrayCapabilityOrderCCompileProbe
    [(SLANG_NVVM_BUILDER_API_V2_SCALAR_ARRAY_MIN_SIZE >
      SLANG_NVVM_BUILDER_API_V2_SCALAR_POINTER_ARITHMETIC_MIN_SIZE)
         ? 1
         : -1];
