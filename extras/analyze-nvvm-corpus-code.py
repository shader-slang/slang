#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Strict static instruction inventories; these are not dynamic execution counts."""
import collections
import difflib
import hashlib
import re
import struct

IDENT = r'[A-Za-z_$%][A-Za-z0-9_.$%]*'
TOKEN = re.compile(r'"(?:\\.|[^"\\])*"|' + IDENT + r'|0[xX][0-9a-fA-F]+|[0-9]+(?:\.[0-9]+)?|[^\s]')


def strip_comments(text):
    """Keep quoted strings intact while removing line and block comments."""
    return re.sub(r'"(?:\\.|[^"\\])*"|//[^\n]*|/\*.*?\*/',
                  lambda m: m[0] if m[0].startswith('"') else '\n' * m[0].count('\n') + ' ',
                  text, flags=re.S)


def closing(text, start, left='(', right=')'):
    depth = 0
    for index in range(start, len(text)):
        if text[index] == left:
            depth += 1
        elif text[index] == right:
            depth -= 1
            if depth == 0:
                return index
    raise ValueError('unclosed ' + left)


def functions(text):
    """Extract function definitions, excluding prototypes and preserving nested local scopes."""
    text = strip_comments(text)
    rows = []
    cursor = 0
    searchable = re.sub(r'"(?:\\.|[^"\\])*"', lambda m: ' ' * len(m[0]), text)
    while match := re.search(r'\.(entry|func)\b', searchable[cursor:]):
        start = cursor + match.start()
        kind = match[1]
        pos = cursor + match.end()
        while text[pos].isspace(): pos += 1
        if text[pos] == '(':
            pos = closing(searchable, pos) + 1
            while text[pos].isspace(): pos += 1
        name = re.match(IDENT, text[pos:])
        if not name: raise ValueError('function name missing')
        name = name[0]
        arg_start = text.find('(', pos + len(name))
        if arg_start < 0: raise ValueError('function parameters missing')
        end_args = closing(searchable, arg_start)
        end_header = re.search(r'[;{]', searchable[end_args + 1:])
        if not end_header: raise ValueError('function terminator missing')
        end_header_pos = end_args + 1 + end_header.start()
        if end_header[0] == ';':
            cursor = end_header_pos + 1
            continue
        end_body = closing(searchable, end_header_pos, '{', '}')
        rows.append(dict(name=name, kind=kind, header=text[start:end_header_pos],
                         body=text[end_header_pos + 1:end_body]))
        cursor = end_body + 1
    if not rows: raise ValueError('no PTX function definitions')
    if len({r['name'] for r in rows}) != len(rows): raise ValueError('duplicate function definitions')
    return rows


def statements(body):
    """Split semicolon-terminated instructions, retaining labels and skipping line directives.

    PTX uses braces both for local scopes and vector operands. An opening brace before a statement
    starts a scope; a brace after an opcode belongs to that instruction and must stay in its operands.
    """
    body = re.sub(r'^\s*\.(?:loc|file)\b[^\n]*', '', body, flags=re.M)
    result = []
    labels = []
    pending = ''
    for token in TOKEN.findall(body):
        if token == ':' and re.fullmatch(IDENT, pending.strip()):
            labels.append((pending.strip(), len(result)))
            pending = ''
        elif token in ('{', '}') and not pending.strip():
            continue
        elif token == ';':
            if pending.strip(): result.append(pending.strip())
            pending = ''
        else:
            pending += token + ' '
    if pending.strip(): raise ValueError('unterminated PTX statement: ' + pending[:100])
    return result, labels


def parse_function(row, names):
    stmts, statement_labels = statements(row['body'])
    instructions = []
    instruction_positions = []
    declarations = []
    virtual = collections.Counter()
    register_names = []
    register_patterns = []
    for reg in re.finditer(r'\.reg\s+\.(\w+)\s+([^;]+);', row['body']):
        for name, extent in re.findall('(' + IDENT + r')(?:\s*<\s*(\d+)\s*>)?', reg[2]):
            virtual[reg[1]] += int(extent or 1)
            register_names.append(name)
            register_patterns.append((name, int(extent) if extent else None))
    for index, statement in enumerate(stmts):
        if statement.startswith('.'):
            declarations.append(statement)
            continue
        match = re.match(r'(?:@\s*!?\s*' + IDENT + r'\s+)?([A-Za-z][\w.]*)\b', statement)
        if not match: raise ValueError('unrecognized instruction: ' + statement[:120])
        opcode = match[1]
        instructions.append(dict(text=statement, opcode=opcode))
        instruction_positions.append(index)
    if len({name for name,_ in statement_labels}) != len(statement_labels):
        raise ValueError('duplicate PTX labels require lexical-scope analysis')
    labels = {name: sum(p < position for p in instruction_positions)
              for name, position in statement_labels}
    calls = []
    unresolved = []
    for inst in instructions:
        if inst['opcode'].split('.')[0] != 'call': continue
        targets = [name for name in names if re.search(r'(?<![\w.$%])' + re.escape(name) + r'(?![\w.$%])', inst['text'])]
        if len(targets) == 1: calls.append(targets[0])
        else: unresolved.append(inst['text'])
    return {**row, 'instructions': instructions, 'labels': labels, 'declarations': declarations,
            'virtual_registers': dict(virtual), 'register_patterns': register_patterns,
            'normalization_supported': len(set(register_names)) == len(register_names),
            'calls': calls, 'unresolved_calls': unresolved}


