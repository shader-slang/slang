//TEST:SIMPLE(filecheck=CHECK): -warnings-as-errors 30856

interface ICompatibleWith<T> {}
interface IBase {}
struct INT : IBase {}
struct FLOAT : IBase {}

extension<EX : IBase> EX : ICompatibleWith<INT> {}
extension<EY : IBase> EY : ICompatibleWith<FLOAT> {}

struct VECTOR<T> {}

// CHECK: ([[# @LINE+1]]): error 30856: the extension is non-standard
extension<T, U : ICompatibleWith<T>> VECTOR<U> : ICompatibleWith<VECTOR<T>> {}

void main()
{}