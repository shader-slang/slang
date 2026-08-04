//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Hidden
{}

public struct Pair<X, Y>
{}

typealias T0 = Hidden;
typealias T1 = Pair<T0, T0>;
typealias T2 = Pair<T1, T1>;
typealias T3 = Pair<T2, T2>;
typealias T4 = Pair<T3, T3>;

// The shared inner type still contributes its lower visibility when visited only once.
public typealias Exposed = T4;
//CHECK:         ^^^^^^^ references less visible type
//CHECK:         ^^^^^^^ 'Exposed' references less visible type
