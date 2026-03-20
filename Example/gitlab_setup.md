Download the docker_image.zip from https://drive.google.com/file/d/1vSxV-IrYU2vK5ZKYnlATPeEYIEbLP2yz/view?usp=drive_link.

uzip the file and go to the folder DevOps_CI/ and open terminal

## Step 1: Pull & Save for Offline Use

```bash
# On ONLINE machine — pull both GitLab CE and Runner
docker pull gitlab/gitlab-ce:latest
docker pull gitlab/gitlab-runner:latest

# Save as tar for offline transfer
docker save gitlab/gitlab-ce:latest -o gitlab-ce.tar
docker save gitlab/gitlab-runner:latest -o gitlab-runner.tar

# On OFFLINE server — load them
docker load -i gitlab-ce.tar
docker load -i gitlab-runner.tar
```

---

## Step 2: Set Up Directory Structure

```bash
# Create GitLab home directory
sudo mkdir -p /srv/gitlab

# Add to your shell profile so it persists
echo 'export GITLAB_HOME=/srv/gitlab' >> ~/.bashrc
source ~/.bashrc

# GitLab will auto-create these subdirectories:
# /srv/gitlab/config   → gitlab.rb config, SSL certs, root password
# /srv/gitlab/logs     → all GitLab service logs
# /srv/gitlab/data     → repositories, uploads, DB data
```

---

## Step 3: Docker Compose File (Recommended)

Create `/srv/gitlab/docker-compose.yml`:

```yaml
version: '3.8'

services:
  gitlab:
    image: gitlab/gitlab-ce:latest
    container_name: gitlab
    hostname: 'gitlab.yourdomain.com'       # change this
    restart: always
    shm_size: '256m'
    ports:
      - '80:80'
      - '443:443'
      - '2222:22'                            # SSH on 2222 to avoid conflict with host SSH
    volumes:
      - '$GITLAB_HOME/config:/etc/gitlab'
      - '$GITLAB_HOME/logs:/var/log/gitlab'
      - '$GITLAB_HOME/data:/var/opt/gitlab'
    environment:
      GITLAB_OMNIBUS_CONFIG: |
        external_url 'http://gitlab.yourdomain.com'
        gitlab_rails['gitlab_shell_ssh_port'] = 2222
        gitlab_rails['time_zone'] = 'Asia/Kolkata'

        # Reduce memory usage (good for servers with limited RAM)
        puma['worker_processes'] = 2
        puma['min_threads'] = 1
        puma['max_threads'] = 4
        sidekiq['concurrency'] = 5
        prometheus_monitoring['enable'] = false

  gitlab-runner:
    image: gitlab/gitlab-runner:latest
    container_name: gitlab-runner
    restart: always
    depends_on:
      - gitlab
    volumes:
      - '/var/run/docker.sock:/var/run/docker.sock'
      - 'runner-config:/etc/gitlab-runner'

volumes:
  runner-config:
```

---

## Step 4: Start GitLab

```bash
cd /srv/gitlab

# Start in background
docker compose up -d

# Watch startup logs (takes 3–5 mins on first boot)
docker logs -f gitlab
```

> Wait until you see `gitlab Reconfigured!` in the logs before opening the browser.

---

## Step 5: Get Initial Root Password

After the container starts, sign in with username `root` and retrieve the password using this command:

```bash
docker exec -it gitlab grep 'Password:' /etc/gitlab/initial_root_password
```

> ⚠️ This password file is automatically deleted after the first container restart within 24 hours — change it immediately after first login.

---

## Step 6: Access GitLab

```
Web UI  → http://gitlab.yourdomain.com   (or http://localhost if local)
SSH Git → ssh://git@gitlab.yourdomain.com:2222
```

---

## Step 7: Register GitLab Runner

After logging in to GitLab UI:

1. Go to **Admin → CI/CD → Runners → New instance runner**
2. Copy the registration token, then run:

```bash
docker exec -it gitlab-runner gitlab-runner register \
  --url http://gitlab.yourdomain.com \
  --token <YOUR_RUNNER_TOKEN> \
  --executor docker \
  --docker-image alpine:latest \
  --description "docker-runner" \
  --non-interactive
```

---

## Memory Optimization (Important!)

GitLab is resource-intensive — plan for the following minimums:

| Team Size | RAM | CPU |
|---|---|---|
| 1–5 users | 4 GB | 2 cores |
| 5–20 users | 8 GB | 4 cores |
| 20+ users | 16 GB+ | 8 cores |

Add these to `GITLAB_OMNIBUS_CONFIG` to reduce memory on smaller servers:

```ruby
# Disable features you don't need
registry['enable'] = false           # Docker registry
pages_external_url nil
gitlab_pages['enable'] = false       # GitLab Pages
prometheus_monitoring['enable'] = false

# Tune workers
puma['worker_processes'] = 2
sidekiq['concurrency'] = 5
```

---

## Useful Commands

```bash
# Check container status
docker ps | grep gitlab

# Restart GitLab
docker restart gitlab

# Open a shell inside GitLab container
docker exec -it gitlab /bin/bash

# Reconfigure after editing gitlab.rb
docker exec -it gitlab gitlab-ctl reconfigure

# Check all GitLab service status
docker exec -it gitlab gitlab-ctl status

# View specific service logs
docker exec -it gitlab gitlab-ctl tail nginx
docker exec -it gitlab gitlab-ctl tail puma

# Backup GitLab data
docker exec -it gitlab gitlab-backup create

# Upgrade GitLab (pull new image, recreate container)
docker compose pull
docker compose up -d
```

---

## Enable HTTPS (Optional but Recommended)

Update your `GITLAB_OMNIBUS_CONFIG`:

```ruby
# For Let's Encrypt (needs internet access)
external_url 'https://gitlab.yourdomain.com'
letsencrypt['enable'] = true
letsencrypt['contact_emails'] = ['admin@yourdomain.com']

# For self-signed cert (offline environments)
external_url 'https://gitlab.yourdomain.com'
letsencrypt['enable'] = false
nginx['ssl_certificate'] = '/etc/gitlab/ssl/gitlab.crt'
nginx['ssl_certificate_key'] = '/etc/gitlab/ssl/gitlab.key'
```

For self-signed cert, generate and place certs in `$GITLAB_HOME/config/ssl/`.

---

## Volume Reference

| Volume | Inside Container | Contains |
|---|---|---|
| `$GITLAB_HOME/config` | `/etc/gitlab` | `gitlab.rb`, certs, root password |
| `$GITLAB_HOME/logs` | `/var/log/gitlab` | Nginx, Puma, Sidekiq logs |
| `$GITLAB_HOME/data` | `/var/opt/gitlab` | Repos, uploads, PostgreSQL, Redis |

> 💡 **Tip for offline:** Since your server has no internet, make sure `letsencrypt['enable'] = false` and use either a self-signed cert or plain HTTP internally.



