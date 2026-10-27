//TEST:LANG_SERVER(filecheck=CHECK):

interface IBox<T> {}

//HOVER:6,17
void accept(dyn IBox<int> value) {}

//CHECK: IBox
