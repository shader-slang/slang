#ifndef SLANG_IR_SPECIALIZE_MATRIX_LAYOUT_H
#define SLANG_IR_SPECIALIZE_MATRIX_LAYOUT_H

namespace Slang
{
    struct IRModule;
    class TargetRequest;

    // Repalce all matrix types whose layout is not specified with the default layout
    // of the target request.
    //
    void specializeMatrixLayout(TargetRequest* target, IRModule* module);

}

#endif
