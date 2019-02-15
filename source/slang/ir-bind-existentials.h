// ir-bind-existentials.h
#pragma once

namespace Slang
{

class DiagnosticSink;
struct IRModule;

void bindExistentialSlots(
    IRModule*       module,
    DiagnosticSink* sink);

}
