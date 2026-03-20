## Running MATLAB Compiled App with MATLAB Runtime R2021a in Docker

---

## Step 1: Compile Your MATLAB Code (On Machine with MATLAB R2021a)

### Simple script compilation
```matlab
% In MATLAB R2021a Command Window
mcc -m your_script.m -o your_app
```

### With dependencies / multiple files
```matlab
mcc -m main_script.m \
    -a helper_function.m \
    -a /path/to/data/ \
    -o your_app \
    -d /output/compiled/
```

### What gets generated after compilation
```
compiled/
├── your_app          ← main executable (this goes into Docker)
├── run_your_app.sh   ← shell wrapper script (use this instead of binary directly)
├── your_app.ctf      ← component technology file (needed alongside binary)
└── readme.txt
```

> ⚠️ **Important:** Always use `run_your_app.sh` to launch — it sets up the environment paths automatically.

---

## Step 2: Download MATLAB Runtime R2021a

On an **online machine**, download the exact matching runtime:

```bash
# Download MCR R2021a for Linux
https://drive.google.com/file/d/1RqTw33YtJ8EBZIK-5w2lCJccUltke308/view?usp=sharing

# Or visit:
# https://www.mathworks.com/products/compiler/matlab-runtime.html
# → Select R2021a → Linux 64-bit
```

> ⚠️ **Version must match exactly** — R2021a MATLAB compiled app **only works** with R2021a Runtime. Mismatch = crash.

---

## Step 3: Create the Dockerfile

```dockerfile
FROM ubuntu:20.04

# Avoid interactive prompts during install
ENV DEBIAN_FRONTEND=noninteractive

# Install required dependencies for MCR R2021a
RUN apt-get update && apt-get install -y \
    wget \
    unzip \
    ca-certificates \
    libxt6 \
    libxmu6 \
    libxpm4 \
    libgl1-mesa-glx \
    libglu1-mesa \
    libasound2 \
    libice6 \
    libsm6 \
    libx11-6 \
    libxext6 \
    libxrender1 \
    libxtst6 \
    && rm -rf /var/lib/apt/lists/*

# Copy MCR installer into image
COPY MATLAB_Runtime_R2021a_Update_8_glnxa64.zip /tmp/

# Install MATLAB Runtime silently
RUN unzip /tmp/MATLAB_Runtime_R2021a_Update_8_glnxa64.zip -d /tmp/mcr_install && \
    /tmp/mcr_install/install \
        -mode silent \
        -agreeToLicense yes \
        -destinationFolder /usr/local/matlab_runtime && \
    rm -rf /tmp/mcr_install \
           /tmp/MATLAB_Runtime_R2021a_Update_8_glnxa64.zip

# Set MCR environment variables for R2021a
ENV MCR_ROOT=/usr/local/matlab_runtime/v910
ENV LD_LIBRARY_PATH=\
${MCR_ROOT}/runtime/glnxa64:\
${MCR_ROOT}/bin/glnxa64:\
${MCR_ROOT}/sys/os/glnxa64:\
${MCR_ROOT}/sys/opengl/lib/glnxa64

# Copy your compiled app files
COPY your_app        /app/your_app
COPY run_your_app.sh /app/run_your_app.sh
COPY your_app.ctf    /app/your_app.ctf

RUN chmod +x /app/your_app /app/run_your_app.sh

WORKDIR /app

# Use the shell wrapper to run
CMD ["./run_your_app.sh", "/usr/local/matlab_runtime/v910"]
```

> 💡 **R2021a MCR version folder is `v910`** — this is critical to get right.

---

## Step 4: Project Folder Structure

Organize your files like this before building:

```
project/
├── Dockerfile
├── MATLAB_Runtime_R2021a_Update_8_glnxa64.zip
├── your_app                  ← compiled binary
├── run_your_app.sh           ← generated shell wrapper
└── your_app.ctf              ← compiled archive
```

---

## Step 5: Build the Docker Image

```bash
cd project/

docker build -t matlab-r2021a-app .

# Watch for any errors during MCR silent install
# Build will take 5-10 mins (MCR is ~2GB)
```

---

## Step 6: Run the Container

### Basic run
```bash
docker run --rm matlab-r2021a-app
```

### Pass input arguments to your app
```bash
docker run --rm matlab-r2021a-app \
  ./run_your_app.sh /usr/local/matlab_runtime/v910 arg1 arg2
```

### Mount data folders (input/output)
```bash
docker run --rm \
  -v /host/input:/app/input \
  -v /host/output:/app/output \
  matlab-r2021a-app \
  ./run_your_app.sh /usr/local/matlab_runtime/v910 /app/input /app/output
```

### With GPU support
```bash
docker run --rm --gpus all \
  -v /host/data:/app/data \
  matlab-r2021a-app \
  ./run_your_app.sh /usr/local/matlab_runtime/v910
```

---

## Step 7: Save Image for Offline Use

```bash
# Save
docker save matlab-r2021a-app -o matlab-r2021a-app.tar

# Transfer to offline server, then load
docker load -i matlab-r2021a-app.tar

# Run on offline server
docker run --rm matlab-r2021a-app
```

---

## Troubleshooting

| Error | Cause | Fix |
|---|---|---|
| `version mismatch` | Wrong MCR version | Must use **R2021a** runtime with R2021a compiled app |
| `libmwmclmcrrt.so not found` | `LD_LIBRARY_PATH` wrong | Check `v910` path is correct |
| `cannot execute binary` | Missing `.ctf` file | Copy `your_app.ctf` alongside the binary |
| `Segmentation fault` | Missing system libs | Install `libxt6`, `libxmu6`, `libgl1-mesa-glx` |
| `license error` | MCR needs no license | Make sure you're using compiled app, not raw `.m` file |

---

## MCR Version Number Reference

| MATLAB Version | MCR Folder |
|---|---|
| R2021a | `v910` |
| R2021b | `v911` |
| R2022a | `v912` |
| R2022b | `v913` |
| R2023a | `v914` |

---

Let me know if you face any errors during the build or runtime — paste the error message and I'll help debug it!