def normalized_instructions(function, function_names):
    """Rename generated symbols consistently; preserve constants, modifiers and operand reuse.

    This is a comparison of instruction streams, not semantic equivalence. Headers, global data and
    register declarations are not included, so exact normalized equality must retain that label.
    """
    symbols = dict(function_names)
    for index, (name, position) in enumerate(function['labels'].items()):
        symbols[name] = 'LABEL_AT_' + str(position)
    params = re.findall(r'\.param\b\s+(?:\.align\s+\d+\s+)?\.\w+\s+(' + IDENT + ')', function['header'])
    for index, name in enumerate(params): symbols[name] = 'PARAM_' + str(index)
    registers = {}
    output = []
    for inst in function['instructions']:
        tokens = TOKEN.findall(inst['text'])
        mapped = []
        for token in tokens:
            if token in symbols: token = symbols[token]
            elif any(token == name if extent is None else
                     token.startswith(name) and token[len(name):].isdigit() and
                     int(token[len(name):]) < extent
                     for name,extent in function['register_patterns']):
                # Special registers such as %tid.x and %laneid retain their architectural meaning.
                token = registers.setdefault(token, 'REG_' + str(len(registers)))
            mapped.append(token)
        output.append(' '.join(mapped))
    return output


def instruction_metrics(instructions):
    opcodes = collections.Counter(i['opcode'] for i in instructions)
    base = collections.Counter()
    for opcode, count in opcodes.items(): base[opcode.split('.')[0]] += count
    spaces = {}
    for action in ['ld', 'st', 'atom', 'red']:
        for space in ['local', 'global', 'shared', 'param', 'const']:
            spaces[action + '_' + space] = sum(n for op, n in opcodes.items()
                                              if op.split('.')[0] == action and space in op.split('.'))
    return dict(instructions=len(instructions), opcode_counts=dict(sorted(opcodes.items())),
                families=dict(sorted(base.items())), memory=spaces,
                branches=sum(n for op,n in opcodes.items() if op.split('.')[0] in ('bra','brx')),
                calls=base['call'], shuffles=base['shfl'], barriers=base['bar'] + base['barrier'])


def parse_ptx(text, entry):
    raw = functions(text)
    names = [r['name'] for r in raw]
    parsed = {r['name']: parse_function(r, names) for r in raw}
    if entry not in parsed or parsed[entry]['kind'] != 'entry': raise ValueError('selected PTX entry missing')
    reachable = [entry]
    for name in reachable:
        for target in parsed[name]['calls']:
            if target not in reachable: reachable.append(target)
    mapped = {name: 'FUNCTION_' + str(i) for i,name in enumerate(reachable)}
    normalized = ['FUNCTION ' + mapped[name] + '\n' + '\n'.join(normalized_instructions(parsed[name], mapped))
                  for name in reachable]
    per_function = {}
    for name, row in parsed.items():
        per_function[name] = dict(kind=row['kind'], virtual_registers=row['virtual_registers'],
                                  normalization_supported=row['normalization_supported'],
                                  direct_calls=row['calls'], unresolved_calls=row['unresolved_calls'],
                                  **instruction_metrics(row['instructions']))
    all_instructions = [i for name in reachable for i in parsed[name]['instructions']]
    return dict(entry=entry, entries=[r['name'] for r in raw if r['kind']=='entry'],
                module_functions=len(raw), reachable_functions=reachable,
                unresolved_calls=[i for n in reachable for i in parsed[n]['unresolved_calls']],
                per_function=per_function, entry_metrics=per_function[entry],
                reachable_metrics=instruction_metrics(all_instructions),
                normalized_stream=normalized,
                normalized_stream_sha256=(hashlib.sha256('\n'.join(normalized).encode()).hexdigest()
                                          if all(parsed[name]['normalization_supported'] for name in reachable) else None),
                opcode_sequence=[i['opcode'] for i in all_instructions])


