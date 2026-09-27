#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Measure fixed synthetic material workloads using previously qualified shader artifacts.

Preparation pins the accepted scalar oracles, compile artifacts, shared driver and input tiles.
Qualification checks all enlarged output bytes before any timing is accepted. Measurement uses
CUDA event intervals with complete reset/readback/reference checks around every launch. Tiny hot
textures and periodic inputs make this a synthetic comparison, not application performance.
"""
import argparse
from datetime import datetime, timezone
import hashlib
import importlib.util
import json
import math
import os
from pathlib import Path
import re
import statistics
import subprocess
import sys
import time

REPO = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location(
    'material', REPO / 'extras/validate-nvvm-material-runtime.py'
)
material = importlib.util.module_from_spec(spec)
spec.loader.exec_module(material)
PROTOCOL = 'tiled-brass-periodic-device-events-v1'
COUNTS = (65537, 1048577)
ENTRIES = ('eval_buffer', 'sample_buffer')
MODES = ('nvrtc-o3', 'nvvm-o0', 'nvvm-o3')
WARMUPS, SAMPLES, ROUNDS = (3, 9, 2)


def sha(path):
    """Hash a complete artifact without loading large buffers into memory."""
    digest = hashlib.sha256()
    with Path(path).open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            digest.update(block)
    return digest.hexdigest()


def save(path, value):
    """Persist the current report without allowing nonfinite JSON values."""
    Path(path).write_text(json.dumps(value, indent=2, allow_nan=False) + '\n')


def fixed_cells():
    """Enumerate the twelve independent large-output qualification cells."""
    return [
        dict(id=f'{entry}-{count}-{mode}', entry=entry, count=count, mode=mode)
        for entry in ENTRIES for count in COUNTS for mode in MODES
    ]


def timed_cells():
    """Freeze two rounds and reverse the complete cell order in round two."""
    base = fixed_cells()
    return [
        dict(cell, id=f"round{round_index}-{cell['id']}", round=round_index)
        for round_index in range(ROUNDS)
        for cell in (base if round_index == 0 else reversed(base))
    ]


def stride(entry):
    """Return the existing entry output stride after validating the entry name."""
    return 1 << material.entry_layout(entry)['output_shift']


def repeat_inputs(entry, count):
    """Repeat the accepted 65-record input bytes and truncate to the reviewed count."""
    if count not in (65,) + COUNTS:
        raise ValueError('count outside frozen protocol')
    layout = material.entry_layout(entry)
    tile = b''.join(
        layout['input_struct'].pack(
            *row['uv'], *row['incoming'], *row.get('outgoing', ()), row['seed']
        )
        for row in layout['cases']()
    )
    return (tile * ((count + 64) // 65))[:count * layout['input_stride']]


def qualify_outputs(data, entry, count):
    """Independently check the first tile, then every active byte against that verified tile.

    Record i receives exactly the original input record i % 65. After the independent oracle
    checks all 65 tile outputs, byte equality establishes the same contract for each subsequent
    output, including exact sample flags and rejection zeros. The actual guard bytes are checked
    separately; synthetic guards below only adapt the original 65-record comparison interface.
    """
    if count not in (65,) + COUNTS:
        raise ValueError('count outside frozen protocol')
    size = stride(entry)
    if len(data) != (count + 63) * size:
        return dict(status='failed', reason='wrong output length', bytes=len(data))
    layout = material.entry_layout(entry)
    expected = layout['expected'](layout['cases']())
    tile = data[:65 * size]
    first = layout['compare'](tile + b'\xa5' * (63 * size), expected)
    periodic = (tile * ((count + 64) // 65))[:count * size] == data[:count * size]
    guards = data[count * size:] == b'\xa5' * (63 * size)
    rejected = count // 65 if entry == 'sample_buffer' else 0
    return dict(
        status='passed' if first['status'] == 'passed' and periodic and guards else 'failed',
        tile_comparison=first,
        all_active_bytes_periodic=periodic,
        guards_unchanged=guards,
        active_records=count,
        compared_bytes=len(data),
        guard_records=63,
        rejection_records=rejected,
        non_rejected_records=count - rejected
    )


def verify_pins(pins):
    """Reject any artifact whose current bytes differ from its recorded identity."""
    for path, digest in pins.items():
        if sha(path) != digest:
            raise ValueError('changed pinned artifact: ' + path)


def validate_report_cells(cells, wanted):
    """Require exactly one passed result for each requested cell and its full identity."""
    if (len(cells) != len(wanted)
            or {row.get('id') for row in cells} != {row['id'] for row in wanted}):
        raise ValueError('missing or duplicate protocol cells')
    by_id = {row['id']: row for row in cells}
    for expected in wanted:
        actual = by_id[expected['id']]
        if (any(actual.get(key) != value for key, value in expected.items())
                or actual.get('status') != 'passed'):
            raise ValueError('unqualified or changed protocol cell: ' + expected['id'])
    return by_id


def read_protocol(path):
    """Verify the prepared source, inputs, execution environment and fixed sampling policy."""
    protocol = json.loads(Path(path).read_text())
    if protocol.get('protocol') != PROTOCOL or protocol.get('status') != 'prepared':
        raise ValueError('invalid protocol')
    if (protocol.get('correctness_order') != fixed_cells()
            or protocol.get('measurement_order') != timed_cells()):
        raise ValueError('changed frozen order')
    if (
        protocol.get('warmups'),
        protocol.get('samples'),
        protocol.get('rounds')
    ) != (WARMUPS, SAMPLES, ROUNDS):
        raise ValueError('changed frozen sampling policy')
    verify_pins(protocol['artifact_sha256'])
    if protocol.get('cuda_environment') != {
        key: os.environ.get(key) for key in ('CUDA_VISIBLE_DEVICES', 'CUDA_MODULE_LOADING')
    }:
        raise ValueError('CUDA environment changed since preparation')
    return protocol


def prepare(args, report):
    """Pin qualified shader artifacts and materialize the unchanged periodic inputs."""
    pins = {}
    for path in (
        Path(__file__),
        REPO / 'extras/validate-nvvm-material-runtime.py',
        REPO / 'extras/nvvm-material-runtime-driver.cpp',
        args.helper
    ):
        pins[str(path.resolve())] = sha(path)
    accepted = {}
    payloads = {}
    for entry, report_path in zip(ENTRIES, (args.eval_reference, args.sample_reference)):
        path = report_path.resolve()
        value = json.loads(path.read_text())
        if (value.get('status') != 'passed'
                or value.get('contract') != material.entry_layout(entry)['contract']):
            raise ValueError('accepted entry reference is invalid')
        if {row['id'] for row in value['cells']} != set(MODES) or len(value['cells']) != 3:
            raise ValueError('accepted mode inventory is invalid')
        verify_pins(value['artifact_sha256'])
        pins.update(value['artifact_sha256'])
        pins[str(path)] = sha(path)
        oracle_path = path.parent / 'expected.json'
        if sha(oracle_path) != value['oracle_sha256']:
            raise ValueError('accepted oracle hash changed')
        layout = material.entry_layout(entry)
        inputs = layout['cases']()
        expected = layout['expected'](inputs)
        material.validate_oracle_hash(path, sha(oracle_path))
        oracle = dict(
            inputs=inputs,
            expected=expected,
            absolute_tolerance=material.ABS_TOL,
            relative_tolerance=material.REL_TOL
        )
        if json.loads(json.dumps(oracle)) != json.loads(oracle_path.read_text()):
            raise ValueError('accepted oracle differs from live equations')
        pins[str(oracle_path)] = sha(oracle_path)
        accepted[entry] = dict(
            report=str(path), oracle=str(oracle_path), oracle_sha256=sha(oracle_path), modes={}
        )
        for row in value['cells']:
            if row['status'] != 'passed' or row.get('entry') != entry:
                raise ValueError('accepted cell not passed')
            directory = path.parent / row['id']
            ptx = directory / 'shader.ptx'
            cubin = directory / 'shader.cubin'
            if sha(ptx) != row['ptx_sha256'] or sha(cubin) != row['cubin_sha256']:
                raise ValueError('changed accepted shader bytes')
            material.validate_ptx(ptx.read_text(), row['architecture'], entry)
            pins[str(ptx)] = sha(ptx)
            pins[str(cubin)] = sha(cubin)
            accepted[entry]['modes'][row['id']] = dict(
                cubin=str(cubin), cubin_sha256=sha(cubin), ptx_sha256=sha(ptx)
            )
        payloads[entry] = {}
        for count in (65,) + COUNTS:
            directory = args.output / f'inputs-{entry}-{count}'
            directory.mkdir()
            blobs = {
                'inputs.bin': repeat_inputs(entry, count),
                'color.bin': material.struct.pack(
                    '<16f', *(v for color in material.COLORS for v in color)),
                'roughness.bin': material.struct.pack(
                    '<16f',
                    *(v for r in material.ROUGHNESS for v in (r, 0, 0, 1))
                )
            }
            for name, data in blobs.items():
                file = directory / name
                file.write_bytes(data)
                pins[str(file)] = sha(file)
            payloads[entry][str(count)] = str(directory)
    report.update(
        status='prepared',
        accepted=accepted,
        payloads=payloads,
        artifact_sha256=pins,
        helper=str(args.helper),
        cuda_environment={
            key: os.environ.get(key) for key in ('CUDA_VISIBLE_DEVICES', 'CUDA_MODULE_LOADING')
        },
        correctness_order=fixed_cells(),
        measurement_order=timed_cells(),
        warmups=WARMUPS,
        samples=SAMPLES,
        rounds=ROUNDS,
        timeout_seconds=args.timeout,
        event_api=('cuEventElapsedTime_v2 (CUDA12.9 three-argument ABI); '
                   'start/stop in default stream'),
        timing_scope=('single kernel event interval; host submission gaps can contribute; '
                      'reset/readback/comparison excluded'),
        limitations=[
            '2x2 hot textures',
            'periodic 65-record input',
            'sample rejection fraction about 1/65',
            'persistent cell allocations with correctness transfers between every launch',
            'no application performance claim',
            '<0.1ms is timer/launch-scale, not throughput evidence'
        ]
    )


def monitor():
    """Capture read-only device/process snapshots and flag observed competing processes."""
    command = [
        'nvidia-smi',
        '--query-gpu=uuid,name,driver_version,pstate,clocks.current.sm,clocks.current.memory,'
        'temperature.gpu,power.draw,utilization.gpu',
        '--format=csv'
    ]
    result = {}
    for name, cmd in (
        ('state', command),
        ('processes', ['nvidia-smi', '--query-compute-apps=pid,process_name,used_memory',
                       '--format=csv'])
    ):
        try:
            process = subprocess.run(
                cmd,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                timeout=15
            )
            result[name] = dict(command=cmd, return_code=process.returncode, output=process.stdout)
        except (OSError, subprocess.SubprocessError) as error:
            result[name] = dict(command=cmd, error=str(error))
    process = result.get('processes', {})
    observed = process.get('return_code') == 0
    result['process_visibility'] = 'snapshot-only' if observed else 'unavailable'
    result['competing_process_observed'] = observed and len([
        line for line in process.get('output', '').splitlines() if line.strip()
    ]) > 1
    return result


def validate_log(log, entry, count, measure):
    """Require real launches, exact layout/counts, valid texture handles and cleanup."""
    size = stride(entry)
    capacity = count + 63
    launches = WARMUPS + SAMPLES if measure else 1
    expected = (
        f"execution launches={launches} active={count} output_capacity={capacity} "
        f"global_bytes=168 input_stride={material.entry_layout(entry)['input_stride']} "
        f"output_stride={size} blocks={capacity // 64} "
        f"phase={'measure' if measure else 'qualify'}"
    )
    if log.splitlines().count(expected) != 1 or log.splitlines().count('cleanup=PASS') != 1:
        raise ValueError('missing execution or cleanup evidence')
    handles = re.findall(
        r'texture=(color|roughness) handle_decimal=(\d+) '
        r'handle_hex=(0x[0-9a-f]{16}) low30_fit=true nonzero=true',
        log
    )
    if len(handles) != 2 or {r[0] for r in handles} != {'color', 'roughness'}:
        raise ValueError('missing texture handle proof')
    for _, decimal, hexadecimal in handles:
        n = int(decimal)
        if n != int(hexadecimal, 16) or not 0 < n <= 1073741823:
            raise ValueError('invalid texture handle')
    device = re.search(
        'cuda_header_version=(\\d+) driver_api_version=(\\d+) '
        'device_ordinal=0 device=(.+) sm=(\\d+)',
        log
    )
    resource = re.search('device_resources sm_count=(\\d+) l2_bytes=(\\d+)', log)
    if not device or not resource:
        raise ValueError('missing device identity')
    return dict(
        header_version=int(device[1]),
        driver_api_version=int(device[2]),
        device=device[3],
        sm=int(device[4]),
        multiprocessors=int(resource[1]),
        l2_bytes=int(resource[2]),
        launches=launches
    )


def parse_measurements(log, count, entry):
    """Preserve every requested warmup/sample and reject missing or invalid measurements."""
    pattern = (
        r"measurement index=(\d+) phase=(warmup|sample) device_ms=(\S+) "
        r"host_submit_ns=(\d+) host_until_stop_ns=(\d+) reset_bytes=(\d+) "
        r"compared_bytes=(\d+) equal=(true|false) valid_time=(true|false)"
    )
    found = re.findall(pattern, log)
    rows = []
    for index in range(WARMUPS + SAMPLES):
        matches = [r for r in found if int(r[0]) == index]
        expected_phase = 'warmup' if index < WARMUPS else 'sample'
        if len(matches) != 1:
            rows.append(dict(
                index=index, phase=expected_phase, status='failed',
                reason='missing or duplicate measurement'
            ))
            continue
        _, phase, device, submit, stop, reset, compared, equal, valid = matches[0]
        device = float(device)
        nbytes = (count + 63) * stride(entry)
        passed = (
            phase == expected_phase and math.isfinite(device) and device > 0
            and int(submit) > 0 and int(stop) >= int(submit)
            and int(reset) == nbytes and int(compared) == nbytes
            and equal == 'true' and valid == 'true'
        )
        rows.append(dict(
            index=index,
            phase=phase,
            status='passed' if passed else 'failed',
            device_ms=device if math.isfinite(device) else str(device),
            host_submit_ns=int(submit),
            host_until_stop_ns=int(stop),
            reset_bytes=int(reset),
            compared_bytes=int(compared),
            equal=equal == 'true'
        ))
    if len(found) != WARMUPS + SAMPLES or any((int(r[0]) >= WARMUPS + SAMPLES for r in found)):
        return dict(status='failed', reason='measurement inventory mismatch', rows=rows)
    return dict(
        status='passed' if all((r['status'] == 'passed' for r in rows)) else 'failed', rows=rows)


def summaries(measurements, count):
    """Summarize all nine successful samples, excluding warmups and limiting throughput claims."""
    values = [
        r['device_ms'] for r in measurements['rows']
        if r['phase'] == 'sample' and r['status'] == 'passed'
    ]
    if measurements['status'] != 'passed' or len(values) != SAMPLES:
        return None
    quartiles = statistics.quantiles(values, n=4, method='inclusive')
    median = statistics.median(values)
    return dict(
        device_ms=dict(
            median=median,
            q1=quartiles[0],
            q3=quartiles[2],
            iqr=quartiles[2] - quartiles[0],
            minimum=min(values),
            maximum=max(values)
        ),
        all_intervals_at_least_0_1ms=min(values) >= 0.1,
        records_per_second=count * 1000 / median if min(values) >= 0.1 else None
    )


def link_payloads(protocol, cell, directory):
    """Share immutable input artifacts without duplicating each cell's large input buffer."""
    source = Path(protocol['payloads'][cell['entry']][str(cell['count'])])
    for name in ('inputs.bin', 'color.bin', 'roughness.bin'):
        (directory / name).symlink_to(source / name)
    return {
        str(source / name): protocol['artifact_sha256'][str(source / name)]
        for name in ('inputs.bin', 'color.bin', 'roughness.bin')
    }


