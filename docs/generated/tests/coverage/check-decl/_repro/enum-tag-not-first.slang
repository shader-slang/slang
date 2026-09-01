// Repro for finding check-decl-enum-tag-not-first-internal-error.
// NOT a test (lives in _repro/, outside the bundle's *.slang glob).
// An enum base list with an interface before the integer tag type emits
// E30821 (correctly) but then aborts with an internal error E99997.
interface IThing {}
enum E : IThing, int { A }
[numthreads(1, 1, 1)] void main() {}
