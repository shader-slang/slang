#!/usr/bin/env python3
"""
Run repro for GitHub #9267: max() autodiff gradient depends on load pattern.
Requires: PyTorch, slangtorch, CUDA.

Usage:
  python tests/bugs/9267-autodiff-max-gradient-runner.py

Reveals the bug when gradients from loss_kernel_vec_max (loadVecOnce) differ
from loss_kernel_vec_max_manual (manual loadOnce) for the same inputs.
"""
from pathlib import Path
import sys

try:
    import torch
    import slangtorch
except ImportError as e:
    print("Need PyTorch and slangtorch:", e, file=sys.stderr)
    sys.exit(1)

SCRIPT_DIR = Path(__file__).resolve().parent
SLANG_FILE = SCRIPT_DIR / "9267-autodiff-max-gradient.slang"
THREADS = 256


def main():
    if not torch.cuda.is_available():
        print("CUDA is required", file=sys.stderr)
        sys.exit(1)

    if not SLANG_FILE.exists():
        print("Slang file not found:", SLANG_FILE, file=sys.stderr)
        sys.exit(1)

    module = slangtorch.loadModule(
        str(SLANG_FILE),
        verbose=False,
    )

    device = torch.device("cuda")
    # Same test data as #9267
    positions_base = torch.tensor(
        [
            [0.40, -0.20, 0.15],
            [-0.35, 0.50, -0.25],
            [0.10, 0.05, -0.45],
        ],
        dtype=torch.float32,
        device=device,
    )
    dims = torch.tensor(
        [
            [0.60, 0.50, 0.40],
            [0.70, 0.90, 0.80],
            [0.20, 0.30, 0.90],
        ],
        dtype=torch.float32,
        device=device,
    )
    n = positions_base.shape[0]
    scale = 1.0 / max(int(n), 1)

    class LossFn(torch.autograd.Function):
        @staticmethod
        def forward(ctx, positions, dims, kernel_name):
            positions = positions.contiguous()
            dims = dims.contiguous()
            n = positions.shape[0]
            scale = 1.0 / max(int(n), 1)
            loss = torch.empty((n,), dtype=positions.dtype, device=positions.device)
            blocks = (n + THREADS - 1) // THREADS
            kernel = getattr(module, kernel_name)
            kernel(
                count=n,
                invCount=scale,
                positions=positions,
                dims=dims,
                lossOut=loss,
            ).launchRaw(blockSize=(THREADS, 1, 1), gridSize=(blocks, 1, 1))
            ctx.save_for_backward(positions, dims, loss)
            ctx.n = n
            ctx.scale = scale
            ctx.kernel_name = kernel_name
            ctx.input_shape = positions.shape
            return loss

        @staticmethod
        def backward(ctx, grad_loss):
            positions, dims, loss = ctx.saved_tensors
            grad_positions = torch.zeros_like(positions)
            grad_loss = grad_loss.contiguous()
            blocks = (ctx.n + THREADS - 1) // THREADS
            kernel = getattr(module, ctx.kernel_name)
            kernel.bwd(
                count=ctx.n,
                invCount=ctx.scale,
                positions=(positions, grad_positions),
                dims=dims,
                lossOut=(loss, grad_loss),
            ).launchRaw(blockSize=(THREADS, 1, 1), gridSize=(blocks, 1, 1))
            return grad_positions.view(ctx.input_shape), None, None

    p_vec = positions_base.clone().detach().requires_grad_(True)
    p_manual = positions_base.clone().detach().requires_grad_(True)
    loss_vec = LossFn.apply(p_vec, dims, "loss_kernel_vec_max")
    loss_manual = LossFn.apply(p_manual, dims, "loss_kernel_vec_max_manual")

    loss_vec.sum().backward()
    loss_manual.sum().backward()

    g_vec = p_vec.grad.detach().cpu()
    g_manual = p_manual.grad.detach().cpu()

    # Reference: PyTorch relu
    p_ref = positions_base.clone().detach().requires_grad_(True)
    half = dims * 0.5
    exceed = p_ref.abs() - half
    loss_ref = torch.relu(exceed).sum(dim=-1) * scale
    loss_ref.sum().backward()
    g_ref = p_ref.grad.detach().cpu()

    diff_vec_ref = (g_vec - g_ref).abs().max().item()
    diff_manual_ref = (g_manual - g_ref).abs().max().item()
    diff_vec_vs_manual = (g_vec - g_manual).abs().max().item()

    print("Gradient comparison:")
    print("  vec_max vs reference (max abs error):", diff_vec_ref)
    print("  vec_max_manual vs reference (max abs error):", diff_manual_ref)
    print("  vec_max vs vec_max_manual (max abs error):", diff_vec_vs_manual)
    print()
    if diff_vec_vs_manual > 1e-5:
        print("BUG REPRODUCED: vec_max and vec_max_manual gradients differ (same math).")
        print("grad(vec_max):")
        print(g_vec)
        print("grad(vec_max_manual):")
        print(g_manual)
        sys.exit(1)
    else:
        print("Gradients match (bug not seen with this build/data).")
        sys.exit(0)


if __name__ == "__main__":
    main()
