del *.dxil
del *.slang-module
slangc.exe hit.slang -stage closesthit -entry closesthit -target dxil -o monolithic.dxil
slangc.exe environment.slang -o environment.slang-module
slangc.exe hit.slang -stage closesthit -entry closesthit -target dxil -o precompiled.dxil