def parse_sass(text):
    result = {}
    current = None
    for line in text.splitlines():
        function = re.match(r'\s*Function\s*:\s*(\S+)', line)
        if function:
            current = function[1]
            if current in result: raise ValueError('duplicate SASS function')
            result[current] = []
            continue
        instruction = re.match(r'\s*/\*([0-9a-fA-F]+)\*/\s+([^/]+?);', line)
        if re.match(r'\s*/\*[0-9a-fA-F]+\*/', line) and not instruction:
            raise ValueError('malformed offset-prefixed SASS instruction')
        if instruction:
            if current is None: raise ValueError('SASS instruction outside function')
            if int(instruction[1],16) != 16 * len(result[current]):
                raise ValueError('SASS offsets must uniquely cover consecutive 16-byte instructions')
            content = instruction[2].strip()
            opcode = re.match(r'(?:@!?\w+\s+)?([A-Z][A-Z0-9_.]*)\b', content)
            if not opcode: raise ValueError('unknown SASS instruction')
            result[current].append(dict(offset=int(instruction[1],16), text=content, opcode=opcode[1]))
    if not result or not all(result.values()): raise ValueError('missing SASS instructions')
    return {name: dict(instructions=len(rows), non_nop_instructions=sum(i['opcode']!='NOP' for i in rows),
                       opcode_counts=dict(collections.Counter(i['opcode'] for i in rows)),
                       instruction_text=[i['text'] for i in rows],
                       normalized_text_sha256=hashlib.sha256('\n'.join(i['text'] for i in rows).encode()).hexdigest())
            for name,rows in result.items()}


def executable_sections(data):
    """Read actual executable bytes from a little-endian ELF64 cubin, including alignment padding."""
    if data[:6] != b'\x7fELF\x02\x01': raise ValueError('expected little-endian ELF64')
    header=struct.unpack_from('<16sHHIQQQIHHHHHH',data)
    offset,size,count,strings=header[6],header[11],header[12],header[13]
    if size!=64 or not count or strings>=count: raise ValueError('unsupported ELF section table')
    rows=[struct.unpack_from('<IIQQQQIIQQ',data,offset+i*size) for i in range(count)]
    sr=rows[strings];names=data[sr[4]:sr[4]+sr[5]]
    output={}
    for row in rows:
        if not row[2]&4: continue
        name=names[row[0]:].split(b'\0',1)[0].decode()
        code=data[row[4]:row[4]+row[5]]
        if len(code)!=row[5]: raise ValueError('truncated executable section')
        output[name]=dict(bytes=len(code),sha256=hashlib.sha256(code).hexdigest())
    if not output: raise ValueError('no executable sections')
    return output


def validate_sass_coverage(sass, sections):
    """Require every disassembled function to cover its complete SM80/89 text section."""
    if {'.text.'+name for name in sass} != set(sections):
        raise ValueError('SASS function inventory differs from executable sections')
    for name,row in sass.items():
        if row['instructions'] * 16 != sections['.text.'+name]['bytes']:
            raise ValueError('SASS instructions do not cover executable section bytes')


def opcode_similarity(left, right):
    """Bounded heuristic over opcode order; operands are deliberately not an equivalence proof."""
    # SequenceMatcher with autojunk disabled is quadratic on giant repeated programs. The default
    # heuristic remains conservative and is labeled explicitly; exact stream hashes are separate.
    return difflib.SequenceMatcher(None,left,right,autojunk=True).ratio()


def read(path):
    import json
    return json.loads(path.read_text())


def save(path, value):
    import json
    path.write_text(json.dumps(value, indent=2, allow_nan=False) + '\n')


def reference(path):
    return dict(path=str(path.resolve()), sha256=hashlib.sha256(path.read_bytes()).hexdigest())


def check_reference(ref):
    from pathlib import Path
    path = Path(ref['path'])
    if reference(path)['sha256'] != ref['sha256']: raise ValueError('artifact identity changed: ' + str(path))
    return path