def run_cell(args, protocol, cell, reference=None):
    """Execute one qualified or measured cell with before/after artifact and device checks."""
    directory = args.output / cell['id']
    directory.mkdir()
    entry, count = (cell['entry'], cell['count'])
    shader = protocol['accepted'][entry]['modes'][cell['mode']]
    pins = link_payloads(protocol, cell, directory)
    pins[protocol['helper']] = protocol['artifact_sha256'][protocol['helper']]
    pins[shader['cubin']] = shader['cubin_sha256']
    if reference:
        (directory / 'reference.bin').symlink_to(reference['reference'])
        pins[reference['reference']] = reference['reference_sha256']
    cell.update(
        status='running',
        cubin_sha256=shader['cubin_sha256'],
        input_sha256=sha(directory / 'inputs.bin'),
        oracle_sha256=protocol['accepted'][entry]['oracle_sha256'],
        input_stride=material.entry_layout(entry)['input_stride'],
        output_stride=stride(entry),
        output_capacity=count + 63,
        artifact_sha256=pins
    )
    if reference:
        cell.update(
            reference=reference['reference'], reference_sha256=reference['reference_sha256'])
    verify_pins(pins)
    command = [
        protocol['helper'],
        shader['cubin'],
        str(directory),
        entry,
        str(count),
        'measure' if reference else 'qualify'
    ]
    cell['command'] = command
    cell['device_before'] = monitor()
    start = time.monotonic_ns()
    try:
        with (directory / 'execution.log').open('w') as stream:
            process = subprocess.run(
                command, stdout=stream, stderr=subprocess.STDOUT, timeout=args.timeout)
        cell['return_code'] = process.returncode
    except subprocess.TimeoutExpired:
        cell.update(return_code=None, error='timeout')
    cell['process_wall_ns'] = time.monotonic_ns() - start
    cell['device_after'] = monitor()
    log = (directory / 'execution.log').read_text()
    if reference:
        cell['measurements'] = parse_measurements(log, count, entry)
        cell['summary'] = None
    verify_pins(pins)
    cell['runtime'] = validate_log(log, entry, count, bool(reference))
    if cell['return_code'] != 0:
        raise ValueError('driver failed')
    if reference:
        if any(
            cell['runtime'][key] != value
            for key, value in reference['runtime'].items() if key != 'launches'
        ):
            raise ValueError('device/runtime identity differs from qualified reference')
        cell['contaminated'] = any(
            cell[name]['competing_process_observed'] for name in ('device_before', 'device_after')
        )
        cell['status'] = (
            ('contaminated' if cell['contaminated'] else 'passed')
            if cell['measurements']['status'] == 'passed' else 'failed'
        )
        if cell['status'] == 'passed':
            cell['summary'] = summaries(cell['measurements'], count)
    else:
        output = directory / 'outputs.bin'
        cell['comparison'] = qualify_outputs(output.read_bytes(), entry, count)
        cell.update(
            status=cell['comparison']['status'], reference=str(output), reference_sha256=sha(output))
    return cell


