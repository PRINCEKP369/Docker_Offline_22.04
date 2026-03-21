#!/bin/bash
# Run this on your ONLINE machine (needs NVIDIA GPU or just download the image)

mkdir -p ~/dl-images-gpu
cd ~/dl-images-gpu

echo "=== Pulling PyTorch with CUDA 12.1 ==="
docker pull pytorch/pytorch:2.2.0-cuda12.1-cudnn8-runtime
docker save pytorch/pytorch:2.2.0-cuda12.1-cudnn8-runtime \
  -o pytorch-2.2.0-cuda12.1.tar

echo "=== Pulling TensorFlow GPU ==="
docker pull tensorflow/tensorflow:latest-gpu
docker save tensorflow/tensorflow:latest-gpu \
  -o tensorflow-latest-gpu.tar

echo "=== Pulling TensorFlow GPU + Jupyter ==="
docker pull tensorflow/tensorflow:latest-gpu-jupyter
docker save tensorflow/tensorflow:latest-gpu-jupyter \
  -o tensorflow-latest-gpu-jupyter.tar

echo "=== Pulling Jupyter PyTorch Notebook (CUDA 12) ==="
docker pull quay.io/jupyter/pytorch-notebook:cuda12-python-3.11.8
docker save quay.io/jupyter/pytorch-notebook:cuda12-python-3.11.8 \
  -o jupyter-pytorch-cuda12.tar

echo "=== Pulling Jupyter TensorFlow Notebook (CUDA) ==="
docker pull quay.io/jupyter/tensorflow-notebook:cuda-latest
docker save quay.io/jupyter/tensorflow-notebook:cuda-latest \
  -o jupyter-tensorflow-cuda.tar

echo ""
echo "=== Done! File sizes: ==="
ls -lh ~/dl-images-gpu/
