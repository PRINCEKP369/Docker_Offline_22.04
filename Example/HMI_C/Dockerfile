FROM ubuntu:latest
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt install -y \
    build-essential \
    gcc \
    libx11-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY GIU.c .
RUN gcc GUI.C -o app -lX11

CMD["./app"]
