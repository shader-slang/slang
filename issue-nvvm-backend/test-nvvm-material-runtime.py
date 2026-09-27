#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""CPU contracts for the fixed material oracle and fail-closed runtime validation."""

import contextlib
import importlib.util
import io
import json
import math
from pathlib import Path
import struct
import sys
import tempfile
import unittest
from unittest.mock import patch

REPO = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("material_runtime", REPO / "extras/validate-nvvm-material-runtime.py")
MATERIAL = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(MATERIAL)


class MaterialRuntimeContracts(unittest.TestCase):
    def setUp(self):
        self.inputs = MATERIAL.cases()
        self.expected = MATERIAL.expected_outputs(self.inputs)
        self.output = b"".join(struct.pack("<4f", *row) for row in self.expected) + b"\xa5" * (63 * 16)

    def test_normal_incidence_closed_form(self):
        # At normal incidence, D=1/(pi*r^4), G2=1, Fcoat=.04, Fmetal=.99.
        # Evaluate the x=1 fit as four independently collected quadratics in r.
        for texel in range(4):
            r = MATERIAL.f32(MATERIAL.ROUGHNESS[texel])
            a = (28.81 - 18.73*r + 6.297*r*r) / (28.195 - 16.317*r + 41.723*r*r)
            b = (.0355 - .083*r + .1515*r*r) / (14.3081 + 15.01*r + 56.36*r*r)
            energy = min(1, max(0, a)) + min(1, max(0, b))
            compensation = (1 - energy) / energy
            prefactor = 1 / (4 * math.pi * r**4)
            color = [MATERIAL.f32(value) for value in MATERIAL.COLORS[texel][:3]]
            linear = [v/12.92 if v <= .04045 else ((v+.055)/1.055)**2.4 for v in color]
            wanted = [prefactor * (.04*(1+.04*compensation) + v*.96*.99*(1+.99*compensation))
                      for v in linear]
            wanted.append((1+r**4) / (4*math.pi*r**4))
            for actual, expected in zip(self.expected[texel * 3], wanted):
                self.assertAlmostEqual(actual, expected, places=10)

    def test_oracle_perturbations_and_positive_outputs(self):
        result = MATERIAL.oracle_checks(self.inputs, self.expected)
        self.assertEqual(result["positive_finite_components"], 260)
        self.assertGreater(min(result["minimum_perturbation_tolerance_multiples"].values()), 100)
        self.assertEqual(self.expected[0], self.expected[12])
        self.assertNotEqual(self.inputs[0]["seed"], self.inputs[13]["seed"])
        self.assertEqual(self.expected[:13], self.expected[13:26])

    def test_uploaded_record_layout(self):
        data = MATERIAL.INPUT.pack(.25, .75, 1, 2, 3, 4, 5, 6, 0x12345678)
        self.assertEqual(len(data), 40)
        self.assertEqual(struct.unpack_from("<3f", data, 8), (1, 2, 3))
        self.assertEqual(struct.unpack_from("<3f", data, 20), (4, 5, 6))
        self.assertEqual(data[32:36], bytes.fromhex("78563412"))
        self.assertEqual(data[36:], b"\0" * 4)

    def test_correct_output_and_sentinels(self):
        result = MATERIAL.compare_outputs(self.output, self.expected)
        self.assertEqual(result["status"], "passed")
        self.assertEqual(result["compared_components"], 260)
        self.assertTrue(result["tail_sentinels_unchanged"])
        self.assertTrue(result["different_seed_outputs_identical"])

    def test_nonfinite_zero_and_wrong_outputs_fail(self):
        for value in (float("nan"), float("inf"), -float("inf"), 0, -1, 1e10):
            with self.subTest(value=value):
                data = struct.pack("<f", value) + self.output[4:]
                result = MATERIAL.compare_outputs(data, self.expected)
                self.assertEqual(result["status"], "failed")
                self.assertIn([0, 0], result["failed_components"])
                json.dumps(result, allow_nan=False)

    def test_missing_output_and_tail_writes_fail(self):
        self.assertEqual(MATERIAL.compare_outputs(self.output[:-1], self.expected)["status"], "failed")
        data = self.output[:65*16] + b"\0" + self.output[65*16+1:]
        result = MATERIAL.compare_outputs(data, self.expected)
        self.assertFalse(result["tail_sentinels_unchanged"])
        self.assertEqual(result["status"], "failed")

    def test_small_seed_dependent_change_is_detected(self):
        # A one-ULP change passes the numerical tolerance but violates deterministic evaluation.
        data = bytearray(self.output)
        bits = struct.unpack_from("<I", data, 13*16)[0]
        struct.pack_into("<I", data, 13*16, bits + 1)
        result = MATERIAL.compare_outputs(data, self.expected)
        self.assertFalse(result["different_seed_outputs_identical"])
        self.assertEqual(result["failed_components"], [])
        self.assertEqual(result["status"], "failed")

    def test_driver_evidence_rejects_false_execution(self):
        log = "\n".join((
            "cuda_header_version=12090 driver_api_version=13000 device_ordinal=0 device=NVIDIA L4 sm=89",
            "texture=color handle_decimal=1 handle_hex=0x0000000000000001 low30_fit=true nonzero=true",
            "texture=roughness handle_decimal=2 handle_hex=0x0000000000000002 low30_fit=true nonzero=true",
            "execution launches=1 active=65 output_capacity=128 global_bytes=168 input_stride=40",
            "cleanup=PASS"))
        self.assertEqual(MATERIAL.validate_driver_log(log)["launches"], 1)
        for broken in (log.replace("launches=1", "launches=0"), log.replace("cleanup=PASS", "cleanup=FAIL"),
                       log.replace("handle_decimal=1", "handle_decimal=1073741824"), log + "\ncleanup=PASS"):
            with self.assertRaises(ValueError):
                MATERIAL.validate_driver_log(broken)

    def test_ptx_abi_rejects_changed_contract(self):
        text = """.target sm_80
.const .align 8 .b8 SLANG_globalParams[168];
.visible .entry eval_buffer() {
ld.const.u64 %a, [SLANG_globalParams+80];
ld.const.u64 %b, [SLANG_globalParams+96];
mul.wide.s32 %c, %i, 40;
ld.const.u64 %d, [SLANG_globalParams+112];
shl.b64 %e, %i, 4;
ld.const.u32 %f, [SLANG_globalParams+160];
}"""
        MATERIAL.validate_ptx(text, 80)
        for broken in (text.replace("[168]", "[160]"), text.replace(", 40;", ", 36;"),
                       text.replace("eval_buffer()", "eval_buffer(.param .u64 pointer)"),
                       text.replace("+112", "+120"), text.replace(", 4;", ", 5;")):
            with self.assertRaises(ValueError):
                MATERIAL.validate_ptx(broken, 80)

    def test_abi_reference_requires_complete_source_matched_modes(self):
        modes = (("nvrtc", 3), ("nvvm", 0), ("nvvm", 3))
        good = dict(status="prepared", contract=MATERIAL.CONTRACT, source={"source_sha256": "source"},
                    cells=[dict(id=f"{backend}-o{optimization}", status="prepared", ptx_sha256="a"*64)
                           for backend, optimization in modes])
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / "reference.json"
            path.write_text(json.dumps(good))
            self.assertEqual(len(MATERIAL.reviewed_abi_hashes(path, "source", modes)), 3)
            with self.assertRaises(ValueError):
                MATERIAL.reviewed_abi_hashes(path, "changed-source", modes)
            good["cells"][2] = good["cells"][1]
            path.write_text(json.dumps(good))
            with self.assertRaises(ValueError):
                MATERIAL.reviewed_abi_hashes(path, "source", modes)

    def test_cli_refuses_overwrite(self):
        with tempfile.TemporaryDirectory() as directory:
            marker = Path(directory) / "preserve.txt"
            marker.write_text("prior failure")
            argv = ["validator", "--prepare-only", "--slangc", "missing", "--provider", "missing", "--cuda-root", "missing",
                    "--output", directory]
            with patch.object(sys, "argv", argv), contextlib.redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit) as error:
                    MATERIAL.main()
            self.assertEqual(error.exception.code, 2)
            self.assertEqual(marker.read_text(), "prior failure")

    def test_cli_records_unavailable_host_without_execution(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "new"
            argv = ["validator", "--prepare-only", "--slangc", "missing", "--provider", "missing", "--cuda-root", "missing",
                    "--output", str(output)]
            with patch.object(sys, "argv", argv), patch.object(MATERIAL.platform, "system", return_value="Darwin"), \
                    contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(MATERIAL.main(), 2)
            report = json.loads((output / "results.json").read_text())
            self.assertEqual(report["status"], "infrastructure-failed")
            self.assertEqual(report["cells"], [])


if __name__ == "__main__":
    unittest.main()
