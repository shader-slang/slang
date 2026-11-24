//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[__AttributeUsage(_AttributeTargets.Function)]
struct uint8ProbeAttribute { uint8_t value; };
[__AttributeUsage(_AttributeTargets.Function)]
struct int8ProbeAttribute { int8_t value; };
[__AttributeUsage(_AttributeTargets.Function)]
struct uint16ProbeAttribute { uint16_t value; };
[__AttributeUsage(_AttributeTargets.Function)]
struct int16ProbeAttribute { int16_t value; };
[__AttributeUsage(_AttributeTargets.Function)]
struct uint32ProbeAttribute { uint value; };
[__AttributeUsage(_AttributeTargets.Function)]
struct int32ProbeAttribute { int value; };
[__AttributeUsage(_AttributeTargets.Function)]
struct uint64ProbeAttribute { uint64_t value; };
[__AttributeUsage(_AttributeTargets.Function)]
struct int64ProbeAttribute { int64_t value; };

RWStructuredBuffer<int> outBuf;

// these don't trigger diagnostics
[uint32Probe(operator/(0x80000000u, 0xFFFFFFFFu))]
[uint64Probe(operator/(0x8000000000000000ull, 0xFFFFFFFFFFFFFFFFull))]
[uint8Probe(uint8_t(0x80u) / uint16_t(0xFFu))]
[uint16Probe(uint16_t(0x8000u) / uint16_t(0xFFFFu))]
[uint32Probe(0x80000000u / 0xFFFFFFFFu)]
[uint64Probe(0x8000000000000000ull / 0xFFFFFFFFFFFFFFFFull)]

// these do
[int8Probe(int8_t(0x80)     / int8_t(-1))]
//CHECK:                    ^ target-defined value
//CHECK:                    ^ the value of expression int8_t(-128) / int8_t(-1) is target-defined.
//CHECK:                    ^ could not extract value from integer constant

[int16Probe(int16_t(0x8000) / int16_t(-1))]
//CHECK:                    ^ target-defined value
//CHECK:                    ^ the value of expression int16_t(-32768) / int16_t(-1) is target-defined.
//CHECK:                    ^ could not extract value from integer constant

[int32Probe(int(0x80000000) / -1)]
//CHECK:                    ^ target-defined value
//CHECK:                    ^ the value of expression -2147483648 / -1 is target-defined.
//CHECK:                    ^ could not extract value from integer constant

[int64Probe(int64_t(0x8000000000000000) / -1)]
//CHECK:                                ^ target-defined value
//CHECK:                                ^ the value of expression -9223372036854775808LL / -1LL is target-defined.
//CHECK:                                ^ could not extract value from integer constant

// double-check that calling with the operator also triggers diagnostics
[int32Probe(operator/(int(0x80000000), -1))]
//CHECK:    ^^^^^^^^ target-defined value
//CHECK:    ^^^^^^^^ the value of expression -2147483648 / -1 is target-defined.
//CHECK:             ^ expression does not evaluate to a compile-time constant

[int64Probe(operator/(int64_t(0x8000000000000000), -1))]
//CHECK:    ^^^^^^^^ target-defined value
//CHECK:    ^^^^^^^^ the value of expression -9223372036854775808LL / -1LL is target-defined.
//CHECK:             ^ expression does not evaluate to a compile-time constant

[numthreads(1,1,1)]
void computeMain()
{
}
