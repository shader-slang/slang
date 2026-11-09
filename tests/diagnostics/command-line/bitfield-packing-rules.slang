// The deprecated flag uses MSB-first packing, but warns once even when repeated.
// Diagnostic annotation checks are exhaustive: a second warning would be unannotated and fail.
//DIAGNOSTIC_TEST:SIMPLE(diag=LEGACY):-msvc-style-bitfield-packing -msvc-style-bitfield-packing -no-codegen
//LEGACY: `-msvc-style-bitfield-packing` is deprecated: it packs bitfields MSB-first, unlike MSVC on little-endian platforms. Use `-bitfield-packing-rules legacy-msb-first-msvc` for the same packing behavior, or `-bitfield-packing-rules msvc` for MSVC bitfield packing

// Reject an unknown named rule and show the accepted spellings.
//DIAGNOSTIC_TEST:SIMPLE(diag=INVALID):-bitfield-packing-rules not-a-rule -no-codegen
//INVALID: unknown value for option '-bitfield-packing-rules'. Valid values are 'default, msvc, legacy-msb-first-msvc'

// Reject the old and new options in either order; neither silently takes precedence.
//DIAGNOSTIC_TEST:SIMPLE(diag=LEGACY_FIRST):-msvc-style-bitfield-packing -bitfield-packing-rules msvc -no-codegen
//LEGACY_FIRST: is deprecated: it packs bitfields MSB-first, unlike MSVC
//LEGACY_FIRST: `-msvc-style-bitfield-packing` cannot be combined with `-bitfield-packing-rules`
//DIAGNOSTIC_TEST:SIMPLE(diag=NEW_FIRST):-bitfield-packing-rules msvc -msvc-style-bitfield-packing -no-codegen
//NEW_FIRST: is deprecated: it packs bitfields MSB-first, unlike MSVC
//NEW_FIRST: `-msvc-style-bitfield-packing` cannot be combined with `-bitfield-packing-rules`

// An explicit default rule is still an option selection and conflicts with the legacy flag.
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT_FIRST):-bitfield-packing-rules default -msvc-style-bitfield-packing -no-codegen
//DEFAULT_FIRST: is deprecated: it packs bitfields MSB-first, unlike MSVC
//DEFAULT_FIRST: `-msvc-style-bitfield-packing` cannot be combined with `-bitfield-packing-rules`

//DIAGNOSTIC_TEST:SIMPLE(diag=MISSING):-bitfield-packing-rules
//MISSING: expected an argument for command-line option '-bitfield-packing-rules'

struct Sample
{
    uint a : 4;
};
