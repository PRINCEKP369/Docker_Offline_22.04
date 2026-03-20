#!/bin/bash
# Run this on your ONLINE machine

IMAGES=(
  "ubuntu:22.04"
  "alpine:3.19"
  "debian:bookworm-slim"
  "nginx:stable"
  "httpd:2.4"
  "postgres:15"
  "mysql:8.0"
  "mongo:6"
  "redis:7"
  "python:3.11-slim"
  "node:20-alpine"
  "openjdk:17-slim"
  "golang:1.22-alpine"
  "rabbitmq:3-management"
  "prom/prometheus"
  "grafana/grafana"
  "jenkins/jenkins:lts"
  "registry:2"
)

mkdir -p ~/docker-images
cd ~/docker-images

for img in "${IMAGES[@]}"; do
  echo "Pulling $img..."
  docker pull "$img"
  
  # Replace / with _ for filename
  fname="${img//\//_}"
  fname="${fname//:/_}"
  
  echo "Saving $img → ${fname}.tar"
  docker save "$img" -o "${fname}.tar"
done

echo "Done! Transfer the .tar files to your offline server."
