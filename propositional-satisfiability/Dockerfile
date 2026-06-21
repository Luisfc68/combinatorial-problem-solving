FROM debian:trixie-slim@sha256:4ffb3a1511099754cddc70eb1b12e50ffdb67619aa0ab6c13fcd800a78ef7c7a AS builder

ENV KISSAT_REPO=https://github.com/arminbiere/kissat
ENV DEBIAN_FRONTEND=noninteractive

RUN useradd -m builder
WORKDIR /app

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    cmake \
    git \
    time \
    pkg-config \
    && update-ca-certificates \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/arminbiere/kissat && \
    chown -R builder:builder ./kissat

USER builder
WORKDIR /app/kissat
RUN ./configure && make test

WORKDIR /app
COPY src/Makefile src/*.cpp .

USER root
RUN make all && \
    chmod a+x sdp sdp-checker && \
    cp ./kissat/build/kissat /usr/bin/kissat && \
    cp sdp /usr/bin/sdp && \
    cp sdp-checker /usr/bin/sdp-checker 

COPY src/*.sh .