// slang-ast-reflect.h

#ifndef SLANG_AST_REFLECT_H
#define SLANG_AST_REFLECT_H

//#include "slang-syntax.h"

//#include "slang-ast-generated.h"



#define SLANG_CLASS_REFLECT(NAME) \
    public:     \
    static const ASTNodeType kType = ASTNodeType::NAME; \
    typedef ASTNodeSuper::NAME##_Super Super; \
    SLANG_FORCE_INLINE static bool isDerivedFrom(ASTNodeType type) { return int(type) >= int(kType) && int(ASTNodeLast::NAME) <= int(type); } \
    static const ReflectClassInfo kReflectClassInfo; \
    virtual const ReflectClassInfo& getClassInfo() const { return kReflectClassInfo; }

#define SLANG_REFLECT_BASE_CLASS(NAME)

#define SLANG_ABSTRACT_CLASS(NAME) SLANG_CLASS_REFLECT(NAME)
#define SLANG_CLASS(NAME) SLANG_CLASS_REFLECT(NAME) \
     virtual void accept(NAME::Visitor* visitor, void* extra) override;  

#define SLANG_NON_VISITOR_ABSTRACT_CLASS(x) SLANG_CLASS_REFLECT(x)
#define SLANG_NON_VISITOR_CLASS(x) SLANG_CLASS_REFLECT(x)

#define SLANG_VISITOR_DISPATCH(NAME, SUPER, STYLE, NODE_TYPE, PARAM) \
    virtual void dispatch_##NAME(NAME* obj, void* extra) = 0;

#endif // SLANG_AST_REFLECT_H
