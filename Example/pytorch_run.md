Here's a complete guide to running a PyTorch model inside the Docker container.

---

## Step 1: Start the PyTorch Container

Find the pytorch image at https://drive.google.com/file/d/1oAtdqgdLtNM8LjHFXse7OAy_sSA3cEjs/view?usp=sharing

### CPU
```bash

docker load

docker run -it --rm \
  -v ~/notebooks:/workspace \
  pytorch/pytorch:2.2.0-cuda11.8-cudnn8-runtime bash
```

### GPU
```bash
docker run -it --rm --gpus all \
  --shm-size=8g \
  -v ~/notebooks:/workspace \
  pytorch/pytorch:2.2.0-cuda12.1-cudnn8-runtime bash
```

> `-v ~/notebooks:/workspace` mounts your local folder into the container so your files are saved even after the container stops.

---

## Step 2: Write Your PyTorch Script

On your **host machine** (not inside container), create `~/notebooks/model.py`:

```python
import torch
import torch.nn as nn
import torch.optim as optim

# ── 1. Check device ──────────────────────────────────────────
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
print(f"Using device: {device}")

# ── 2. Create dummy dataset ──────────────────────────────────
# 100 samples, 4 input features, 1 output
X = torch.randn(100, 4).to(device)
y = torch.randn(100, 1).to(device)

# ── 3. Define a simple neural network ────────────────────────
class SimpleModel(nn.Module):
    def __init__(self):
        super(SimpleModel, self).__init__()
        self.network = nn.Sequential(
            nn.Linear(4, 16),    # input layer  → hidden
            nn.ReLU(),
            nn.Linear(16, 8),    # hidden       → hidden
            nn.ReLU(),
            nn.Linear(8, 1)      # hidden       → output
        )

    def forward(self, x):
        return self.network(x)

model = SimpleModel().to(device)
print(f"\nModel architecture:\n{model}")

# ── 4. Define loss function and optimizer ─────────────────────
criterion = nn.MSELoss()
optimizer = optim.Adam(model.parameters(), lr=0.001)

# ── 5. Training loop ──────────────────────────────────────────
print("\nTraining...")
epochs = 20

for epoch in range(epochs):
    model.train()

    # Forward pass
    predictions = model(X)
    loss = criterion(predictions, y)

    # Backward pass
    optimizer.zero_grad()
    loss.backward()
    optimizer.step()

    if (epoch + 1) % 5 == 0:
        print(f"  Epoch [{epoch+1:02d}/{epochs}]  Loss: {loss.item():.4f}")

# ── 6. Save the trained model ─────────────────────────────────
torch.save(model.state_dict(), "/workspace/simple_model.pth")
print("\nModel saved → /workspace/simple_model.pth")

# ── 7. Load and run inference ─────────────────────────────────
loaded_model = SimpleModel().to(device)
loaded_model.load_state_dict(torch.load("/workspace/simple_model.pth"))
loaded_model.eval()

with torch.no_grad():
    test_input = torch.randn(5, 4).to(device)
    output = loaded_model(test_input)

print(f"\nSample inference output (5 predictions):\n{output}")
print("\nDone!")
```

---

## Step 3: Run the Script Inside the Container

Once inside the container shell:

```bash
# You are now inside the container
cd /workspace

# Run the script
python model.py
```

### Expected output:
```
Using device: cuda      ← or "cpu" if no GPU

Model architecture:
SimpleModel(
  (network): Sequential(
    (0): Linear(in_features=4, out_features=16, bias=True)
    (1): ReLU()
    (2): Linear(in_features=16, out_features=8, bias=True)
    (3): ReLU()
    (4): Linear(in_features=8, out_features=1, bias=True)
  )
)

Training...
  Epoch [05/20]  Loss: 0.9821
  Epoch [10/20]  Loss: 0.9134
  Epoch [15/20]  Loss: 0.8762
  Epoch [20/20]  Loss: 0.8401

Model saved → /workspace/simple_model.pth

Sample inference output (5 predictions):
tensor([[ 0.1243],
        [-0.3812],
        [ 0.5631],
        [-0.1029],
        [ 0.2847]])

Done!
```

---

## Step 4: Run Without Entering the Container

You can run the script directly without entering the container shell:

```bash
docker run --rm \
  -v ~/notebooks:/workspace \
  pytorch/pytorch:2.2.0-cuda11.8-cudnn8-runtime \
  python /workspace/model.py
```

---

## Step 5: Run with Jupyter Notebook (Optional)

If you prefer a notebook interface instead of a script:

```bash
# Install Jupyter inside the PyTorch container
docker run -it --rm \
  -p 8888:8888 \
  -v ~/notebooks:/workspace \
  pytorch/pytorch:2.2.0-cuda11.8-cudnn8-runtime \
  bash -c "pip install jupyter && jupyter notebook \
    --ip=0.0.0.0 --port=8888 --no-browser \
    --notebook-dir=/workspace --allow-root"
```

Then open `http://localhost:8888` in your browser and paste the token shown in the terminal.

---

## Common Issues & Fixes

| Error | Cause | Fix |
|---|---|---|
| `CUDA out of memory` | GPU RAM full | Add `torch.cuda.empty_cache()` or reduce batch size |
| `RuntimeError: Expected all tensors on same device` | Model on CPU, data on GPU (or vice versa) | Make sure both have `.to(device)` |
| `bus error / shared memory` | Default shm too small | Add `--shm-size=8g` to docker run |
| `model.pth not found` | Wrong path | Use `/workspace/` prefix for all paths |
| `No module named torch` | Wrong container | Make sure you're using the pytorch image |

---

## Useful PyTorch One-liners Inside the Container

```python
# Check PyTorch version
import torch
print(torch.__version__)

# Check CUDA availability
print(torch.cuda.is_available())
print(torch.cuda.get_device_name(0))    # GPU name

# Check how many GPUs
print(torch.cuda.device_count())

# Check tensor device
tensor = torch.randn(3, 3)
print(tensor.device)                    # cpu or cuda:0
```

Let me know if you want to try a more specific model like a CNN for image classification or an LSTM for sequences!