def validate_cell_binding(cell, declared):
    """Reject a valid PTX/output artifact borrowed from a different case or mode."""
    from pathlib import Path
    for key in ('id', 'mode', 'corpus', 'source', 'generated', 'dump', 'accepted_outcome'):
        if cell.get(key) != declared.get(key):
            raise ValueError('capture cell differs from manifest: ' + key)
    if cell.get('ptx') and Path(cell['ptx']['path']).parent != Path(declared['dump']):
        raise ValueError('PTX lies outside its declared cell directory')
    if cell.get('actual_output') and cell['actual_output']['path'] != declared['generated'] + '.actual.txt':
        raise ValueError('reference output belongs to another cell')


def validate_capture(capture, manifest, repo, nr):
    """Revalidate identities, authoritative logs and classifications before consuming the capture."""
    import importlib.util
    from pathlib import Path
    def load(name,path):
        spec=importlib.util.spec_from_file_location(name,path)
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module
    collector=load('analysis_collector',repo/'extras/capture-nvvm-corpus-code.py')
    runtime=load('analysis_runtime',repo/'extras/measure-nvvm-corpus-runtime.py')
    census=load('analysis_census',repo/'issue-nvvm-backend/run-compute-census.py')
    expected=manifest['cells']
    if [(c['id'],c['mode']) for c in capture['cells']] != [(c['id'],c['mode']) for c in expected]:
        raise ValueError('capture inventory mismatch')
    if len({(c['id'],c['mode']) for c in expected})!=len(expected):raise ValueError('duplicate capture cell')
    check_reference(manifest['runtime_manifest']);check_reference(manifest['provenance'])
    for path,digest in manifest['mirror_input_sha256'].items():check_reference(dict(path=path,sha256=digest))
    batches=[]
    for mode in manifest['modes']:
        group=[c for c in expected if c['mode']==mode]
        batches.extend(group[start:start+manifest['batch_size']] for start in range(0,len(group),manifest['batch_size']))
    if len(batches)!=len(capture['batches']):raise ValueError('batch inventory mismatch')
    observed={}
    for rows,process in zip(batches,capture['batches']):
        names=[str(Path(c['generated']).relative_to(repo))+' (cuda)' for c in rows]
        path=check_reference(dict(path=process['log'],sha256=process['log_sha256']))
        statuses=runtime.parse_batch(path.read_text(errors='replace'),names,process,census)
        for c,name in zip(rows,names):observed[c['id'],c['mode']]=(statuses[name],str(path))
    for cell,declared in zip(capture['cells'],expected):
        validate_cell_binding(cell,declared)
        oracle,log=observed[cell['id'],cell['mode']]
        if cell['oracle']!=oracle or cell['log']!=log:raise ValueError('oracle/log binding mismatch')
        for artifact in cell['artifacts']:
            path=check_reference(artifact)
            if path.parent!=Path(declared['dump']) or path.stat().st_size!=artifact['bytes']:raise ValueError('artifact inventory binding mismatch')
        derived=collector.inspect_ptx(cell['artifacts'])
        if derived!=cell['capture']:raise ValueError('capture classification changed')
        status=collector.qualification(oracle,declared['accepted_outcome']['classification'],derived)
        if cell.get('actual_output'):check_reference(cell['actual_output'])
        elif oracle=='passed':status='missing-oracle-output'
        if status!=cell['status']:raise ValueError('qualification classification changed')
        if derived['status']=='captured':
            if cell['ptx']['path']!=derived['ptx'] or cell['entry']!=derived['entry']:raise ValueError('PTX/entry binding mismatch')
            check_reference(cell['ptx'])