def qualification_bindings(protocol, row):
    """Bind a reference buffer to its exact shader, inputs, oracle, entry layout and count."""
    shader = protocol['accepted'][row['entry']]['modes'][row['mode']]
    payload = Path(protocol['payloads'][row['entry']][str(row['count'])]) / 'inputs.bin'
    return dict(
        cubin_sha256=shader['cubin_sha256'],
        input_sha256=sha(payload),
        oracle_sha256=protocol['accepted'][row['entry']]['oracle_sha256'],
        input_stride=material.entry_layout(row['entry'])['input_stride'],
        output_stride=stride(row['entry']),
        output_capacity=row['count'] + 63
    )


def run(args, report):
    """Require large-output qualification before the frozen measurement rounds."""
    protocol = read_protocol(args.protocol)
    if args.timeout != protocol.get('timeout_seconds'):
        raise ValueError('timeout differs from frozen protocol')
    report.update(
        protocol_path=str(args.protocol),
        protocol_sha256=sha(args.protocol),
        warmups=WARMUPS,
        samples=SAMPLES,
        rounds=ROUNDS
    )
    if args.phase == 'qualify':
        small = [
            dict(id=f'small-{entry}-{mode}', entry=entry, mode=mode, count=65)
            for entry in ENTRIES for mode in MODES
        ]
        cells = small + fixed_cells()
        references = {}
    else:
        qualification = json.loads(args.qualification.read_text())
        if (qualification.get('status') != 'passed'
                or qualification.get('protocol_sha256') != sha(args.protocol)):
            raise ValueError('qualification does not match prepared protocol')
        small = [
            dict(id=f'small-{entry}-{mode}', entry=entry, mode=mode, count=65)
            for entry in ENTRIES for mode in MODES
        ]
        by_id = validate_report_cells(qualification['cells'], small + fixed_cells())
        references = {}
        for row in by_id.values():
            if any((row.get(k) != v for k, v in qualification_bindings(protocol, row).items())):
                raise ValueError('qualification identity mismatch')
            if sha(row['reference']) != row['reference_sha256']:
                raise ValueError('changed qualified output')
            if qualify_outputs(
                Path(row['reference']).read_bytes(),
                row['entry'],
                row['count']
            )['status'] != 'passed':
                raise ValueError('reference failed independent requalification')
            references[row['entry'], row['count'], row['mode']] = row
        report.update(
            qualification=str(args.qualification), qualification_sha256=sha(args.qualification))
        cells = timed_cells()
    report['cells'] = [dict(row, status='pending') for row in cells]
    report['status'] = 'running'
    save(args.output / 'results.json', report)
    for cell in report['cells']:
        try:
            if (args.phase == 'qualify' and cell['count'] != 65
                    and any(r['status'] != 'passed' for r in report['cells'][:6])):
                raise ValueError('small-contract prerequisite failed')
            reference = (references.get((cell['entry'], cell['count'], cell['mode']))
                         if args.phase == 'measure' else None)
            run_cell(args, protocol, cell, reference)
        except (OSError, ValueError, subprocess.SubprocessError) as error:
            cell.update(status='failed', error=str(error))
        print(cell['id'] + ': ' + cell['status'], flush=True)
        save(args.output / 'results.json', report)
    verify_pins(protocol['artifact_sha256'])
    if sha(args.protocol) != report['protocol_sha256']:
        raise ValueError('protocol changed during execution')
    if args.phase == 'measure' and sha(args.qualification) != report['qualification_sha256']:
        raise ValueError('qualification changed during execution')
    report['status'] = (
        'passed' if all((row['status'] == 'passed' for row in report['cells'])) else 'failed')


