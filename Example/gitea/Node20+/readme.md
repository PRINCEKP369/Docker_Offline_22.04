
Here's the complete guide for the standalone binary method:

---

## On Your Online Machine — Download

```bash
# Download the standalone binary
wget https://nodejs.org/dist/v20.19.0/node-v20.19.0-linux-x64.tar.xz
or from https://drive.google.com/file/d/17rC5OdH6kY80zyJEUfzyeNo4Cie_ISIn/view?usp=sharing
```

Transfer it to your offline PC via **USB drive or any method**.

---

## On Your Offline PC — Install

### Step 1 — Extract
```bash
tar -xf node-v20.19.0-linux-x64.tar.xz
```

### Step 2 — Move to /usr/local
```bash
sudo cp -r node-v20.19.0-linux-x64/* /usr/local/
```

### Step 3 — Verify
```bash
node --version   # v20.19.0
npm --version
```

If these return versions ✅ you're done! No PATH setup needed since `/usr/local/bin` is already in PATH by default on Ubuntu.

---

## If `node` is Not Found After Copy

Manually add to PATH:

```bash
# Open bashrc
nano ~/.bashrc

# Add this line at the bottom
export PATH=/usr/local/bin:$PATH

# Reload
source ~/.bashrc

# Verify
node --version
```

---

## If You Want to Keep It in a Custom Folder Instead

```bash
# Extract to a custom location
tar -xf node-v20.19.0-linux-x64.tar.xz -C ~/

# Add to PATH permanently
nano ~/.bashrc
```

Add at the bottom:
```bash
export PATH=~/node-v20.19.0-linux-x64/bin:$PATH
```

```bash
source ~/.bashrc
node --version
```

---

## Make Node Available System-wide (For gitea-runner Service)

Since `gitea-runner` runs as a systemd service, it needs to find node too:

```bash
# Create symlinks in /usr/local/bin
sudo ln -s ~/node-v20.19.0-linux-x64/bin/node /usr/local/bin/node
sudo ln -s ~/node-v20.19.0-linux-x64/bin/npm /usr/local/bin/npm

# Verify system-wide
sudo node --version
```

---

## Summary

```
Online PC                         Offline PC
─────────────────                 ──────────────────────────────
wget nodejs tar.xz   →  USB  →   tar -xf node-v20.19.0.tar.xz
                                  sudo cp -r node-*/* /usr/local/
                                  node --version ✅
```

One file, no internet, no apt, no dependency issues! Want help verifying it works with your gitea-runner on the offline PC?