def analyze_capture(args):
    """Assemble only original-oracle-qualified runtime captures; preserve every failed analysis."""
    import importlib.util
    import os
    from pathlib import Path
    repo=Path(__file__).resolve().parents[1]
    spec=importlib.util.spec_from_file_location('code_results',repo/'issue-nvvm-backend/nvvm-results.py')
    nr=importlib.util.module_from_spec(spec);spec.loader.exec_module(nr)
    capture=read(args.capture)
    if capture['status'] not in ('completed','completed-with-explicit-exclusions'):
        raise ValueError('capture has not completed qualification')
    manifest=read(check_reference(capture['manifest']))
    validate_capture(capture,manifest,repo,nr)
    environment=os.environ.copy()
    environment['PATH']=str(args.nvdisasm.resolve().parent)+os.pathsep+environment.get('PATH','')
    args.output.mkdir(parents=True,exist_ok=False)
    tools={name:reference(getattr(args,name)) for name in ('ptxas','cuobjdump','nvdisasm')}
    versions={name:nr.run([getattr(args,name),'--version'],args.output/(name+'-version.log'),environment,30)
              for name in tools}
    for row in versions.values():nr.require_success(row)
    report=dict(schema=1,kind='corpus-code-quality',status='running',capture=reference(args.capture),
                manifest=capture['manifest'],tools=tools,versions=versions,
                analyzer=reference(Path(__file__)),
                analysis_helpers={name:reference(repo/name) for name in ['issue-nvvm-backend/nvvm-results.py','extras/capture-nvvm-corpus-code.py','extras/measure-nvvm-corpus-runtime.py','issue-nvvm-backend/run-compute-census.py']},
                architecture=args.architecture,
                scope='Fresh runtime-route PTX; offline ptxas assembly, not recorded driver-JIT machine code.',cells=[])
    save(args.output/'results.json',report)
    for index,cell in enumerate(capture['cells']):
        row={k:cell[k] for k in ('id','corpus','source','mode','oracle','status')}
        row['capture_status']=row.pop('status')
        if cell['status']!='qualified':
            row['status']='capture-excluded';report['cells'].append(row);continue
        directory=args.output/(cell['mode']+'-'+hashlib.sha256(cell['id'].encode()).hexdigest()[:20])
        directory.mkdir()
        try:
            path=check_reference(cell['ptx']);row['ptx']=cell['ptx'];row['entry']=cell['entry']
            # Both the compiler dump and its retained intermediate inventory must bind this exact cell.
            if not any(a['path']==str(path) and a['sha256']==cell['ptx']['sha256'] for a in cell['artifacts']):
                raise ValueError('PTX absent from capture artifact inventory')
            parsed=parse_ptx(path.read_text(),cell['entry'])
            save(directory/'ptx.json',parsed)
            (directory/'normalized-ptx.txt').write_text('\n\n'.join(parsed['normalized_stream'])+'\n')
            cubin=directory/'code.cubin'
            assembly=nr.run([args.ptxas,'-v',f'-arch=sm_{args.architecture}',path,'-o',cubin],directory/'ptxas.log',environment,120)
            row['assembly']=assembly;nr.require_success(assembly)
            resources=nr.parse_resources(Path(assembly['log']).read_text())
            entry_resources=[r for r in resources if r['function']==cell['entry']]
            if len(entry_resources)!=1 or entry_resources[0]['registers'] is None:
                raise ValueError('missing unique entry register allocation')
            sass_process=nr.run([args.cuobjdump,'--dump-sass',cubin],directory/'sass.log',environment,120)
            row['disassembly']=sass_process;nr.require_success(sass_process)
            sass=parse_sass(Path(sass_process['log']).read_text());sections=executable_sections(cubin.read_bytes())
            validate_sass_coverage(sass,sections)
            if cell['entry'] not in sass: raise ValueError('missing SASS entry')
            section=sections.get('.text.'+cell['entry'])
            if not section: raise ValueError('missing executable entry section')
            save(directory/'sass.json',sass)
            row.update(status='analyzed',resources=resources,entry_resources=entry_resources[0],
                       ptx_metrics={k:v for k,v in parsed.items() if k not in ('normalized_stream','opcode_sequence')},
                       sass_metrics={name:{k:v for k,v in value.items() if k!='instruction_text'} for name,value in sass.items()},
                       executable_sections=sections,
                       artifacts={name:reference(directory/name) for name in ['ptx.json','normalized-ptx.txt','code.cubin','ptxas.log','sass.log','sass.json']})
        except (ValueError,OSError,KeyError,struct.error) as error:
            row.update(status='analysis-failed',error=str(error))
        report['cells'].append(row)
        if index%32==0:save(args.output/'results.json',report);print(f'Analyzed {index+1}/{len(capture["cells"])}',flush=True)
    for name,ref in tools.items():check_reference(ref)
    check_reference(report['analyzer']);check_reference(report['capture'])
    report['counts']=dict(collections.Counter(r['status'] for r in report['cells']))
    report['status']='completed-with-analysis-failures' if report['counts'].get('analysis-failed') else 'completed'
    save(args.output/'results.json',report)
    return report


