// Each module is valid by itself. Composition closes the associated-type cycle.
//TEST:COMPILE: tests/serialization/link-time-associated-cycle/cycle_contract.slang -o tests/serialization/link-time-associated-cycle/cycle_contract.slang-module
//TEST:COMPILE: tests/serialization/link-time-associated-cycle/cycle_a.slang -o tests/serialization/link-time-associated-cycle/cycle_a.slang-module
//TEST:COMPILE: tests/serialization/link-time-associated-cycle/cycle_b.slang -o tests/serialization/link-time-associated-cycle/cycle_b.slang-module
//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): tests/serialization/link-time-associated-cycle/cycle_contract.slang-module tests/serialization/link-time-associated-cycle/cycle_a.slang-module tests/serialization/link-time-associated-cycle/cycle_b.slang-module -target spirv

/*CHECK:
cyclic reference 'Assoc'
*/
