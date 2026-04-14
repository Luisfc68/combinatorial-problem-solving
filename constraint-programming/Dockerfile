FROM debian:trixie-slim@sha256:4ffb3a1511099754cddc70eb1b12e50ffdb67619aa0ab6c13fcd800a78ef7c7a AS builder


ENV GECODE_VERSION=6.2.0-7
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    pkg-config \
    libgecode-dev=$GECODE_VERSION \
    && rm -rf /var/lib/apt/lists/*
    
COPY src/Makefile src/*.cpp .

RUN make all && \
    chmod a+x sdp sdp-checker && \
    cp sdp /usr/bin/sdp && \
    cp sdp-checker /usr/bin/sdp-checker 

WORKDIR /app
COPY src/*.sh .


