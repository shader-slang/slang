#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Validate physical surface execution proof before comparing recurring outcomes.

The runner's known negatives retain exit1. Only a complete, replayed physical report may
be compared. Successful converted NaNs are compared by class; every other byte remains exact.
"""
import collections
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import sys
from types import SimpleNamespace

REPO = Path(__file__).resolve().parents[1]
MODES = ('nvrtc-o3', 'nvvm-o0', 'nvvm-o3')
SCHEMA = 1


def require(condition, message):
    if not condition:
        raise ValueError(message)


def read(path):
    return json.loads(Path(path).read_text())


def sha(data):
    return hashlib.sha256(data).hexdigest()


def canonical_sha(value):
    return sha(json.dumps(value, sort_keys=True, separators=(',', ':')).encode())


def reference(path):
    path = Path(path).resolve()
    return {'path': str(path), 'sha256': sha(path.read_bytes())}


def load_harness():
    path = REPO / 'extras/validate-nvvm-surfaces.py'
    spec = importlib.util.spec_from_file_location('surface_harness', path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def diagnostic(text, output, repo):
    """Strip ANSI styling and invocation-specific roots, retaining full diagnostic text."""
    text = re.sub(r'\x1b\[[0-9;]*m', '', text)
    for path, token in ((output, '<output>'), (repo, '<repo>')):
        text = text.replace(str(Path(path).resolve()), token)
    return '\n'.join(line.rstrip() for line in text.splitlines()).strip()


def phase(cell, name, directory, repo, expected_code=None, expected_command=None):
    row = cell[name]
    code = row.get('return_code')
    require(type(code) is int and 0 <= code <= 255 and not row.get('timed_out') and
            not row.get('error'), 'invalid process completion: ' + name)
    if expected_code is not None:
        require(code == expected_code, 'wrong process return code: ' + name)
    require(row.get('command'), 'missing process command: ' + name)
    if expected_command is not None:
        require(row['command'] == [str(x) for x in expected_command], 'wrong requested phase command: ' + name)
    log_name = 'runtime' if name == 'runtime_process' else name
    log = directory / (log_name + '.log')
    require(Path(row['log']).resolve() == log.resolve(), 'phase log does not belong to cell')
    return {'return_code': code, 'diagnostic': diagnostic(log.read_text(), directory.parent.parent, repo)}


def validate_report(path, exit_code, harness=None):
    """Replay all current cases and return a compact candidate block, never accepted status.

    Runtime/compiler hashes prove this invocation's identity, but are deliberately outside
    outcome equality so a later compiler can be tested. Oracle identity hashes semantic
    descriptors and host bytes rather than JSON whitespace from a previous report.
    """
    harness = harness or load_harness()
    path = Path(path).resolve()
    directory = path.parent
    report = read(path)
    cases = harness.cases()
    case_map = {row['case']: row for row in cases}
    require(cases and len(case_map) == len(cases), 'invalid canonical case inventory')
    requested = {(name, mode) for name in case_map for mode in MODES}
    require(report.get('requested_cells') == len(requested), 'wrong requested cell count')
    cells = report['cells']
    ids = [(c['case'], c['mode']) for c in cells]
    require(len(ids) == len(set(ids)) and set(ids) == requested, 'incomplete/duplicate/unknown surface inventory')
    require(report.get('identity_verification') == 'unchanged', 'surface identity drift')
    provenance_path = directory / 'provenance.json'
    require(sha(provenance_path.read_bytes()) == report['provenance_sha256'], 'provenance tampered')
    provenance = read(provenance_path)
    require(provenance.get('artifacts'), 'missing runtime identity inventory')
    external = provenance.get('external_provenance')
    if external:
        require(sha(Path(external['path']).read_bytes()) == external['sha256'], 'external provenance changed')
    inventory_args = SimpleNamespace(slangc=Path(report['slangc']['path']),
        cuda_root=Path(report['ptxas']['path']).parent.parent,
        provenance=Path(external['path']) if external else None)
    expected_paths = harness.inventory_paths(inventory_args, Path(report['provider']['path']).parent)
    require(set(provenance['artifacts']) == {str(p) for p in expected_paths}, 'incomplete runtime identity inventory')
    harness.verify_identity(provenance['artifacts'])
    require(report['tool_sha256'] == sha(Path(harness.__file__).read_bytes()), 'surface tool changed')
    for tool in ('slangc', 'provider', 'libnvvm', 'ptxas'):
        ref = report[tool]
        # The inventory pins both a library symlink and its resolved target. Tool references
        # may name the target directly; resolve them through that existing canonical identity.
        target = str(Path(ref['path']).resolve())
        selected = [item for item in provenance['artifacts'].values()
                    if item['resolved_path'] == target]
        require(selected and all(item['sha256'] == ref['sha256'] for item in selected) and
                ref['sha256'] == sha(Path(target).read_bytes()), 'tool identity mismatch: ' + tool)
    expected_status = 'passed' if all(c['status'] == 'passed' for c in cells) else 'failed'
    require(report['status'] == expected_status and exit_code == (0 if expected_status == 'passed' else 1), 'raw report/exit status mismatch')
    require(report['counts'] == dict(collections.Counter(c['status'] for c in cells)), 'raw count mismatch')
    outcomes = []
    for cell in sorted(cells, key=lambda c: (c['case'], c['mode'])):
        row = case_map[cell['case']]
        case_dir = directory / row['case']
        cell_dir = case_dir / cell['mode']
        source = harness.REPO / 'tests/cuda' / ('nvvm-surface-physical-' + row['fixture'] + '.slang')
        require(Path(cell['source']).resolve() == source.resolve() and cell['entry'] == row['entry'], 'wrong source/entry')
        source_hash = sha(source.read_bytes())
        require(cell['source_sha256'] == source_hash, 'source changed during execution')
        buffers = harness.oracle(row)
        specs, resources = harness.resource_specs(row), harness.resource_buffers(row, buffers)
        require(len(specs) == len(resources) and len({s['name'] for s in specs}) == len(specs), 'resource inventory mismatch')
        files = harness.oracle_files(row, buffers)
        file_hashes = {key: sha(value) for key, value in files.items()}
        for key, data in files.items():
            require((case_dir / (key + '.bin')).read_bytes() == data, 'host oracle bytes changed')
        oracle_path = case_dir / 'oracle.json'
        frozen = read(oracle_path)
        oracle = {'row': row, 'source_sha256': source_hash, 'file_sha256': file_hashes,
                  'converted_nan_positions': [sorted(r['nan_positions']) for r in resources]}
        require(frozen == oracle and sha(oracle_path.read_bytes()) == cell['oracle_sha256'], 'oracle record changed')
        compact = {'id': cell['case'], 'mode': cell['mode'], 'source_sha256': source_hash,
                   'case_sha256': canonical_sha(row), 'oracle_sha256': canonical_sha(oracle),
                   'status': cell['status'], 'phases': {}}
        require(cell['status'] in ('passed', 'compile-failed', 'runtime-mismatch'), 'unsupported failure phase: ' + cell['status'])
        command = [report['slangc']['path'], source, '-target', 'ptx', '-entry', row['entry'],
                   '-stage', 'compute', '-capability', 'cuda_sm_8_0', '-O' + cell['mode'][-1],
                   '-o', cell_dir / 'code.ptx', '-reflection-json', cell_dir / 'reflection.json']
        command += [f'-D{key}={value}' for key, value in row['defines'].items()]
        if cell['mode'].startswith('nvvm'):
            command += ['-emit-cuda-via-nvvm', '-nvvm-path', report['libnvvm']['path']]
        compiled = phase(cell, 'compile', cell_dir, harness.REPO, expected_command=command)
        compact['phases']['compile'] = compiled
        if cell['status'] == 'compile-failed':
            require(compiled['return_code'] > 0 and compiled['diagnostic'], 'unproven compile negative')
            require(not any(k in cell for k in ('assemble', 'runtime_process', 'runtime')) and not (cell_dir / 'runtime.json').exists(), 'compile failure has later execution')
            compact['phase'] = 'compile'
            outcomes.append(compact)
            continue
        require(compiled['return_code'] == 0 and cell.get('binding_verified') is True, 'false runtime success')
        ptx_path, reflection_path = cell_dir / 'code.ptx', cell_dir / 'reflection.json'
        require(sha(ptx_path.read_bytes()) == cell['ptx_sha256'] and
                sha(reflection_path.read_bytes()) == cell['reflection_sha256'], 'compiled artifact tampered')
        harness.validate_bindings(read(reflection_path), row, ptx_path.read_text(), 80)
        compact['phases']['assemble'] = phase(cell, 'assemble', cell_dir, harness.REPO, 0,
            [report['ptxas']['path'], '-arch=sm_80', ptx_path, '-o', cell_dir / 'code.cubin'])
        require((cell_dir / 'code.cubin').is_file() and (cell_dir / 'code.cubin').stat().st_size > 0, 'missing assembly output')
        config_path = cell_dir / 'device-case.json'
        require(read(config_path) == {'row': row, 'ptx': str(ptx_path), 'oracle_sha256': file_hashes}, 'wrong device worker contract')
        compact['phases']['runtime'] = phase(cell, 'runtime_process', cell_dir, harness.REPO,
            0 if cell['status'] == 'passed' else 1,
            [sys.executable, Path(harness.__file__), '--device-case', config_path])
        runtime = read(cell_dir / 'runtime.json')
        require(runtime == cell['runtime'] and runtime['status'] == cell['status'], 'runtime record differs')
        for proof in ('initial_host_copies_verified', 'surface_bindings_verified',
                      'global_surface_handles_uploaded', 'launched_and_synchronized'):
            require(runtime.get(proof) is True, 'missing runtime proof: ' + proof)
        cleanup = runtime.get('cleanup', [])
        expected_cleanup = ['cuModuleUnload'] + ['cuSurfObjectDestroy'] * len(specs) + ['cuArrayDestroy'] * len(specs) + ['cuCtxDestroy_v2']
        require([r['operation'] for r in cleanup] == expected_cleanup and
                all(r['return_code'] == 0 for r in cleanup), 'incomplete/failed cleanup')
        descriptors = [{'Width': row['width'], 'Height': row['height'] if row['shape'] == 2 else 0,
                        'Depth': row.get('array_layers', 0), 'Format': harness.FORMATS[s['storage']][0],
                        'NumChannels': s['lanes'], 'Flags': 3 if 'array_layers' in row else 2}
                       for s in specs]
        require(runtime['actual_array_descriptors'] == descriptors, 'wrong actual resource descriptors')
        readbacks = runtime['readbacks']
        require([r['array'] for r in readbacks] == [s['name'] for s in specs], 'incomplete/duplicate readbacks')
        compact.update(phase='runtime', resources=[])
        mismatches = 0
        for index, (spec, data, recorded) in enumerate(zip(specs, resources, readbacks)):
            actual = (cell_dir / (spec['name'] + '-actual.bin')).read_bytes()
            replay = harness.compare(row, buffers, index, actual)
            replay.update(array=spec['name'], active_texels=data['active_texels'], guard_texels=data['guard_texels'])
            require(replay == recorded, 'readback proof does not match physical bytes')
            mismatches += replay['mismatch_count']
            normalized = bytearray(actual)
            size = harness.FORMATS[spec['storage']][1]
            if cell['status'] == 'passed':
                for position in data['nan_positions']:
                    bits = int.from_bytes(actual[position * size:(position + 1) * size], 'little')
                    exponent, fraction = (0x7c00, 0x3ff) if size == 2 else (0x7f800000, 0x7fffff)
                    require(bits & exponent == exponent and bits & fraction, 'converted NaN became non-NaN')
                    normalized[position * size:(position + 1) * size] = exponent.to_bytes(size, 'little')
            compact['resources'].append({
                'name': spec['name'], 'descriptor': descriptors[index],
                'initial_sha256': sha(data['initial']), 'expected_sha256': sha(data['expected']),
                'nan_positions_sha256': canonical_sha(sorted(data['nan_positions'])),
                'nan_position_count': len(data['nan_positions']), 'active_texels': data['active_texels'],
                'guard_texels': data['guard_texels'], 'exact_channels': replay['exact_channels'],
                'classified_nan_channels': replay['classified_nan_channels'],
                'actual_semantic_sha256': sha(normalized), 'mismatch_count': replay['mismatch_count'],
                'mismatches': replay['mismatches'],
            })
        require((mismatches == 0) == (cell['status'] == 'passed'), 'false pass/mismatch classification')
        outcomes.append(compact)
    return {'schema': SCHEMA, 'status': 'validated', 'requested_cells': len(requested),
            'raw_results': reference(path), 'provenance': reference(provenance_path),
            'identity': {k: report[k] for k in ('slangc', 'provider', 'libnvvm', 'ptxas', 'tool_sha256')},
            'fresh_cell_outcomes': outcomes}


def compare(previous, current):
    """Require explicit review for bootstrap, added cases, and every changed obligation."""
    require(current.get('schema') == SCHEMA and current.get('status') == 'validated', 'unvalidated surface comparison input')
    def index(block):
        rows = block['fresh_cell_outcomes']
        result = {(r['id'], r['mode']): r for r in rows}
        require(len(result) == len(rows) == block['requested_cells'] and result, 'invalid surface baseline inventory')
        require(all(mode in MODES and all((name, m) in result for m in MODES)
                    for name, mode in result), 'incomplete surface baseline modes')
        return result
    after = index(current)
    if previous is None:
        return {'status': 'review-required', 'reason': 'surface baseline bootstrap requires explicit review',
                'previous_cells': 0, 'observed_cells': len(after), 'additions': sorted(after),
                'missing': [], 'transitions': []}
    require(previous.get('schema') == SCHEMA, 'unsupported surface baseline schema')
    before = index(previous)
    transitions = [{'id': k[0], 'mode': k[1], 'before': before[k], 'after': after[k]}
                   for k in sorted(before.keys() & after.keys()) if before[k] != after[k]]
    additions, missing = sorted(after.keys() - before.keys()), sorted(before.keys() - after.keys())
    return {'status': 'review-required' if transitions or additions or missing else 'passed',
            'previous_cells': len(before), 'observed_cells': len(after),
            'preserved_cells': len(before.keys() & after.keys()) - len(transitions),
            'additions': additions, 'missing': missing, 'transitions': transitions}
