// slang-ir-specialize.h
#pragma once

namespace Slang
{
struct IRModule;
struct IRInst;
struct IRSpecialize;
class DiagnosticSink;
class TargetProgram;

struct SpecializationOptions
{
    // Option that allows specializeModule to generate dynamic-dispatch code
    // wherever possible to open up more specialization opportunities.
    //
    bool lowerWitnessLookups = false;

    // Option to report dynamic dispatch sites.
    bool reportDynamicDispatchSites = false;
};

/// Specialize generic and interface-based code to use concrete types.
bool specializeModule(
    IRModule* module,
    TargetProgram* target,
    DiagnosticSink* sink,
    SpecializationOptions options);

void finalizeSpecialization(IRModule* module);

IRInst* specializeGeneric(IRSpecialize* specInst);

// Specialize a generic with one or more arguments that are collections rather
// than single concrete values.
//
IRInst* specializeDynamicGeneric(IRSpecialize* specializeInst);

} // namespace Slang
