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

FROM node:24-alpine AS client-builder
WORKDIR /app/Web/Client
COPY Web/Client/package*.json ./
RUN npm ci
COPY Web/Client ./
COPY Web/Server/src /app/Web/Server/src
COPY --from=wasm-builder /src/Web/Client/public/hoverrace.* ./public/
ENV VITE_SERVER_URL=""
ENV VITE_GAME_URL=""
RUN npm run build

FROM node:24-alpine AS server-deps
RUN apk add --no-cache python3 make g++ pkgconfig
WORKDIR /app/Web/Server
COPY Web/Server/package*.json ./
RUN npm ci --omit=dev \
    && sqlite3_binary="$(mktemp)" \
    && cp node_modules/sqlite3/build/Release/node_sqlite3.node "$sqlite3_binary" \
    && rm -rf \
        node_modules/.bin \
        node_modules/node-gyp \
        node_modules/prebuild-install \
        node_modules/sqlite3/build \
        node_modules/sqlite3/deps \
        node_modules/sqlite3/src \
        node_modules/sqlite3/node-addon-api \
    && mkdir -p node_modules/sqlite3/build/Release \
    && cp "$sqlite3_binary" node_modules/sqlite3/build/Release/node_sqlite3.node

FROM node:24-alpine AS runtime
ENV NODE_ENV=production
ENV PORT=3000
ENV CLIENT_URL=http://localhost:3001
ENV DB_DIR=/data
WORKDIR /app

RUN apk add --no-cache nginx su-exec \
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
