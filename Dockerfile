# syntax=docker/dockerfile:1

FROM emscripten/emsdk:4.0.23 AS wasm-builder
WORKDIR /src
COPY build-web.sh CMakeLists.txt ./
COPY Assets ./Assets
COPY ColorTools ./ColorTools
COPY GameSDL ./GameSDL
COPY MainCharacter ./MainCharacter
COPY Model ./Model
COPY ObjFac1 ./ObjFac1
COPY ObjFacTools ./ObjFacTools
COPY Util ./Util
COPY VideoServices ./VideoServices
RUN mkdir -p Web/Client/public
RUN ./build-web.sh

FROM node:24-bookworm-slim AS client-builder
WORKDIR /app/Web/Client
COPY Web/Client/package*.json ./
RUN npm ci
COPY Web/Client ./
COPY Web/Server/src /app/Web/Server/src
COPY --from=wasm-builder /src/Web/Client/public/hoverrace.* ./public/
ENV VITE_SERVER_URL=""
ENV VITE_GAME_URL=""
RUN npm run build

FROM node:24-bookworm-slim AS server-deps
RUN apt-get update \
    && apt-get install -y --no-install-recommends python3 make g++ pkg-config \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app/Web/Server
COPY Web/Server/package*.json ./
RUN npm ci --omit=dev

FROM node:24-bookworm-slim AS runtime
ENV NODE_ENV=production
ENV PORT=3000
ENV CLIENT_URL=http://localhost:3001
ENV DB_DIR=/data
WORKDIR /app

RUN apt-get update \
    && apt-get install -y --no-install-recommends nginx \
    && rm -rf /var/lib/apt/lists/* \
    && mkdir -p /data /run/nginx \
    && chown node:node /data

COPY --from=server-deps --chown=node:node /app/Web/Server/node_modules /app/Web/Server/node_modules
COPY --chown=node:node Web/Server /app/Web/Server
COPY --from=client-builder /app/Web/Client/dist /app/Web/Client/dist
COPY docker/nginx.conf /etc/nginx/nginx.conf
COPY docker/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

VOLUME ["/data"]
EXPOSE 3001

ENTRYPOINT ["/entrypoint.sh"]
