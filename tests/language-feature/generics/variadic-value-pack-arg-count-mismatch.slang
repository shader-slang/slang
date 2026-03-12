//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct TwoPacks<let each A : int, let each B : int> {}

// 5 args cannot be evenly divided into 2 packs.
TwoPacks<1, 2, 3, 4, 5> bad;
/*CHECK:
^^^^^^^^ cannot specialize generic
^^^^^^^^ cannot specialize generic 'TwoPacks' with the provided arguments.
*/