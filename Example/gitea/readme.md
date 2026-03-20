Here's a complete step-by-step guide to setting up Gitea Actions CI/CD on your Ubuntu 22.04 server.

---

## Overview

The goal is to turn your Gitea from a simple code repository into a full automation pipeline using Gitea Actions, which is compatible with GitHub Actions syntax. You'll need two things: **Gitea Actions enabled** in your config, and an **act_runner** to execute jobs.

---

## Step 1 — Install Docker

The runner needs Docker to spin up isolated job containers.

```bash
cd ../../Debfiles/
sudo dpkg -i *.deb

# Allow your user to run docker without sudo
sudo usermod -aG docker $USER
newgrp docker
```

---

## Step 2 — Enable Gitea Actions

Edit your Gitea config file:

```bash
sudo nano /etc/gitea/app.ini
# or wherever your app.ini lives, commonly:
# /opt/gitea/custom/conf/app.ini
# or if you are installed gitea locally it will be the folder where your executable present.
```

Add this section:

```ini
[actions]
ENABLED = true
```

Then restart Gitea:

```bash
sudo systemctl restart gitea
#or ./gitea web
```

---

## Step 3 — Get a Runner Registration Token

Go to **Site Administration → Actions → Runners → Create new Runner** and copy the generated token.

You can also do this via URL: `http://<your-gitea-ip>:3000/-/admin/actions/runners`

---

## Step 4 — Download and Install act_runner

```bash
# Create a directory for the runner

mkdir gitea_runner
cd gitea_runner
cp act_runner-0.3.0-linux-amd64 act_runner
chmod +x act_runner
```

---

## Step 5 — Register the Runner

```bash
./act_runner register

It will ask you questions one by one:
```
Enter the Gitea instance URL:
> http://localhost:3000

Enter the runner token:
> <paste your token here>

Enter the runner name (leave empty to use hostname):
> my-ubuntu-runner

Enter the runner labels (comma-separated, leave empty for defaults):
> ubuntu-latest,ubuntu-22.04

if no docker type self-hosted:host

Enter the runner workflow working directory (leave empty for default):
> (just press Enter)

```

After registration, a new file named `.runner` will appear in the current directory — it stores registration info. Do not edit it manually.

---

## Step 6 — Run the Runner as a systemd Service

This ensures it starts automatically on reboot.

```bash
sudo nano /etc/systemd/system/gitea-runner.service
```

Paste:

```ini
[Unit]
Description=Gitea Act Runner
After=network.target docker.service
Requires=docker.service

[Service]
User=YOUR_USERNAME
WorkingDirectory=/home/YOUR_USERNAME/gitea-runner
ExecStart=/home/YOUR_USERNAME/gitea-runner/act_runner daemon
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
```

Enable and start:

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now gitea-runner
sudo systemctl status gitea-runner

 or run it simply by ./git-runner daemon

```
---

## Step 7 — Verify in Gitea

Go to **Site Administration → Actions → Runners** — you should see your runner listed as **online** (green dot). ✅

---

## Step 8 — Create Your First Workflow

In any repository, create this file: `.gitea/workflows/ci.yml`

```yaml
name: CI Pipeline
on: [push, pull_request]

jobs:
  build-and-test:
    runs-on: ubuntu-latest  # if no docker use self-hosted:host
    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Run tests
        run: |
          echo "✅ Running tests..."
          # Replace with your actual test commands:
          # npm test
          # pytest
          # go test ./...

      - name: Build
        run: |
          echo "🔨 Building..."
          # your build command here
```

When you push this file, the `on: [push]` trigger fires immediately — you'll see the workflow run appear in the **Actions** tab of your repository.

---

## Bonus: Full Deploy Workflow Example

Here's an end-to-end example — push to `main`, build a Docker image, and deploy automatically:

```yaml
name: Deploy to Production
on:
  push:
    branches: [main]

jobs:
  build-and-deploy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Build Docker image
        run: docker build -t myapp:latest .

      - name: Deploy
        run: |
          docker stop myapp || true
          docker rm myapp || true
          docker run -d --name myapp -p 8080:8080 myapp:latest
```

---

## Summary

| Step | What you did |
|------|-------------|
| 1 | Installed Docker |
| 2 | Enabled Actions in `app.ini` |
| 3 | Got runner token from Gitea admin |
| 4 | Downloaded `act_runner` binary |
| 5 | Registered runner to Gitea |
| 6 | Created a systemd service for auto-start |
| 7 | Verified runner is online |
| 8 | Created a workflow YAML |


Great observation! Let me clarify both questions.

---

## Is Docker Necessary?

**No, Docker is not strictly required.** You have two execution modes for `act_runner`:

| Mode | How it works | Needs Docker? |
|---|---|---|
| **Container mode** (default) | Each job runs in a fresh Docker container | ✅ Yes |
| **Host mode** | Jobs run directly on your Ubuntu machine | ❌ No |

---

## Why We Didn't Explicitly Use Docker in the Workflow

That's the clever part — **the runner uses Docker invisibly behind the scenes.**

When you write this in your workflow:
```yaml
runs-on: ubuntu-latest
```

What actually happens:
```
Your workflow YAML
       ↓
   act_runner reads it
       ↓
   act_runner pulls a Docker image (e.g. catthehacker/ubuntu:act-22.04)
       ↓
   runs your steps INSIDE that container
       ↓
   container is destroyed after job finishes
```

You never write `docker run` yourself — the runner handles it automatically. That's why we installed Docker in Step 1 but never referenced it in the YAML.

---

## Running Without Docker (Host Mode)

If you don't want Docker, register the runner with the `--exec` flag:

```bash
./act_runner register \
  --no-interactive \
  --instance http://localhost:3000 \
  --token <YOUR_TOKEN> \
  --name my-host-runner \
  --labels host-runner \
  --exec          # ← this enables host mode
```

Then in your workflow, reference that label:
```yaml
jobs:
  build:
    runs-on: host-runner   # ← matches the label above
    steps:
      - run: echo "Running directly on Ubuntu, no Docker!"
```

---

## Which Should You Use?

| | Container Mode (Docker) | Host Mode (no Docker) |
|---|---|---|
| **Isolation** | ✅ Each job is sandboxed | ❌ Jobs share your OS |
| **Clean environment** | ✅ Always fresh | ❌ Leftover files can cause issues |
| **Speed** | 🟡 Slightly slower (pull image) | 🟢 Faster |
| **Security** | 🟢 Safer | 🔴 Riskier (jobs access your real system) |
| **Disk space** | 🟡 Uses space for images | 🟢 Minimal |
| **Best for** | Production pipelines | Quick local dev / trusted projects |

---

## Recommendation for Your Setup

if you have **Higher Ram**, Docker container mode is the best choice because:
- Your jobs won't accidentally break your host system
- You can run **multiple jobs in parallel** in isolated containers
- It matches how GitHub Actions and GitLab CI work in the cloud

Think of Docker here not as something you *use*, but as the **sandbox engine** the runner uses under the hood. 

Want me to show you how to configure a custom Docker image for your workflow, or set up host mode instead?


