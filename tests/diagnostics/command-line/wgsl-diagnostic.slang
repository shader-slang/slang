// wgsl-diagnostic.slang

// `-wgsl-diagnostic <severity>:<rule>`: the severity must be error, warning, info or off, and the
// rule a WGSL identifier or two identifiers separated by a period.

//DIAGNOSTIC_TEST:SIMPLE(diag=BADSEV):-target wgsl -wgsl-diagnostic warn:subgroup_uniformity
//BADSEV: unknown value for option '-wgsl-diagnostic'. Valid values are '<error|warning|info|off>:<rule>[.<rule>]'

//DIAGNOSTIC_TEST:SIMPLE(diag=NOCOLON):-target wgsl -wgsl-diagnostic off
//NOCOLON: unknown value for option '-wgsl-diagnostic'. Valid values are '<error|warning|info|off>:<rule>[.<rule>]'

//DIAGNOSTIC_TEST:SIMPLE(diag=NORULE):-target wgsl -wgsl-diagnostic off:
//NORULE: unknown value for option '-wgsl-diagnostic'. Valid values are '<error|warning|info|off>:<rule>[.<rule>]'

//DIAGNOSTIC_TEST:SIMPLE(diag=BADRULE):-target wgsl -wgsl-diagnostic off:not-a-rule!
//BADRULE: unknown value for option '-wgsl-diagnostic'. Valid values are '<error|warning|info|off>:<rule>[.<rule>]'

//DIAGNOSTIC_TEST:SIMPLE(diag=SPACE):-target wgsl -wgsl-diagnostic "off:subgroup uniformity"
//SPACE: unknown value for option '-wgsl-diagnostic'. Valid values are '<error|warning|info|off>:<rule>[.<rule>]'
