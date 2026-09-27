#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""CPU-only contracts for the frozen enlarged-output and measurement protocol."""
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import unittest
from unittest.mock import patch
from types import SimpleNamespace
spec = importlib.util.spec_from_file_location(
    'measure',
    Path(__file__).resolve().parents[1] / 'extras/measure-nvvm-material-runtime.py'
)
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class ProtocolContracts(unittest.TestCase):

    def output(self, entry, count, model='float'):
        layout = m.material.entry_layout(entry)
        expected = layout['expected'](layout['cases']())
        if entry == 'sample_buffer':
            tile = b''.join((m.material.SAMPLE_OUTPUT.pack(*row) for row in expected[model]))
        else:
            tile = b''.join((m.material.OUTPUT.pack(*row) for row in expected))
        return ((tile * ((count + 64) // 65))[:count * m.stride(entry)]
                + b'\xa5' * (63 * m.stride(entry)))

    def log(self, count=65537, entry='sample_buffer', device='.2'):
        nbytes = (count + 63) * m.stride(entry)
        return '\n'.join(
            f"measurement index={i} phase={'warmup' if i < 3 else 'sample'} "
            f"device_ms={device} host_submit_ns=100 host_until_stop_ns=200 "
            f"reset_bytes={nbytes} compared_bytes={nbytes} equal=true valid_time=true"
            for i in range(12)
        )

    def test_complete_fixed_inventory_and_reverse_order(self):
        self.assertEqual(len(m.fixed_cells()), 12)
        self.assertEqual(len(m.timed_cells()), 24)
        cells = m.timed_cells()
        self.assertEqual(
            [(r['entry'], r['count'], r['mode']) for r in cells[:12]],
            list(reversed([(r['entry'], r['count'], r['mode']) for r in cells[12:]]))
        )
        self.assertEqual(len({r['id'] for r in cells}), 24)
        self.assertEqual((m.WARMUPS, m.SAMPLES, m.ROUNDS), (3, 9, 2))

    def test_exact_input_tile_and_truncation(self):
        for entry in m.ENTRIES:
            tile = m.repeat_inputs(entry, 65)
            size = m.material.entry_layout(entry)['input_stride']
            for count in m.COUNTS:
                data = m.repeat_inputs(entry, count)
                self.assertEqual(len(data), count * size)
                self.assertEqual(data, (tile * ((count + 64) // 65))[:count * size])
        with self.assertRaises(ValueError):
            m.repeat_inputs('eval_buffer', 66)

    def test_all_active_outputs_and_guards(self):
        for entry in m.ENTRIES:
            for count in m.COUNTS:
                result = m.qualify_outputs(self.output(entry, count), entry, count)
                self.assertEqual(result['status'], 'passed')
                self.assertEqual(result['compared_bytes'], (count + 63) * m.stride(entry))
                self.assertEqual(
                    result['rejection_records'], count // 65 if entry == 'sample_buffer' else 0)

    def test_all_global_candidates(self):
        for model in m.material.SAMPLE_MODELS:
            self.assertEqual(
                m.qualify_outputs(
                    self.output('sample_buffer', 65537, model),
                    'sample_buffer',
                    65537
                )['status'],
                'passed'
            )

    def test_body_tail_missing_and_nonfinite_fail(self):
        count = 65537
        for entry in m.ENTRIES:
            output = self.output(entry, count)
            size = m.stride(entry)
            for offset in (0, 65 * size, 30000 * size, (count - 1) * size,
                           count * size, len(output) - 1):
                data = bytearray(output)
                data[offset] ^= 1
                self.assertEqual(m.qualify_outputs(bytes(data), entry, count)['status'], 'failed')
            self.assertEqual(m.qualify_outputs(output[:-1], entry, count)['status'], 'failed')
            data = bytearray(output)
            struct.pack_into('<f', data, 0, float('nan'))
            self.assertEqual(m.qualify_outputs(bytes(data), entry, count)['status'], 'failed')

    def test_later_rejection_and_model_mixing_fail(self):
        data = bytearray(self.output('sample_buffer', 65537))
        struct.pack_into('<I', data, (65 + 64) * 32, 2147483648)
        self.assertEqual(m.qualify_outputs(bytes(data), 'sample_buffer', 65537)['status'], 'failed')
        other = self.output('sample_buffer', 65537, 'fma')
        data = bytearray(self.output('sample_buffer', 65537))
        data[65 * 32:130 * 32] = other[65 * 32:130 * 32]
        self.assertEqual(m.qualify_outputs(bytes(data), 'sample_buffer', 65537)['status'], 'failed')

    def test_missing_duplicate_or_changed_cell_fails(self):
        wanted = m.fixed_cells()
        good = [dict(row, status='passed') for row in wanted]
        m.validate_report_cells(good, wanted)
        for broken in (
            good[:-1],
            good[:-1] + [good[0]],
            [dict(r, status='failed') if i == 0 else r for i, r in enumerate(good)],
            [dict(r, count=65) if i == 0 else r for i, r in enumerate(good)]
        ):
            with self.assertRaises(ValueError):
                m.validate_report_cells(broken, wanted)

    def test_reference_and_artifact_pins_fail_closed(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'reference.bin'
            path.write_bytes(b'qualified')
            pins = {str(path): m.sha(path)}
            m.verify_pins(pins)
            path.write_bytes(b'changed')
            with self.assertRaises(ValueError):
                m.verify_pins(pins)

    def test_all_warmups_and_samples_required(self):
        log = self.log()
        result = m.parse_measurements(log, 65537, 'sample_buffer')
        self.assertEqual(result['status'], 'passed')
        self.assertEqual(len(result['rows']), 12)
        for broken in (
            '\n'.join(log.splitlines()[1:]),
            log + '\n' + log.splitlines()[0],
            log.replace('index=11', 'index=99'),
            log.replace('equal=true', 'equal=false', 1),
            log.replace('reset_bytes=2099200', 'reset_bytes=0'),
            log.replace('compared_bytes=2099200', 'compared_bytes=0'),
            log.replace('device_ms=.2', 'device_ms=nan', 1)
        ):
            self.assertEqual(
                m.parse_measurements(broken, 65537, 'sample_buffer')['status'], 'failed')

    def test_prepare_requires_current_accepted_artifacts(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            helper = root / 'helper'
            helper.write_bytes(b'current helper')
            driver = root / 'accepted-driver.cpp'
            driver.write_bytes(b'old driver')
            accepted_hash = m.sha(driver)
            driver.write_bytes(b'changed driver')
            accepted = root / 'accepted.json'
            accepted.write_text(json.dumps(dict(
                status='passed', contract=m.material.CONTRACT,
                cells=[dict(id=mode) for mode in m.MODES],
                artifact_sha256={str(driver): accepted_hash}
            )))
            args = SimpleNamespace(
                helper=helper,
                eval_reference=accepted,
                sample_reference=accepted,
                output=root / 'prepare',
                timeout=120
            )
            with self.assertRaisesRegex(ValueError, 'changed pinned artifact'):
                m.prepare(args, {})

    def test_process_snapshots_mark_contamination_and_unknown_visibility(self):

        def completed(output, code=0):
            return SimpleNamespace(returncode=code, stdout=output)
        with patch.object(
            m.subprocess,
            'run',
            side_effect=[
                completed('device'),
                completed('pid, process_name, used_memory\n123, competing, 42')
            ]
        ):
            self.assertTrue(m.monitor()['competing_process_observed'])
        with patch.object(
            m.subprocess,
            'run',
            side_effect=[completed('device'), completed('pid, process_name, used_memory')]
        ):
            state = m.monitor()
            self.assertFalse(state['competing_process_observed'])
            self.assertEqual(state['process_visibility'], 'snapshot-only')
        with patch.object(
            m.subprocess,
            'run',
            side_effect=[completed('device'), completed('query unavailable', 1)]
        ):
            state = m.monitor()
            self.assertFalse(state['competing_process_observed'])
            self.assertEqual(state['process_visibility'], 'unavailable')

    def test_failed_cleanup_and_contamination_never_get_summary(self):
        runtime = '\n'.join((
            'cuda_header_version=12090 driver_api_version=13000 '
            'device_ordinal=0 device=NVIDIA L4 sm=89',
            'device_resources sm_count=58 l2_bytes=50331648',
            'texture=color handle_decimal=1 handle_hex=0x0000000000000001 '
            'low30_fit=true nonzero=true',
            'texture=roughness handle_decimal=2 handle_hex=0x0000000000000002 '
            'low30_fit=true nonzero=true',
            'execution launches=12 active=65537 output_capacity=65600 global_bytes=168 '
            'input_stride=24 output_stride=32 blocks=1025 phase=measure',
            'cleanup=PASS'
        ))
        for broken_cleanup, contaminated in ((True, False), (False, True)):
            with tempfile.TemporaryDirectory() as directory:
                args = SimpleNamespace(output=Path(directory), timeout=1)
                cell = dict(id='cell', entry='sample_buffer', count=65537, mode='nvrtc-o3')
                protocol = dict(
                    helper='helper',
                    artifact_sha256={'helper': 'digest'},
                    accepted={'sample_buffer': dict(
                        oracle_sha256='oracle',
                        modes={'nvrtc-o3': dict(cubin='shader', cubin_sha256='digest')}
                    )}
                )
                reference = dict(
                    reference=str(Path(directory) / 'qualified.bin'),
                    reference_sha256='digest',
                    runtime=dict(
                        header_version=12090,
                        driver_api_version=13000,
                        device='NVIDIA L4',
                        sm=89,
                        multiprocessors=58,
                        l2_bytes=50331648,
                        launches=1
                    )
                )
                log = self.log() + '\n' + runtime.replace(
                    'cleanup=PASS', 'cleanup=FAIL' if broken_cleanup else 'cleanup=PASS'
                )

                def execute(command, stdout, **kwargs):
                    stdout.write(log)
                    return SimpleNamespace(returncode=0)
                with patch.object(m, 'link_payloads', return_value={}), \
                     patch.object(m, 'verify_pins'), \
                     patch.object(m, 'sha', return_value='digest'), \
                     patch.object(m, 'monitor', return_value={
                         'competing_process_observed': contaminated
                     }), \
                     patch.object(m.subprocess, 'run', side_effect=execute):
                    if broken_cleanup:
                        with self.assertRaises(ValueError):
                            m.run_cell(args, protocol, cell, reference)
                    else:
                        m.run_cell(args, protocol, cell, reference)
                        self.assertEqual(cell['status'], 'contaminated')
                    self.assertIsNone(cell['summary'])
                    self.assertEqual(cell['measurements']['status'], 'passed')

    def test_no_warmup_in_summary_and_small_interval_no_throughput(self):
        result = m.parse_measurements(self.log(device='.05'), 65537, 'sample_buffer')
        summary = m.summaries(result, 65537)
        self.assertFalse(summary['all_intervals_at_least_0_1ms'])
        self.assertIsNone(summary['records_per_second'])
        result = m.parse_measurements(self.log(), 65537, 'sample_buffer')
        for row in result['rows'][:3]:
            row['device_ms'] = 1000
        self.assertEqual(m.summaries(result, 65537)['device_ms']['median'], 0.2)
        result['status'] = 'failed'
        self.assertIsNone(m.summaries(result, 65537))


if __name__ == '__main__':
    unittest.main()
