#!/bin/bash
# Run on ONLINE machine — downloads all free UBI images

UBI_IMAGES=(
  # UBI 9 family
  "redhat/ubi9:latest"
  "redhat/ubi9-minimal:latest"
  "redhat/ubi9-micro:latest"
  "redhat/ubi9-init:latest"

  # UBI 8 family
  "redhat/ubi8:latest"
  "redhat/ubi8-minimal:latest"
  "redhat/ubi8-micro:latest"
  "redhat/ubi8-init:latest"

  # UBI 9 language runtimes (from Red Hat Catalog)
  "registry.access.redhat.com/ubi9/python-311:latest"
  "registry.access.redhat.com/ubi9/nodejs-20:latest"
  "registry.access.redhat.com/ubi9/openjdk-21:latest"
  "registry.access.redhat.com/ubi9/go-toolset:latest"
)

mkdir -p ~/redhat-images
cd ~/redhat-images

for img in "${UBI_IMAGES[@]}"; do
  echo "Pulling: $img"
  docker pull "$img"

  fname="${img//\//_}"
  fname="${fname//:/_}"
  fname="${fname//registry.access.redhat.com_/}"

  docker save "$img" -o "${fname}.tar"
  echo "Saved: ${fname}.tar"
done

echo "All done! Transfer .tar files to offline server."
