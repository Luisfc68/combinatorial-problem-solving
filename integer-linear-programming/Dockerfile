FROM eclipse-temurin:17-jdk@sha256:b04a8c5d46e210873ffd1af6ad5f4d62c69ed3a6736993556eae60bba1373a23 AS builder

ARG CPLEX_INSTALLER
ENV CPLEX_DIR=/opt/ibm/ILOG/CPLEX_Studio201
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    time \
    && rm -rf /var/lib/apt/lists/*

COPY ${CPLEX_INSTALLER} /tmp/cplex_installer.bin

RUN chmod +x /tmp/cplex_installer.bin && \
    /tmp/cplex_installer.bin -i silent \
        -DLICENSE_ACCEPTED=TRUE \
        -DUSER_INSTALL_DIR=${CPLEX_DIR} && \
    rm /tmp/cplex_installer.bin

COPY src/Makefile src/*.cpp .

RUN make all && \
    chmod a+x sdp sdp-checker && \
    cp sdp /usr/bin/sdp && \
    cp sdp-checker /usr/bin/sdp-checker 

WORKDIR /app
COPY src/*.sh .