def main():
    """Run one protocol phase in a fresh directory while preserving failures."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('phase', choices=('prepare', 'qualify', 'measure'))
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--helper', type=Path)
    parser.add_argument('--eval-reference', type=Path)
    parser.add_argument('--sample-reference', type=Path)
    parser.add_argument('--protocol', type=Path)
    parser.add_argument('--qualification', type=Path)
    parser.add_argument('--timeout', type=int, default=120)
    args = parser.parse_args()
    if args.timeout < 1 or args.timeout > 1800:
        parser.error('timeout must be 1..1800 seconds')
    needed = (
        ('helper', 'eval_reference', 'sample_reference') if args.phase == 'prepare'
        else ('protocol',) + (('qualification',) if args.phase == 'measure' else ())
    )
    for name in needed:
        if getattr(args, name) is None:
            parser.error('--' + name.replace('_', '-') + ' required')
    for name in ('output', 'helper', 'eval_reference', 'sample_reference',
                 'protocol', 'qualification'):
        if getattr(args, name) is not None:
            setattr(args, name, getattr(args, name).resolve())
    if args.output.exists():
        parser.error('output exists; preserve every prior attempt')
    args.output.mkdir(parents=True)
    report = dict(
        protocol=PROTOCOL,
        status='infrastructure-failed',
        phase=args.phase,
        started_utc=datetime.now(timezone.utc).isoformat(),
        cells=[]
    )
    save(args.output / 'results.json', report)
    try:
        if args.phase == 'prepare':
            prepare(args, report)
        else:
            run(args, report)
    except (OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
        report.update(status='infrastructure-failed', error=str(error))
        print(str(error), file=sys.stderr)
    save(args.output / 'results.json', report)
    return 0 if report['status'] in ('prepared', 'passed') else 1


if __name__ == '__main__':
    raise SystemExit(main())
