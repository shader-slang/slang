#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Neural MLP Performance Benchmark: For-loop vs CoopVec vs CoopMat

Compares identical network architecture with three implementations:
  - For-loop: Scalar GPU loops (original SlangPy LinearLayer style)
  - CoopVec: coopVecMatMulAdd intrinsics (original SlangPy CoopVecLinearLayer style)
  - CoopMat: WaveTangledVector + FFLayer (current neural.slang module)

This benchmark tracks performance and guides future CoopMat optimizations.

Usage:
    python benchmark_neural_mlp.py [OPTIONS]

Options:
    --iterations N          Number of measured iterations (default: 100)
    --warmup N              Number of warmup iterations (default: 10)
    --batch-size N          Batch size for forward pass (default: 256)
    --json                  Output results in JSON format
    --forloop-only          Run only for-loop benchmark
    --coopvec-only          Run only CoopVec benchmark  
    --coopmat-only          Run only CoopMat benchmark
    --device TYPE           Device type: vulkan, d3d12, cuda (default: vulkan)
    --size SIZE             Network size: small, medium, large, xlarge (default: small)
    --sweep                 Run benchmark across all network sizes
"""

from __future__ import annotations

import argparse
import json
import sys
import time
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import Optional

import numpy as np

try:
    import slangpy as spy
    from slangpy.core.calldata import SLANG_PATH
except ImportError:
    print("Error: slangpy not found. Please install slangpy first.")
    sys.exit(1)

def find_neural_module_dir() -> Optional[str]:
    """Find the neural module directory (from slang build)."""
    # Look for neural.slang-module in standard locations
    test_dir = Path(__file__).resolve().parent
    
    # Common build locations relative to slang repo
    slang_repo = test_dir.parent.parent.parent.parent.parent  # tests/integration/slangpy/neural/benchmarks -> root
    
    possible_paths = [
        slang_repo / "build" / "Release" / "lib" / "slang-standard-module-2026.1",
        slang_repo / "build" / "Debug" / "lib" / "slang-standard-module-2026.1",
        slang_repo / "build.release" / "lib" / "slang-standard-module-2026.1",
        slang_repo / "build.debug" / "lib" / "slang-standard-module-2026.1",
    ]
    
    for path in possible_paths:
        if (path / "neural.slang-module").exists():
            return str(path)
    
    return None

# Network size configurations
NETWORK_SIZES = {
    "small": {"input": 64, "hidden": 64, "output": 16},      # 5,200 params
    "medium": {"input": 128, "hidden": 128, "output": 32},   # 20,640 params
    "large": {"input": 256, "hidden": 256, "output": 64},    # 82,240 params
    "xlarge": {"input": 512, "hidden": 512, "output": 128},  # 328,320 params
}

def get_network_config(size_name: str) -> dict:
    """Get network configuration for a given size."""
    config = NETWORK_SIZES[size_name]
    input_size = config["input"]
    hidden_size = config["hidden"]
    output_size = config["output"]
    
    layer1_weights = input_size * hidden_size
    layer1_biases = hidden_size
    layer2_weights = hidden_size * output_size
    layer2_biases = output_size
    total_params = layer1_weights + layer1_biases + layer2_weights + layer2_biases
    
    return {
        "input_size": input_size,
        "hidden_size": hidden_size,
        "output_size": output_size,
        "layer1_weights": layer1_weights,
        "layer1_biases": layer1_biases,
        "layer2_weights": hidden_size * output_size,
        "layer2_biases": output_size,
        "total_params": total_params,
        "layer1_weight_offset": 0,
        "layer1_bias_offset": layer1_weights,
        "layer2_weight_offset": layer1_weights + layer1_biases,
        "layer2_bias_offset": layer1_weights + layer1_biases + layer2_weights,
    }

# Default network configuration (small)
INPUT_SIZE = 64
HIDDEN_SIZE = 64
OUTPUT_SIZE = 16
LAYER1_WEIGHTS = INPUT_SIZE * HIDDEN_SIZE
LAYER1_BIASES = HIDDEN_SIZE
LAYER2_WEIGHTS = HIDDEN_SIZE * OUTPUT_SIZE
LAYER2_BIASES = OUTPUT_SIZE
TOTAL_PARAMS = LAYER1_WEIGHTS + LAYER1_BIASES + LAYER2_WEIGHTS + LAYER2_BIASES


@dataclass
class BenchmarkResult:
    """Results from a single benchmark run."""
    backend: str
    device_type: str
    batch_size: int
    iterations: int
    warmup_iterations: int
    total_time_ms: float
    avg_time_ms: float
    min_time_ms: float
    max_time_ms: float
    throughput_samples_per_sec: float
    success: bool
    error_message: Optional[str] = None


@dataclass
class BenchmarkComparison:
    """Comparison between For-loop, CoopVec, and CoopMat implementations."""
    forloop: Optional[BenchmarkResult]  # Scalar GPU loops
    coopvec: Optional[BenchmarkResult]  # Original CoopVec intrinsics
    coopmat: Optional[BenchmarkResult]  # Current neural module
    
    def get_coopvec_vs_coopmat_speedup(self) -> Optional[float]:
        """Get speedup ratio (coopvec_time / coopmat_time). >1 means CoopMat is faster."""
        if (self.coopvec and self.coopmat and 
            self.coopvec.success and self.coopmat.success and
            self.coopmat.avg_time_ms > 0):
            return self.coopvec.avg_time_ms / self.coopmat.avg_time_ms
        return None
    
    def get_forloop_vs_coopvec_speedup(self) -> Optional[float]:
        """Get speedup ratio (forloop_time / coopvec_time). >1 means CoopVec is faster."""
        if (self.forloop and self.coopvec and 
            self.forloop.success and self.coopvec.success and
            self.coopvec.avg_time_ms > 0):
            return self.forloop.avg_time_ms / self.coopvec.avg_time_ms
        return None
    
    def get_forloop_vs_coopmat_speedup(self) -> Optional[float]:
        """Get speedup ratio (forloop_time / coopmat_time). >1 means CoopMat is faster."""
        if (self.forloop and self.coopmat and 
            self.forloop.success and self.coopmat.success and
            self.coopmat.avg_time_ms > 0):
            return self.forloop.avg_time_ms / self.coopmat.avg_time_ms
        return None


def run_forloop_benchmark(
    device_type_str: str,
    batch_size: int,
    iterations: int,
    warmup: int,
    network_size: str = "small",
) -> BenchmarkResult:
    """Run for-loop benchmark using scalar GPU loops (original LinearLayer style)."""
    try:
        config = get_network_config(network_size)
        device_type_map = {
            "vulkan": spy.DeviceType.vulkan,
            "d3d12": spy.DeviceType.d3d12,
            "cuda": spy.DeviceType.cuda,
        }
        device_type = device_type_map[device_type_str.lower()]
        test_dir = Path(__file__).resolve().parent
        
        # Pass network size as preprocessor defines (dict format)
        defines = {
            "INPUT_SIZE": str(config['input_size']),
            "HIDDEN_SIZE": str(config['hidden_size']),
            "OUTPUT_SIZE": str(config['output_size']),
        }
        
        compiler_options = spy.SlangCompilerOptions({
            "include_paths": [test_dir, SLANG_PATH],
            "debug_info": spy.SlangDebugInfoLevel.standard,
            "defines": defines,
        })
        
        device = spy.Device(
            type=device_type,
            enable_debug_layers=False,
            compiler_options=compiler_options,
            label=f"neural-mlp-benchmark-forloop-{device_type_str}",
        )
        
        try:
            module = device.load_module("benchmark_neural_mlp_forloop.slang")
            
            # Create compute kernel
            forward_program = device.link_program(
                modules=[module],
                entry_points=[module.entry_point("compute_batch_forward")]
            )
            forward_kernel = device.create_compute_kernel(forward_program)
            
            # Initialize parameters using config
            rng = np.random.default_rng(42)
            params_data = np.zeros(config['total_params'], dtype=np.float32)
            
            # Layer 1 weights (He initialization)
            scale1 = np.sqrt(2.0 / config['input_size'])
            params_data[:config['layer1_weights']] = rng.standard_normal(config['layer1_weights']).astype(np.float32) * scale1
            
            # Layer 2 weights
            offset = config['layer1_weights'] + config['layer1_biases']
            scale2 = np.sqrt(2.0 / config['hidden_size'])
            params_data[offset:offset + config['layer2_weights']] = rng.standard_normal(config['layer2_weights']).astype(np.float32) * scale2
            
            inputs_data = rng.standard_normal(batch_size * config['input_size']).astype(np.float32)
            
            # Create buffers
            params_buf = device.create_buffer(
                data=params_data,
                usage=spy.BufferUsage.shader_resource,
            )
            inputs_buf = device.create_buffer(
                data=inputs_data,
                usage=spy.BufferUsage.shader_resource,
            )
            outputs_buf = device.create_buffer(
                data=np.zeros(batch_size * config['output_size'], dtype=np.float32),
                usage=spy.BufferUsage.unordered_access,
            )
            
            # Calculate thread count (one thread per sample, 64 threads per group)
            thread_count = ((batch_size + 63) // 64) * 64
            
            # Warmup
            for _ in range(warmup):
                forward_kernel.dispatch(
                    thread_count=[thread_count, 1, 1],
                    params=params_buf,
                    inputs=inputs_buf,
                    outputs=outputs_buf,
                    batch_size=batch_size,
                )
            device.wait_for_idle()
            
            # Measured iterations with GPU timestamps
            query_pool = device.create_query_pool(type=spy.QueryType.timestamp, count=iterations * 2)
            for i in range(iterations):
                command_encoder = device.create_command_encoder()
                command_encoder.write_timestamp(query_pool, i * 2)
                forward_kernel.dispatch(
                    thread_count=[thread_count, 1, 1],
                    command_encoder=command_encoder,
                    params=params_buf,
                    inputs=inputs_buf,
                    outputs=outputs_buf,
                    batch_size=batch_size,
                )
                command_encoder.write_timestamp(query_pool, i * 2 + 1)
                device.submit_command_buffer(command_encoder.finish())
            device.wait()
            
            # Get GPU timestamps and convert to ms
            queries = np.array(query_pool.get_results(0, iterations * 2))
            frequency = float(device.info.timestamp_frequency)
            times = list((queries[1::2] - queries[0::2]) / frequency * 1000.0)
            
            total_time = sum(times)
            avg_time = total_time / iterations
            min_time = min(times)
            max_time = max(times)
            throughput = (batch_size * iterations) / (total_time / 1000)
            
            return BenchmarkResult(
                backend=f"For-loop ({network_size})",
                device_type=device_type.name,
                batch_size=batch_size,
                iterations=iterations,
                warmup_iterations=warmup,
                total_time_ms=total_time,
                avg_time_ms=avg_time,
                min_time_ms=min_time,
                max_time_ms=max_time,
                throughput_samples_per_sec=throughput,
                success=True,
            )
        finally:
            device.close()
            
    except Exception as e:
        import traceback
        return BenchmarkResult(
            backend=f"For-loop ({network_size})",
            device_type=device_type_str,
            batch_size=batch_size,
            iterations=iterations,
            warmup_iterations=warmup,
            total_time_ms=0,
            avg_time_ms=0,
            min_time_ms=0,
            max_time_ms=0,
            throughput_samples_per_sec=0,
            success=False,
            error_message=f"{str(e)}\n{traceback.format_exc()}",
        )


def run_coopvec_benchmark(
    device_type_str: str,
    batch_size: int,
    iterations: int,
    warmup: int,
    network_size: str = "small",
) -> BenchmarkResult:
    """Run CoopVec benchmark using coopVecMatMulAdd intrinsics (original CoopVecLinearLayer style)."""
    try:
        config = get_network_config(network_size)
        device_type_map = {
            "vulkan": spy.DeviceType.vulkan,
            "d3d12": spy.DeviceType.d3d12,
            "cuda": spy.DeviceType.cuda,
        }
        device_type = device_type_map[device_type_str.lower()]
        test_dir = Path(__file__).resolve().parent
        
        # Pass network size as preprocessor defines (dict format)
        defines = {
            "INPUT_SIZE": str(config['input_size']),
            "HIDDEN_SIZE": str(config['hidden_size']),
            "OUTPUT_SIZE": str(config['output_size']),
        }
        
        compiler_options = spy.SlangCompilerOptions({
            "include_paths": [test_dir, SLANG_PATH],
            "debug_info": spy.SlangDebugInfoLevel.standard,
            "enable_experimental_features": True,
            "defines": defines,
        })
        
        device = spy.Device(
            type=device_type,
            enable_debug_layers=False,
            compiler_options=compiler_options,
            label=f"neural-mlp-benchmark-coopvec-{device_type_str}",
        )
        
        try:
            # Check for cooperative vector support
            if not device.has_feature(spy.Feature.cooperative_vector):
                return BenchmarkResult(
                    backend=f"CoopVec ({network_size})",
                    device_type=device_type.name,
                    batch_size=batch_size,
                    iterations=iterations,
                    warmup_iterations=warmup,
                    total_time_ms=0,
                    avg_time_ms=0,
                    min_time_ms=0,
                    max_time_ms=0,
                    throughput_samples_per_sec=0,
                    success=False,
                    error_message="Cooperative vector not supported on this device",
                )
            
            module = device.load_module("benchmark_neural_mlp_coopvec.slang")
            
            # Create compute kernel
            forward_program = device.link_program(
                modules=[module],
                entry_points=[module.entry_point("compute_batch_forward")]
            )
            forward_kernel = device.create_compute_kernel(forward_program)
            
            # Initialize parameters (half precision for CoopVec)
            rng = np.random.default_rng(42)
            
            # Separate buffers for weights and biases
            scale1 = np.sqrt(2.0 / config['input_size'])
            layer1_weights_data = rng.standard_normal(config['layer1_weights']).astype(np.float16) * scale1
            layer1_biases_data = np.zeros(config['layer1_biases'], dtype=np.float16)
            
            scale2 = np.sqrt(2.0 / config['hidden_size'])
            layer2_weights_data = rng.standard_normal(config['layer2_weights']).astype(np.float16) * scale2
            layer2_biases_data = np.zeros(config['layer2_biases'], dtype=np.float16)
            
            inputs_data = rng.standard_normal(batch_size * config['input_size']).astype(np.float16)
            
            # Create buffers
            buffer_usage = spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access
            
            layer1_weights_buf = device.create_buffer(data=layer1_weights_data, usage=buffer_usage)
            layer1_biases_buf = device.create_buffer(data=layer1_biases_data, usage=buffer_usage)
            layer2_weights_buf = device.create_buffer(data=layer2_weights_data, usage=buffer_usage)
            layer2_biases_buf = device.create_buffer(data=layer2_biases_data, usage=buffer_usage)
            inputs_buf = device.create_buffer(data=inputs_data, usage=spy.BufferUsage.shader_resource)
            outputs_buf = device.create_buffer(
                data=np.zeros(batch_size * config['output_size'], dtype=np.float16),
                usage=spy.BufferUsage.unordered_access,
            )
            
            # Create network params struct with device addresses
            network_params = {
                "layer1_weights": layer1_weights_buf.device_address,
                "layer1_biases": layer1_biases_buf.device_address,
                "layer2_weights": layer2_weights_buf.device_address,
                "layer2_biases": layer2_biases_buf.device_address,
            }
            
            # Dispatch one thread group (32 threads) per sample
            num_groups = batch_size
            
            # Warmup
            for _ in range(warmup):
                forward_kernel.dispatch(
                    thread_count=[num_groups * 32, 1, 1],
                    network=network_params,
                    inputs=inputs_buf,
                    outputs=outputs_buf,
                    batch_size=batch_size,
                )
            device.wait_for_idle()
            
            # Measured iterations with GPU timestamps
            query_pool = device.create_query_pool(type=spy.QueryType.timestamp, count=iterations * 2)
            for i in range(iterations):
                command_encoder = device.create_command_encoder()
                command_encoder.write_timestamp(query_pool, i * 2)
                forward_kernel.dispatch(
                    thread_count=[num_groups * 32, 1, 1],
                    command_encoder=command_encoder,
                    network=network_params,
                    inputs=inputs_buf,
                    outputs=outputs_buf,
                    batch_size=batch_size,
                )
                command_encoder.write_timestamp(query_pool, i * 2 + 1)
                device.submit_command_buffer(command_encoder.finish())
            device.wait()
            
            # Get GPU timestamps and convert to ms
            queries = np.array(query_pool.get_results(0, iterations * 2))
            frequency = float(device.info.timestamp_frequency)
            times = list((queries[1::2] - queries[0::2]) / frequency * 1000.0)
            
            total_time = sum(times)
            avg_time = total_time / iterations
            min_time = min(times)
            max_time = max(times)
            throughput = (batch_size * iterations) / (total_time / 1000)
            
            return BenchmarkResult(
                backend=f"CoopVec ({network_size})",
                device_type=device_type.name,
                batch_size=batch_size,
                iterations=iterations,
                warmup_iterations=warmup,
                total_time_ms=total_time,
                avg_time_ms=avg_time,
                min_time_ms=min_time,
                max_time_ms=max_time,
                throughput_samples_per_sec=throughput,
                success=True,
            )
        finally:
            device.close()
            
    except Exception as e:
        import traceback
        return BenchmarkResult(
            backend=f"CoopVec ({network_size})",
            device_type=device_type_str,
            batch_size=batch_size,
            iterations=iterations,
            warmup_iterations=warmup,
            total_time_ms=0,
            avg_time_ms=0,
            min_time_ms=0,
            max_time_ms=0,
            throughput_samples_per_sec=0,
            success=False,
            error_message=f"{str(e)}\n{traceback.format_exc()}",
        )


def run_coopmat_benchmark(
    device_type_str: str,
    batch_size: int,
    iterations: int,
    warmup: int,
    network_size: str = "small",
) -> BenchmarkResult:
    """Run CoopMat benchmark using WaveTangledVector (current neural module)."""
    try:
        config = get_network_config(network_size)
        device_type_map = {
            "vulkan": spy.DeviceType.vulkan,
            "d3d12": spy.DeviceType.d3d12,
            "cuda": spy.DeviceType.cuda,
        }
        device_type = device_type_map[device_type_str.lower()]
        test_dir = Path(__file__).resolve().parent
        
        # Pass network size as preprocessor defines (dict format)
        defines = {
            "INPUT_SIZE": str(config['input_size']),
            "HIDDEN_SIZE": str(config['hidden_size']),
            "OUTPUT_SIZE": str(config['output_size']),
        }
        
        # Find neural module directory for CoopMat
        neural_module_dir = find_neural_module_dir()
        if neural_module_dir is None:
            return BenchmarkResult(
                backend=f"CoopMat ({network_size})",
                device_type=device_type_str,
                batch_size=batch_size,
                iterations=iterations,
                warmup_iterations=warmup,
                total_time_ms=0,
                avg_time_ms=0,
                min_time_ms=0,
                max_time_ms=0,
                throughput_samples_per_sec=0,
                success=False,
                error_message="Neural module not found - build slang first",
            )
        
        include_paths = [test_dir, neural_module_dir, SLANG_PATH]
        compiler_options = spy.SlangCompilerOptions({
            "include_paths": include_paths,
            "debug_info": spy.SlangDebugInfoLevel.standard,
            "enable_experimental_features": True,
            "defines": defines,
        })
        
        device = spy.Device(
            type=device_type,
            enable_debug_layers=False,
            compiler_options=compiler_options,
            label=f"neural-mlp-benchmark-coopmat-{device_type_str}",
        )
        
        try:
            # Check for cooperative matrix support
            if not device.has_feature(spy.Feature.cooperative_matrix):
                return BenchmarkResult(
                    backend=f"CoopMat ({network_size})",
                    device_type=device_type.name,
                    batch_size=batch_size,
                    iterations=iterations,
                    warmup_iterations=warmup,
                    total_time_ms=0,
                    avg_time_ms=0,
                    min_time_ms=0,
                    max_time_ms=0,
                    throughput_samples_per_sec=0,
                    success=False,
                    error_message="Cooperative matrix not supported on this device",
                )
            
            module = device.load_module("benchmark_neural_mlp_coopmat_fp16.slang")
            
            # Create compute kernel for batch forward
            forward_program = device.link_program(
                modules=[module],
                entry_points=[module.entry_point("compute_batch_forward")]
            )
            forward_kernel = device.create_compute_kernel(forward_program)
            
            # Initialize parameters using config (fp16 for fair comparison with Tin)
            rng = np.random.default_rng(42)
            params_data = np.zeros(config['total_params'], dtype=np.float16)
            
            # Layer 1 weights (He initialization)
            scale1 = np.sqrt(2.0 / config['input_size'])
            params_data[:config['layer1_weights']] = rng.standard_normal(config['layer1_weights']).astype(np.float16) * scale1
            
            # Layer 2 weights
            offset = config['layer1_weights'] + config['layer1_biases']
            scale2 = np.sqrt(2.0 / config['hidden_size'])
            params_data[offset:offset + config['layer2_weights']] = rng.standard_normal(config['layer2_weights']).astype(np.float16) * scale2
            
            inputs_data = rng.standard_normal(batch_size * config['input_size']).astype(np.float16)
            
            # Create buffers (fp16)
            params_buf = device.create_buffer(
                data=params_data,
                usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
            )
            inputs_buf = device.create_buffer(
                data=inputs_data,
                usage=spy.BufferUsage.shader_resource,
            )
            outputs_buf = device.create_buffer(
                data=np.zeros(batch_size * config['output_size'], dtype=np.float16),
                usage=spy.BufferUsage.unordered_access,
            )
            
            # Dispatch one thread group (32 threads) per sample
            num_groups = batch_size
            
            # Warmup
            for _ in range(warmup):
                forward_kernel.dispatch(
                    thread_count=[num_groups * 32, 1, 1],
                    params=params_buf,
                    inputs=inputs_buf,
                    outputs=outputs_buf,
                    batch_size=batch_size,
                )
            device.wait_for_idle()
            
            # Measured iterations with GPU timestamps
            query_pool = device.create_query_pool(type=spy.QueryType.timestamp, count=iterations * 2)
            for i in range(iterations):
                command_encoder = device.create_command_encoder()
                command_encoder.write_timestamp(query_pool, i * 2)
                forward_kernel.dispatch(
                    thread_count=[num_groups * 32, 1, 1],
                    command_encoder=command_encoder,
                    params=params_buf,
                    inputs=inputs_buf,
                    outputs=outputs_buf,
                    batch_size=batch_size,
                )
                command_encoder.write_timestamp(query_pool, i * 2 + 1)
                device.submit_command_buffer(command_encoder.finish())
            device.wait()
            
            # Get GPU timestamps and convert to ms
            queries = np.array(query_pool.get_results(0, iterations * 2))
            frequency = float(device.info.timestamp_frequency)
            times = list((queries[1::2] - queries[0::2]) / frequency * 1000.0)
            
            total_time = sum(times)
            avg_time = total_time / iterations
            min_time = min(times)
            max_time = max(times)
            throughput = (batch_size * iterations) / (total_time / 1000)
            
            return BenchmarkResult(
                backend=f"CoopMat ({network_size})",
                device_type=device_type.name,
                batch_size=batch_size,
                iterations=iterations,
                warmup_iterations=warmup,
                total_time_ms=total_time,
                avg_time_ms=avg_time,
                min_time_ms=min_time,
                max_time_ms=max_time,
                throughput_samples_per_sec=throughput,
                success=True,
            )
        finally:
            device.close()
            
    except Exception as e:
        import traceback
        return BenchmarkResult(
            backend=f"CoopMat ({network_size})",
            device_type=device_type_str,
            batch_size=batch_size,
            iterations=iterations,
            warmup_iterations=warmup,
            total_time_ms=0,
            avg_time_ms=0,
            min_time_ms=0,
            max_time_ms=0,
            throughput_samples_per_sec=0,
            success=False,
            error_message=f"{str(e)}\n{traceback.format_exc()}",
        )


def print_table_results(comparison: BenchmarkComparison, network_size: str = "small"):
    """Print benchmark results as a formatted table."""
    config = get_network_config(network_size)
    print("\n" + "=" * 110)
    print("Neural MLP Performance Benchmark: For-loop vs CoopVec vs CoopMat")
    print("=" * 110)
    print(f"Network: {config['input_size']} -> {config['hidden_size']} -> {config['output_size']} (2-layer MLP with LeakyReLU)")
    print(f"Total Parameters: {config['total_params']}")
    print("")
    print("Backends (all run on GPU):")
    print("  - For-loop: Scalar loops (original SlangPy LinearLayer style)")
    print("  - CoopVec:  coopVecMatMulAdd intrinsics (original SlangPy CoopVecLinearLayer style)")
    print("  - CoopMat:  WaveTangledVector + FFLayer (current neural.slang module)")
    print("-" * 110)
    
    def format_result(result: Optional[BenchmarkResult], metric: str) -> str:
        if result is None:
            return "N/A"
        if not result.success:
            return "FAILED"
        
        metric_map = {
            "Device": result.device_type,
            "Batch Size": str(result.batch_size),
            "Iterations": str(result.iterations),
            "Warmup": str(result.warmup_iterations),
            "Total Time (ms)": f"{result.total_time_ms:.2f}",
            "Avg Time (ms)": f"{result.avg_time_ms:.4f}",
            "Min Time (ms)": f"{result.min_time_ms:.4f}",
            "Max Time (ms)": f"{result.max_time_ms:.4f}",
            "Throughput (samples/s)": f"{result.throughput_samples_per_sec:.0f}",
        }
        return metric_map.get(metric, "N/A")
    
    print(f"\n{'Metric':<25} {'For-loop':<25} {'CoopVec':<25} {'CoopMat':<25}")
    print("-" * 100)
    
    metrics = [
        "Device", "Batch Size", "Iterations", "Warmup",
        "Total Time (ms)", "Avg Time (ms)", "Min Time (ms)", "Max Time (ms)",
        "Throughput (samples/s)"
    ]
    
    for metric in metrics:
        forloop_val = format_result(comparison.forloop, metric)
        coopvec_val = format_result(comparison.coopvec, metric)
        coopmat_val = format_result(comparison.coopmat, metric)
        print(f"{metric:<25} {forloop_val:<25} {coopvec_val:<25} {coopmat_val:<25}")
    
    print("-" * 100)
    
    # Print error messages if any
    if comparison.forloop and not comparison.forloop.success:
        print(f"\nFor-loop Error: {comparison.forloop.error_message[:300]}...")
    if comparison.coopvec and not comparison.coopvec.success:
        print(f"\nCoopVec Error: {comparison.coopvec.error_message[:300]}...")
    if comparison.coopmat and not comparison.coopmat.success:
        print(f"\nCoopMat Error: {comparison.coopmat.error_message[:300]}...")
    
    print("\n--- Performance Comparison ---")
    
    # For-loop vs CoopVec
    speedup_fl_cv = comparison.get_forloop_vs_coopvec_speedup()
    if speedup_fl_cv is not None:
        print(f"\nFor-loop vs CoopVec:")
        if speedup_fl_cv > 1.0:
            print(f"  CoopVec is {speedup_fl_cv:.2f}x faster than For-loop")
        elif speedup_fl_cv < 1.0:
            print(f"  CoopVec is {1/speedup_fl_cv:.2f}x slower than For-loop")
        else:
            print(f"  Similar performance")
    
    # CoopVec vs CoopMat (the key comparison)
    speedup_cv_cm = comparison.get_coopvec_vs_coopmat_speedup()
    if speedup_cv_cm is not None:
        print(f"\nCoopVec vs CoopMat (Original vs Current):")
        if speedup_cv_cm > 1.0:
            print(f"  CoopMat is {speedup_cv_cm:.2f}x FASTER than CoopVec")
        elif speedup_cv_cm < 1.0:
            print(f"  *** CoopMat is {1/speedup_cv_cm:.2f}x SLOWER than CoopVec (needs optimization) ***")
        else:
            print(f"  Similar performance")
    
    # For-loop vs CoopMat
    speedup_fl_cm = comparison.get_forloop_vs_coopmat_speedup()
    if speedup_fl_cm is not None:
        print(f"\nFor-loop vs CoopMat:")
        if speedup_fl_cm > 1.0:
            print(f"  CoopMat is {speedup_fl_cm:.2f}x faster than For-loop")
        elif speedup_fl_cm < 1.0:
            print(f"  CoopMat is {1/speedup_fl_cm:.2f}x slower than For-loop")
        else:
            print(f"  Similar performance")
    
    print("\n" + "=" * 110 + "\n")


def print_json_results(comparison: BenchmarkComparison):
    """Print benchmark results as JSON."""
    result = {
        "network": {
            "input_size": INPUT_SIZE,
            "hidden_size": HIDDEN_SIZE,
            "output_size": OUTPUT_SIZE,
            "total_params": TOTAL_PARAMS,
        },
        "forloop": asdict(comparison.forloop) if comparison.forloop else None,
        "coopvec": asdict(comparison.coopvec) if comparison.coopvec else None,
        "coopmat": asdict(comparison.coopmat) if comparison.coopmat else None,
        "speedup_forloop_vs_coopvec": comparison.get_forloop_vs_coopvec_speedup(),
        "speedup_coopvec_vs_coopmat": comparison.get_coopvec_vs_coopmat_speedup(),
        "speedup_forloop_vs_coopmat": comparison.get_forloop_vs_coopmat_speedup(),
        "coopmat_needs_optimization": (comparison.get_coopvec_vs_coopmat_speedup() or 0) < 1.0,
    }
    print(json.dumps(result, indent=2))


def run_single_size_benchmark(args, network_size: str):
    """Run benchmark for a single network size."""
    config = get_network_config(network_size)
    
    if not args.json:
        print(f"\nNeural MLP Performance Benchmark")
        print(f"Network: {config['input_size']} -> {config['hidden_size']} -> {config['output_size']} ({config['total_params']} params)")
        print(f"Device: {args.device}")
    
    forloop_result = None
    coopvec_result = None
    coopmat_result = None
    
    # Determine which benchmarks to run
    run_forloop = not (args.coopvec_only or args.coopmat_only)
    run_coopvec = not (args.forloop_only or args.coopmat_only)
    run_coopmat = not (args.forloop_only or args.coopvec_only)
    
    # Run For-loop benchmark
    if run_forloop:
        if not args.json:
            print(f"\nRunning For-loop benchmark...")
        forloop_result = run_forloop_benchmark(
            args.device, args.batch_size, args.iterations, args.warmup, network_size
        )
        if not args.json:
            if forloop_result.success:
                print(f"  Completed: avg {forloop_result.avg_time_ms:.4f} ms/iteration")
            else:
                print(f"  FAILED: {forloop_result.error_message[:100]}...")
    
    # Run CoopVec benchmark
    if run_coopvec:
        if not args.json:
            print(f"\nRunning CoopVec benchmark...")
        coopvec_result = run_coopvec_benchmark(
            args.device, args.batch_size, args.iterations, args.warmup, network_size
        )
        if not args.json:
            if coopvec_result.success:
                print(f"  Completed: avg {coopvec_result.avg_time_ms:.4f} ms/iteration")
            else:
                print(f"  FAILED: {coopvec_result.error_message[:100]}...")
    
    # Run CoopMat benchmark
    if run_coopmat:
        if not args.json:
            print(f"\nRunning CoopMat benchmark...")
        coopmat_result = run_coopmat_benchmark(
            args.device, args.batch_size, args.iterations, args.warmup, network_size
        )
        if not args.json:
            if coopmat_result.success:
                print(f"  Completed: avg {coopmat_result.avg_time_ms:.4f} ms/iteration")
            else:
                print(f"  FAILED: {coopmat_result.error_message[:100]}...")
    
    return BenchmarkComparison(
        forloop=forloop_result,
        coopvec=coopvec_result,
        coopmat=coopmat_result,
    )


def print_sweep_results(all_results: dict):
    """Print results from a sweep across network sizes."""
    print("\n" + "=" * 140)
    print("Neural MLP Performance Sweep: For-loop vs CoopVec vs CoopMat")
    print("=" * 140)
    print("\nAll backends run on GPU. Times in ms per iteration.")
    print("-" * 140)
    print(f"{'Size':<10} {'Network':<18} {'Params':<10} {'For-loop':<12} {'CoopVec':<12} {'CoopMat':<12} {'CoopMat vs For-loop':<22} {'CoopMat vs CoopVec':<20}")
    print("-" * 140)
    
    for size_name, comparison in all_results.items():
        config = get_network_config(size_name)
        network_str = f"{config['input_size']}->{config['hidden_size']}->{config['output_size']}"
        
        forloop_time = f"{comparison.forloop.avg_time_ms:.4f}" if comparison.forloop and comparison.forloop.success else "FAILED"
        coopvec_time = f"{comparison.coopvec.avg_time_ms:.4f}" if comparison.coopvec and comparison.coopvec.success else "FAILED"
        coopmat_time = f"{comparison.coopmat.avg_time_ms:.4f}" if comparison.coopmat and comparison.coopmat.success else "FAILED"
        
        # CoopMat vs For-loop comparison
        speedup_fl = comparison.get_forloop_vs_coopmat_speedup()
        if speedup_fl and speedup_fl < 1.0:
            speedup_fl_str = f"{1/speedup_fl:.2f}x slower"
        elif speedup_fl and speedup_fl > 1.0:
            speedup_fl_str = f"{speedup_fl:.2f}x faster"
        else:
            speedup_fl_str = "N/A"
        
        # CoopMat vs CoopVec comparison
        speedup_cv = comparison.get_coopvec_vs_coopmat_speedup()
        if speedup_cv and speedup_cv < 1.0:
            speedup_cv_str = f"{1/speedup_cv:.2f}x slower"
        elif speedup_cv and speedup_cv > 1.0:
            speedup_cv_str = f"{speedup_cv:.2f}x faster"
        else:
            speedup_cv_str = "N/A"
        
        print(f"{size_name:<10} {network_str:<18} {config['total_params']:<10} {forloop_time:<12} {coopvec_time:<12} {coopmat_time:<12} {speedup_fl_str:<22} {speedup_cv_str:<20}")
    
    print("-" * 140)
    print("\nCoopMat vs X: >1x slower means CoopMat takes more time than X")
    print("=" * 140 + "\n")


def main():
    parser = argparse.ArgumentParser(
        description="Neural MLP Benchmark: For-loop vs CoopVec vs CoopMat",
    )
    parser.add_argument("--iterations", type=int, default=100)
    parser.add_argument("--warmup", type=int, default=10)
    parser.add_argument("--batch-size", type=int, default=256)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--forloop-only", action="store_true")
    parser.add_argument("--coopvec-only", action="store_true")
    parser.add_argument("--coopmat-only", action="store_true")
    parser.add_argument("--device", type=str, default="vulkan", choices=["vulkan", "d3d12", "cuda"])
    parser.add_argument("--size", type=str, default="small", choices=["small", "medium", "large", "xlarge"],
                        help="Network size: small (64->64->16), medium (128->128->32), large (256->256->64), xlarge (512->512->128)")
    parser.add_argument("--sweep", action="store_true", help="Run benchmark across all network sizes")
    
    args = parser.parse_args()
    
    if args.sweep:
        # Run benchmark for all sizes
        all_results = {}
        for size_name in ["small", "medium", "large", "xlarge"]:
            if not args.json:
                print(f"\n{'='*60}")
                print(f"Running {size_name.upper()} network...")
                print(f"{'='*60}")
            all_results[size_name] = run_single_size_benchmark(args, size_name)
        
        # Print summary
        if not args.json:
            print_sweep_results(all_results)
        else:
            # JSON output for sweep
            result = {
                "sweep": True,
                "sizes": {
                    name: {
                        "network": get_network_config(name),
                        "forloop": asdict(comp.forloop) if comp.forloop else None,
                        "coopvec": asdict(comp.coopvec) if comp.coopvec else None,
                        "coopmat": asdict(comp.coopmat) if comp.coopmat else None,
                        "speedup_coopvec_vs_coopmat": comp.get_coopvec_vs_coopmat_speedup(),
                    }
                    for name, comp in all_results.items()
                }
            }
            print(json.dumps(result, indent=2))
    else:
        # Run benchmark for single size
        comparison = run_single_size_benchmark(args, args.size)
        
        # Output results
        if args.json:
            print_json_results(comparison)
        else:
            print_table_results(comparison, args.size)


if __name__ == "__main__":
    main()
