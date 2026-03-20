#!/bin/bash
# Run this on your OFFLINE Ubuntu server

cd /path/to/transferred/tars

for f in *.tar; do
  echo "Loading $f ..."
  docker load -i "$f"
done

echo "All images loaded!"
docker images