def load_runtime_summary(args, manifest):
    """Join only the independently accepted timing manifest/summary pinned by focused evidence."""
    from pathlib import Path
    evidence=read(args.runtime_evidence)
    if evidence.get('status')!='accepted-focused-evidence':raise ValueError('runtime evidence is not accepted')
    feature=evidence['features']['corpus-dispatch-performance']
    if not feature['status'].startswith('accepted'):raise ValueError('runtime feature is not accepted')
    expected_manifest=feature['raw']['collection/manifest.json']
    expected_summary=feature['raw']['collection/summary.json']
    if reference(check_reference(expected_manifest))!=reference(check_reference(manifest['runtime_manifest'])):
        raise ValueError('capture and accepted timing manifests differ')
    if args.runtime_summary.resolve()!=check_reference(expected_summary).resolve():
        raise ValueError('runtime summary differs from accepted evidence')
    summary=read(args.runtime_summary)
    inventory={r['id']:r['corpus'] for r in summary['pairs']}
    if len(inventory)!=len(summary['pairs']):raise ValueError('duplicate runtime case')
    selected={r['id']:r['corpus'] for r in manifest['selected']}
    if any(inventory.get(key)!=corpus for key,corpus in selected.items()):raise ValueError('runtime case/corpus mapping differs')
    if manifest['selection']=='full-frozen-and-discovery' and inventory!=selected:
        raise ValueError('runtime and capture full inventories differ')
    return summary


def validate_analysis(report, args):
    """Recompute cached static metrics from pinned source artifacts before producing tables."""
    import importlib.util
    from pathlib import Path
    repo=Path(__file__).resolve().parents[1]
    spec=importlib.util.spec_from_file_location('analysis_validation_results',repo/'issue-nvvm-backend/nvvm-results.py')
    nr=importlib.util.module_from_spec(spec);spec.loader.exec_module(nr)
    if args.capture.resolve()!=check_reference(report['capture']).resolve():raise ValueError('different capture requested for report')
    check_reference(report['analyzer'])
    for ref in report['analysis_helpers'].values():check_reference(ref)
    for name,ref in report['tools'].items():
        if getattr(args,name).resolve()!=check_reference(ref).resolve():raise ValueError('requested tool differs from recorded analysis')
    if args.architecture!=report['architecture']:raise ValueError('requested architecture differs from recorded analysis')
    capture=read(args.capture);manifest=read(check_reference(capture['manifest']))
    if manifest!=read(check_reference(report['manifest'])):raise ValueError('analysis manifest differs from capture')
    validate_capture(capture,manifest,repo,nr)
    if [(r['id'],r['mode']) for r in report['cells']]!=[(r['id'],r['mode']) for r in capture['cells']]:
        raise ValueError('analysis inventory differs from capture')
    for row,cell in zip(report['cells'],capture['cells']):
        if any(row[k]!=cell[k] for k in ['id','mode','corpus','source','oracle']) or row['capture_status']!=cell['status']:
            raise ValueError('analysis/capture binding mismatch')
        if cell['status']!='qualified':
            if row['status']!='capture-excluded':raise ValueError('unqualified capture became analyzed')
            continue
        if row['status']=='analysis-failed':continue
        if row['status']!='analyzed' or row['entry']!=cell['entry'] or row['ptx']!=cell['ptx']:
            raise ValueError('analyzed PTX/entry differs from capture')
        for artifact in row['artifacts'].values():check_reference(artifact)
        raw=parse_ptx(check_reference(row['ptx']).read_text(),row['entry'])
        ptx_path=Path(row['artifacts']['ptx.json']['path'])
        if raw!=read(ptx_path):raise ValueError('PTX metrics differ from source')
        normalized=Path(row['artifacts']['normalized-ptx.txt']['path']).read_text()
        if normalized!='\n\n'.join(raw['normalized_stream'])+'\n':raise ValueError('normalized PTX differs from source')
        if row['ptx_metrics']!={k:v for k,v in raw.items() if k not in ('normalized_stream','opcode_sequence')}:
            raise ValueError('cached PTX metrics changed')
        resources=nr.parse_resources(Path(row['artifacts']['ptxas.log']['path']).read_text())
        if resources!=row['resources'] or row['entry_resources']!=next(r for r in resources if r['function']==row['entry']):
            raise ValueError('cached hardware resource metrics changed')
        for process,artifact in [('assembly','ptxas.log'),('disassembly','sass.log')]:
            nr.require_success(row[process])
            if row[process]['log']!=row['artifacts'][artifact]['path']:raise ValueError('process log differs from artifact')
        if row['disassembly']['command']!=[str(args.cuobjdump),'--dump-sass',row['artifacts']['code.cubin']['path']]:
            raise ValueError('disassembly command scope changed')
        expected_command=[str(args.ptxas),'-v',f'-arch=sm_{report["architecture"]}',row['ptx']['path'],'-o',row['artifacts']['code.cubin']['path']]
        if row['assembly']['command']!=expected_command:raise ValueError('assembly command scope changed')
        sass=parse_sass(Path(row['artifacts']['sass.log']['path']).read_text())
        if sass!=read(Path(row['artifacts']['sass.json']['path'])):raise ValueError('SASS metrics differ from disassembly')
        if row['sass_metrics']!={name:{k:v for k,v in val.items() if k!='instruction_text'} for name,val in sass.items()}:
            raise ValueError('cached SASS metrics changed')
        validate_sass_coverage(sass,row['executable_sections'])
        if executable_sections(Path(row['artifacts']['code.cubin']['path']).read_bytes())!=row['executable_sections']:
            raise ValueError('cached executable bytes changed')
    counts=dict(collections.Counter(r['status'] for r in report['cells']))
    if report['counts']!=counts:raise ValueError('analysis counts changed')
    return manifest


