#!/bin/bash
# Run this on your ONLINE machine

mkdir -p ~/dl-images
cd ~/dl-images

echo "=== Pulling PyTorch (CPU) ==="
docker pull pytorch/pytorch:2.2.0-cuda11.8-cudnn8-runtime
docker save pytorch/pytorch:2.2.0-cuda11.8-cudnn8-runtime \
  -o pytorch-2.2.0-cpu.tar

echo "=== Pulling TensorFlow (CPU) ==="
docker pull tensorflow/tensorflow:latest
docker save tensorflow/tensorflow:latest \
  -o tensorflow-latest-cpu.tar

echo "=== Pulling TensorFlow + Jupyter (CPU) ==="
docker pull tensorflow/tensorflow:latest-jupyter
docker save tensorflow/tensorflow:latest-jupyter \
  -o tensorflow-latest-jupyter.tar

echo "=== Pulling Jupyter Notebook (standalone) ==="
docker pull quay.io/jupyter/base-notebook:latest
docker save quay.io/jupyter/base-notebook:latest \
  -o jupyter-base-notebook.tar

echo "=== Pulling Jupyter with Python + Data Science libs ==="
docker pull quay.io/jupyter/datascience-notebook:latest
docker save quay.io/jupyter/datascience-notebook:latest \
  -o jupyter-datascience-notebook.tar

echo ""
echo "=== Done! File sizes: ==="
ls -lh ~/dl-images/
