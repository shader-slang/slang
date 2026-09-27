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


class SampleContracts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.inputs=MATERIAL.sample_cases(); cls.expected=MATERIAL.sample_expected_outputs(cls.inputs)

    def encoded(self, model='float'):
        return b''.join(MATERIAL.SAMPLE_OUTPUT.pack(*row) for row in self.expected[model])+MATERIAL.SAMPLE_SENTINEL*63

    def test_independently_derived_ior_candidates(self):
        self.assertEqual(MATERIAL.sample_constants("float"),(397.9967956542969,0.0))
        self.assertEqual(MATERIAL.sample_constants("fma"),(397.9967956542969,0.3896454870700836))
        with self.assertRaises(ValueError): MATERIAL.sample_constants("unknown")

    def test_exact_rng_known_values(self):
        self.assertEqual(MATERIAL.sample_draws(0),[(x>>8)/16777216 for x in (1013904223,1196435762,3519870697,2868466484)])
        self.assertEqual(MATERIAL.sample_draws(0xffffffff)[0],(1012239698>>8)/16777216)

    def test_selection_and_estimator_perturbations(self):
        checks=MATERIAL.sample_oracle_checks(self.inputs,self.expected)
        self.assertGreater(min(checks['minimum_perturbation_tolerance_multiples'].values()),5)

    def test_coverage_and_branch_stability(self):
        self.assertEqual(len(self.inputs),65)
        self.assertEqual({(r['texel'],r['direction'],r['lobe']) for r in self.inputs[:24]},
                         {(t,d,l) for t in range(4) for d in range(3) for l in range(2)})
        for row in self.inputs[:64]:
            for model in MATERIAL.SAMPLE_MODELS:
                p=MATERIAL.sample_probability(tuple(map(MATERIAL.f32,MATERIAL.COLORS[row['texel']])),row['incoming'],model)
                self.assertGreater(abs(MATERIAL.sample_draws(row['seed'])[0]-p),.03)
                self.assertEqual(int(MATERIAL.sample_draws(row['seed'])[0]>=p),row['lobe'])

    def test_normal_incidence_closed_form(self):
        for row in self.inputs[:24]:
            if row['direction']!=0: continue
            a=MATERIAL.f32(MATERIAL.ROUGHNESS[row['texel']])**2
            z=1-MATERIAL.sample_draws(row['seed'])[2]*2/(1+a*a)
            # At normal incidence b=(1-a²)/(1+a²), so 1+b=2/(1+a²).
            expected_z=(1+z-a*a*(1-z))/(1+z+a*a*(1-z))
            got=MATERIAL.sample_direction(row['incoming'],MATERIAL.f32(MATERIAL.ROUGHNESS[row['texel']]),MATERIAL.sample_draws(row['seed']))
            self.assertAlmostEqual(got[2],expected_z,places=13)
            self.assertAlmostEqual(sum(v*v for v in got),1,places=13)

    def test_selected_color_ratios(self):
        for row,want in zip(self.inputs[:24],self.expected['float'][:24]):
            if row['lobe']==0:
                self.assertEqual(want[4],want[5]); self.assertEqual(want[5],want[6])
            else:
                color=MATERIAL.linear_color(tuple(map(MATERIAL.f32,MATERIAL.COLORS[row['texel']])))
                for a,b in ((0,1),(1,2)):
                    self.assertAlmostEqual(want[4+a]/want[4+b],color[a]/color[b],places=12)

    def test_abi_packing(self):
        self.assertEqual(MATERIAL.SAMPLE_INPUT.size,24); self.assertEqual(MATERIAL.SAMPLE_OUTPUT.size,32)
        data=MATERIAL.SAMPLE_INPUT.pack(.25,.75,.3,.4,1,0xdeadbeef)
        self.assertEqual(data[20:],b'\xef\xbe\xad\xde')
        data=MATERIAL.SAMPLE_OUTPUT.pack(1,2,3,4,5,6,7,2)
        self.assertEqual(data[28:],b'\x02\x00\x00\x00')

    def test_each_coherent_model_passes(self):
        for model in MATERIAL.SAMPLE_MODELS:
            result=MATERIAL.sample_compare_outputs(self.encoded(model),self.expected)
            self.assertEqual(result['status'],'passed')
            self.assertIn(model,result['matching_global_models'])

    def test_mixed_models_fail(self):
        # A record-level model choice is forbidden even if both models explain some records.
        data=bytearray(self.encoded())
        for index in range(65):
            if index%2:
                data[index*32:(index+1)*32]=MATERIAL.SAMPLE_OUTPUT.pack(*self.expected['fma'][index])
        self.assertEqual(MATERIAL.sample_compare_outputs(bytes(data),self.expected)['status'],'failed')

    def test_nonfinite_wrong_flag_zero_and_tail_fail(self):
        for index,component,value in ((0,0,math.nan),(0,3,math.inf),(0,4,0.),(0,7,0),(64,0,1e-30),(64,0,-0.)):
            data=bytearray(self.encoded()); row=list(MATERIAL.SAMPLE_OUTPUT.unpack_from(data,index*32)); row[component]=value
            MATERIAL.SAMPLE_OUTPUT.pack_into(data,index*32,*row)
            self.assertEqual(MATERIAL.sample_compare_outputs(bytes(data),self.expected)['status'],'failed')
        data=bytearray(self.encoded()); data[-1]=0
        self.assertEqual(MATERIAL.sample_compare_outputs(bytes(data),self.expected)['status'],'failed')

    def test_missing_outputs_or_expected_fail(self):
        self.assertEqual(MATERIAL.sample_compare_outputs(self.encoded()[:-1],self.expected)['status'],'failed')
        with self.assertRaises(ValueError): MATERIAL.sample_compare_outputs(self.encoded(),{'float':self.expected['float']})
        missing_flags={model:[row[:7] for row in rows] for model,rows in self.expected.items()}
        with self.assertRaises(ValueError): MATERIAL.sample_compare_outputs(self.encoded(),missing_flags)

    def test_repeated_and_wrapped_records_are_exact(self):
        for index in (24, 48, 52):
            data=bytearray(self.encoded())
            bits=struct.unpack_from("<I",data,index*32)[0]
            struct.pack_into("<I",data,index*32,bits+1)
            result=MATERIAL.sample_compare_outputs(bytes(data),self.expected)
            self.assertTrue(result["matching_global_models"])
            self.assertEqual(result["status"],"failed")

    def test_sample_ptx_layout_and_wrong_entry(self):
        text=""".target sm_80
.const .align 8 .b8 SLANG_globalParams[168];
.visible .entry sample_buffer() {
ld.const.u64 %a, [SLANG_globalParams+80];
ld.const.u64 %b, [SLANG_globalParams+128];
mul.wide.s32 %c, %i, 24;
ld.const.u64 %d, [SLANG_globalParams+144];
shl.b64 %e, %i, 5;
ld.const.u32 %f, [SLANG_globalParams+160];
}"""
        MATERIAL.validate_ptx(text,80,"sample_buffer")
        with self.assertRaises(ValueError): MATERIAL.validate_ptx(text,80,"eval_buffer")
        for broken in (text.replace(", 24;",", 40;"),text.replace("+144","+112"),
                       text.replace(", 5;",", 4;"),text.replace("sample_buffer()","sample_buffer(.param .u64 p)")):
            with self.assertRaises(ValueError): MATERIAL.validate_ptx(broken,80,"sample_buffer")

    def test_sample_execution_evidence(self):
        log="\n".join((
            "cuda_header_version=12090 driver_api_version=13000 device_ordinal=0 device=NVIDIA L4 sm=89",
            "texture=color handle_decimal=1 handle_hex=0x0000000000000001 low30_fit=true nonzero=true",
            "texture=roughness handle_decimal=2 handle_hex=0x0000000000000002 low30_fit=true nonzero=true",
            "execution launches=1 active=65 output_capacity=128 global_bytes=168 input_stride=24",
            "cleanup=PASS"))
        self.assertEqual(MATERIAL.validate_driver_log(log,"sample_buffer")["launches"],1)
        with self.assertRaises(ValueError): MATERIAL.validate_driver_log(log,"eval_buffer")
        for broken in (log.replace("input_stride=24","input_stride=40"),log.replace("cleanup=PASS","cleanup=FAIL"),
                       log.replace("launches=1","launches=0")):
            with self.assertRaises(ValueError): MATERIAL.validate_driver_log(broken,"sample_buffer")

    def test_preparation_cannot_cross_entries(self):
        modes=(("nvrtc",3),("nvvm",0),("nvvm",3))
        report=dict(status="prepared",contract=MATERIAL.SAMPLE_CONTRACT,source={"source_sha256":"source"},
                    cells=[dict(id=f"{backend}-o{optimization}",status="prepared",ptx_sha256="a"*64)
                           for backend,optimization in modes])
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/"reference.json"
            path.write_text(json.dumps(report))
            self.assertEqual(len(MATERIAL.reviewed_abi_hashes(path,"source",modes,"sample_buffer")),3)
            with self.assertRaises(ValueError): MATERIAL.reviewed_abi_hashes(path,"source",modes,"eval_buffer")

    def test_preparation_freezes_oracle_inputs_and_budget(self):
        with tempfile.TemporaryDirectory() as directory:
            path=Path(directory)/"reference.json"
            path.write_text(json.dumps({"oracle_sha256":"a"*64}))
            MATERIAL.validate_oracle_hash(path,"a"*64)
            for changed in ("b"*64,"", "invalid"):
                with self.assertRaises(ValueError): MATERIAL.validate_oracle_hash(path,changed)
            path.write_text("{}")
            with self.assertRaises(ValueError): MATERIAL.validate_oracle_hash(path,"a"*64)

    def test_early_rejection_exact_fields(self):
        for model in MATERIAL.SAMPLE_MODELS:
            self.assertEqual(self.expected[model][-1],(0.,)*7+(0,))


if __name__ == "__main__":
    unittest.main()
