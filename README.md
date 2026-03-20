# Docker_Offlie_22.04
Here's a comprehensive guide for setting up Docker on Ubuntu 22.04 in an **offline environment**, both with and without GPU support.

### Step 1: Install on offline machine

```bash

git clone <URL>
cd $HOME/Docker_Offline_22.04/Debfiles/

# Install all .deb packages
sudo dpkg -i *.deb

# Fix any dependency issues
sudo apt-get install -f
```

### Step 2: Start and enable Docker

```bash
sudo systemctl enable docker
sudo systemctl start docker
sudo systemctl status docker

# Add your user to the docker group (avoids using sudo)
sudo usermod -aG docker $USER
newgrp docker

# Verify
docker --version
docker run hello-world   # only works if you have the image pre-loaded
```

### Step 2: Pre-load Docker images (offline)

```bash
# On offline machine: load image
docker load -i hello-world.tar
docker run hello-world
```

## Tips for Offline Environments

- Use `apt-cache depends --recurse` to capture **all transitive dependencies** before downloading.
- Always do a `dpkg -i *.deb` followed by `apt-get install -f` to resolve any ordering issues.
- Save your Docker images as `.tar` files and load them with `docker load` on the offline machine.
- Use **Docker Compose** offline by saving the plugin `.deb` along with your other packages.