def summarize(args):
    """Join static metrics with accepted case observations, with explicit scope and similarity labels."""
    import csv
    from pathlib import Path
    report=read(args.output/'results.json')
    if report['status'] not in ('completed','completed-with-analysis-failures'): raise ValueError('analysis incomplete')
    manifest=validate_analysis(report,args)
    runtime=load_runtime_summary(args,manifest)
    timed={p['id']:p for p in runtime['pairs']}
    flat=[];parsed={}
    for row in report['cells']:
        value={k:row[k] for k in ('id','corpus','source','mode','status','capture_status','oracle')}
        if row['status']=='analyzed':
            for artifact in row['artifacts'].values():check_reference(artifact)
            ptx=read(Path(row['artifacts']['ptx.json']['path']));parsed[row['id'],row['mode']]=ptx
            r=row['entry_resources'];e=ptx['entry_metrics'];s=row['sass_metrics'][row['entry']]
            value.update(ptx_entry_instructions=e['instructions'],ptx_reachable_instructions=ptx['reachable_metrics']['instructions'],
                         ptx_reachable_functions=len(ptx['reachable_functions']),
                         ptx_virtual_registers=sum(e['virtual_registers'].values()),ptx_virtual_registers_scope='entry declarations; mixed widths; not allocated registers',
                         hardware_registers=r['registers'],stack_bytes=r['stack_bytes'],spill_store_bytes=r['spill_store_bytes'],spill_load_bytes=r['spill_load_bytes'],shared_bytes=r['shared_bytes'],
                         sass_entry_instructions=s['instructions'],sass_entry_non_nop_instructions=s['non_nop_instructions'],
                         sass_module_instructions=sum(f['instructions'] for f in row['sass_metrics'].values()),
                         entry_text_bytes=row['executable_sections']['.text.'+row['entry']]['bytes'],
                         module_text_bytes=sum(f['bytes'] for f in row['executable_sections'].values()),
                         ptx_entry_branches=e['branches'],ptx_entry_shuffles=e['shuffles'],ptx_entry_calls=e['calls'],
                         ptx_entry_local_loads=e['memory']['ld_local'],ptx_entry_local_stores=e['memory']['st_local'],
                         unresolved_calls=len(ptx['unresolved_calls']))
        flat.append(value)
    lookup={(r['id'],r['mode']):r for r in flat};rawlookup={(r['id'],r['mode']):r for r in report['cells']}
    pairs=[]
    for identity in dict.fromkeys(r['id'] for r in flat):
        a=lookup.get((identity,'nvrtc-o3'));b=lookup.get((identity,'nvvm-o3'));c=lookup.get((identity,'nvvm-o0'))
        p=dict(id=identity,corpus=a['corpus'],runtime_status=timed.get(identity,{}).get('status','unavailable'))
        if not a or not b or a['status']!='analyzed' or b['status']!='analyzed':
            p['status']='incomplete';pairs.append(p);continue
        ra=rawlookup[identity,'nvrtc-o3'];rb=rawlookup[identity,'nvvm-o3'];pa=parsed[identity,'nvrtc-o3'];pb=parsed[identity,'nvvm-o3']
        entry_a=ra['executable_sections']['.text.'+ra['entry']];entry_b=rb['executable_sections']['.text.'+rb['entry']]
        p.update(status='paired',raw_ptx_equal=ra['ptx']['sha256']==rb['ptx']['sha256'],
                 normalized_reachable_ptx_instructions_equal=bool(pa['normalized_stream_sha256']) and pa['normalized_stream_sha256']==pb['normalized_stream_sha256'],
                 offline_entry_text_equal=entry_a['sha256']==entry_b['sha256'],
                 offline_all_text_sections_equal=ra['executable_sections']==rb['executable_sections'],
                 opcode_order_similarity=opcode_similarity(pa['opcode_sequence'],pb['opcode_sequence']))
        for metric in ['ptx_entry_instructions','ptx_reachable_instructions','hardware_registers','stack_bytes','spill_store_bytes','spill_load_bytes','sass_entry_instructions','sass_module_instructions','entry_text_bytes','ptx_reachable_functions','ptx_entry_branches','ptx_entry_shuffles','ptx_entry_local_loads','ptx_entry_local_stores']:
            p['nvrtc_o3_'+metric]=a.get(metric);p['nvvm_o3_'+metric]=b.get(metric);p['nvvm_o0_'+metric]=c.get(metric) if c else None
        p['nvvm_over_nvrtc_ptx_instructions']=b['ptx_reachable_instructions']/a['ptx_reachable_instructions'] if a['ptx_reachable_instructions'] else None
        p['nvvm_over_nvrtc_sass_instructions']=b['sass_entry_instructions']/a['sass_entry_instructions'] if a['sass_entry_instructions'] else None
        p['similarity_class']=('identical-offline-executable-bytes' if p['offline_all_text_sections_equal'] else
                               'equal-normalized-ptx-instructions' if p['normalized_reachable_ptx_instructions_equal'] else
                               'similar-static-profile' if p['opcode_order_similarity']>=.9 and .9<=p['nvvm_over_nvrtc_sass_instructions']<=1.1 and abs(a['hardware_registers']-b['hardware_registers'])<=2 and all(a[k]==b[k] for k in ['stack_bytes','spill_store_bytes','spill_load_bytes']) else 'different-static-profile')
        for key in ['nvrtc_o3_ms','nvvm_o3_ms','nvvm_o0_ms','nvrtc_over_nvvm_o3']:
            p['prior_dispatch_'+key]=timed.get(identity,{}).get(key)
        pairs.append(p)
    def write_csv(path,rows):
        fields=list(dict.fromkeys(k for r in rows for k in r))
        with path.open('w',newline='') as f:
            writer=csv.DictWriter(f,fields);writer.writeheader();writer.writerows(rows)
    write_csv(args.output/'cases.csv',flat);write_csv(args.output/'pairs.csv',pairs)
    summary=dict(schema=1,results=reference(args.output/'results.json'),runtime_summary=reference(args.runtime_summary),runtime_evidence=reference(args.runtime_evidence),
                 scope=report['scope'],architecture=report['architecture'],counts=report['counts'],
                 similarity_counts=dict(collections.Counter(p.get('similarity_class','incomplete') for p in pairs)),pairs=pairs,
                 limitations=['Fresh runtime-route captures match original source/options but historical timed PTX was not retained, so exact timed-byte identity is not proved.',
                              'Offline SM89 ptxas output is not recorded driver-JIT code. PTX virtual declarations are not allocated registers.',
                              'Instruction counts are static, include helper bodies once in reachable totals, and do not describe trip counts or lane activity.',
                              'Executable section hashes include padding; exact class compares every named text section, not global data, relocations or module semantic equivalence. Entry-only equality is also reported but does not qualify the exact class.',
                              'Normalized PTX instruction equality omits declarations/global data and is not a semantic proof. Opcode similarity is SequenceMatcher with autojunk; similar profile threshold is heuristic.',
                              'No performance ratios are newly derived for short-interval cases. Prior dispatch observations retain their original scope and identity.'])
    save(args.output/'summary.json',summary)
    print({k:summary[k] for k in ['counts','similarity_counts']},flush=True)


def main():
    import argparse
    from pathlib import Path
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--capture',type=Path,required=True,help='Completed capture results.json')
    parser.add_argument('--output',type=Path,required=True)
    parser.add_argument('--runtime-summary',type=Path,required=True)
    parser.add_argument('--runtime-evidence',type=Path,required=True,help='Immutable snapshot of accepted focused evidence pinning the timing manifest/summary')
    parser.add_argument('--ptxas',type=Path,default=Path('/usr/local/cuda-12.9/bin/ptxas'))
    parser.add_argument('--cuobjdump',type=Path,required=True)
    parser.add_argument('--nvdisasm',type=Path,required=True)
    parser.add_argument('--architecture',type=int,choices=[80,89],default=89)
    parser.add_argument('--report-only',action='store_true')
    args=parser.parse_args()
    for key in ['capture','output','runtime_summary','runtime_evidence','ptxas','cuobjdump','nvdisasm']:setattr(args,key,getattr(args,key).resolve())
    if not args.report_only:analyze_capture(args)
    summarize(args)


if __name__=='__main__':
    main()
