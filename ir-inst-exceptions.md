# IR Instruction Exceptions

This document lists IR instruction types that cannot use the FIDDLE template system for automatic builder method generation due to special implementation requirements.

## Complex Logic/Special Cases

- **BindExistentialsType**: Has complex variable-operand logic with special case handling for interface types
- **BoundInterfaceType**: Has conditional logic that skips wrapping for __Dynamic types
- **AfterRawPointerTypeBase**: Boundary marker, not used in actual code generation
- **BackwardDiffIntermediateContextType**: Has null->void conversion logic
- **AttributedType**: Uses dynamic operand list
- **RefParamType**: Has high-level helper that takes AddressSpace parameters (FIDDLE generates basic version)
- **BorrowInParamType**: Has high-level helper that takes AddressSpace parameters (FIDDLE generates basic version)