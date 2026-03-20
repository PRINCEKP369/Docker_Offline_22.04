cd /path/to/redhat-images/

for f in *.tar; do
  echo "Loading $f..."
  docker load -i "$f"
done

docker images | grep -E "ubi|redhat"
```

---

## Which UBI Should You Use?
```
Need to install many packages?     → ubi9 (full dnf)
Lean production image?             → ubi9-minimal
Maximum security / tiny size?      → ubi9-micro
Running systemd services?          → ubi9-init
Need Python/Node/Java pre-baked?   → ubi9/python-311 etc.
Need RHEL compliance in prod?      → registry.redhat.io (subscription)
