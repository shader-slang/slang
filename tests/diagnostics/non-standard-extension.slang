//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -warnings-as-errors 30856

interface ICompatibleWith<T> {}
interface IBase {}
struct INT : IBase {}
struct FLOAT : IBase {}

extension<EX : IBase> EX : ICompatibleWith<INT> {}
extension<EY : IBase> EY : ICompatibleWith<FLOAT> {}

struct VECTOR<T> {}

extension<T, U : ICompatibleWith<T>> VECTOR<U> : ICompatibleWith<VECTOR<T>> {}
//CHECK:  ^ the extension is non-standard and may not work as intended because the generic parameter 'T' is not referenced by extension target type 'VECTOR<U>'
//CHECK:  ^ the extension is non-standard and may not work as intended because the generic parameter 'T' is not referenced by extension target type 'VECTOR<U>'

void main()
{}