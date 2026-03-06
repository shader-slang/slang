"""Sweep workgroup sizes for CoopMat MLP test. Run each size in a subprocess.

Usage: run from the slang repo root:
  ../slangpy/.venv/bin/python3 tests/integration/slangpy/neural-large-model/sweep_workgroup.py
"""
import os
import subprocess
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "..", "..", "..", ".."))

for wg in [32, 64, 128, 256, 512, 1024]:
    print(f"\n===== WORKGROUP_SIZE={wg} =====", flush=True)
    r = subprocess.run(
        [sys.executable, "-c", f"""
import numpy as np, slangpy as spy, math, sys
from pathlib import Path
sys.path.insert(0, str(Path('tests/integration/slangpy/neural-large-model')))
from conftest import get_slangpy_paths, REF_IMAGE_PATH
from PIL import Image as PILImage

WG={wg}
def kaiming(layers):
    p=[]
    for fi,fo in layers:
        a=math.sqrt(6.0/fi); p.append(np.random.uniform(-a,a,fo*fi).astype('float32')); p.append(np.zeros(fo,dtype='float32'))
    return np.concatenate(p)
def cd(a,b): return (a+b-1)//b

img=PILImage.open(str(REF_IMAGE_PATH)).convert('RGB')
ref=np.array(img,dtype='float32')/255.0; h,w=ref.shape[:2]
paths=get_slangpy_paths(); TD=Path('tests/integration/slangpy/neural-large-model')

dev=spy.Device(type=spy.DeviceType.cuda,compiler_options=spy.SlangCompilerOptions({{'include_paths':paths,'defines':{{'WORKGROUP_SIZE':str(WG)}}}}))
rm=dev.load_module(str(TD/'neural_mlp_gpu_coopmat.slang'))
def mk(n): return dev.create_compute_kernel(dev.link_program(modules=[rm],entry_points=[rm.entry_point(n)]))
kg,kl,ko=mk('compute_calculate_grads'),mk('compute_show_loss'),mk('compute_optimizer_step')
print(f'  Compiled OK',flush=True)

np.random.seed(42)
mp=kaiming([(4,128),(128,128),(128,128),(128,3)]); tm=len(mp)
ln=np.random.uniform(0,1,(32,32,4)).astype('float32'); tl=ln.size
u=spy.BufferUsage.shader_resource|spy.BufferUsage.unordered_access
bp=dev.create_buffer(data=mp,usage=u); bg=dev.create_buffer(data=np.zeros(tm,dtype='float32'),usage=u)
bm=dev.create_buffer(data=np.zeros(tm,dtype='float32'),usage=u); bv=dev.create_buffer(data=np.zeros(tm,dtype='float32'),usage=u)
bl=dev.create_buffer(data=ln.ravel(),usage=u); blg=dev.create_buffer(data=np.zeros(tl,dtype='float32'),usage=u)
blm=dev.create_buffer(data=np.zeros(tl,dtype='float32'),usage=u); blv=dev.create_buffer(data=np.zeros(tl,dtype='float32'),usage=u)
rb=dev.create_buffer(data=ref.ravel(),usage=u); lb=dev.create_buffer(data=np.zeros(w*h*3,dtype='float32'),usage=u)

ng=cd(w*h,WG)
kl.dispatch(thread_count=[ng*WG,1,1],img_resolution=[w,h],params=bp,latent_data=bl,ref_image=rb,output_loss=lb,latent_width=32,latent_height=32)
il=float(np.mean(lb.to_numpy().view(np.float32)))
print(f'  Initial loss: {{il:.4f}}',flush=True)

ngb=cd(4096,WG)
for it in range(200):
    s=int((it*2654435761)&0xFFFFFFFF)
    kg.dispatch(thread_count=[ngb*WG,1,1],seed=s,batch_size=[64,64],img_resolution=[w,h],loss_scale=1.0,params=bp,params_grad=bg,latent_data=bl,latent_grad=blg,ref_image=rb,latent_width=32,latent_height=32)
    ngp=cd(tm,WG); ko.dispatch(thread_count=[ngp*WG,1,1],primal=bp,grad=bg,mean_buf=bm,variance_buf=bv,lr=0.001,iter=it+1,loss_scale=1.0,param_count=tm)
    ngl=cd(tl,WG); ko.dispatch(thread_count=[ngl*WG,1,1],primal=bl,grad=blg,mean_buf=blm,variance_buf=blv,lr=0.001,iter=it+1,loss_scale=1.0,param_count=tl)

kl.dispatch(thread_count=[ng*WG,1,1],img_resolution=[w,h],params=bp,latent_data=bl,ref_image=rb,output_loss=lb,latent_width=32,latent_height=32)
fl=float(np.mean(lb.to_numpy().view(np.float32)))
red=(1-fl/il)*100
ok='OK' if np.isfinite(fl) and fl<il*0.5 else 'FAIL'
print(f'  Final: {{fl:.4f}} ({{red:.0f}}% reduction) [{{ok}}]',flush=True)
dev.close()
"""],
        capture_output=False, cwd=REPO_ROOT,
        timeout=600
    )
    if r.returncode != 0:
        print(f"  Process exited with code {r.returncode}", flush=True)
