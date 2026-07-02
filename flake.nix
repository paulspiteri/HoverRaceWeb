{
  description = "HoverRaceWeb development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      supportedSystems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;
    in
    {
      packages = forAllSystems (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
          lib = pkgs.lib;

          sdlSrc = pkgs.fetchFromGitHub {
            owner = "libsdl-org";
            repo = "SDL";
            rev = "release-3.2.22";
            sha256 = "sha256-4jGfw2hNZTGuae2DMLz8xJBtfNu5abIN5GlNIKDOUpw=";
          };

          sokolSrc = pkgs.fetchFromGitHub {
            owner = "floooh";
            repo = "sokol";
            rev = "55af0a43deff7e9907ffeb5112b5c6ed1480bfb7";
            sha256 = "sha256-eU+UmhFeI2xLwYTDtuD5KpM/rACAJKCqH9/F0DKa5nM=";
          };

          sokolToolsBinSrc = pkgs.fetchFromGitHub {
            owner = "floooh";
            repo = "sokol-tools-bin";
            rev = "90d2b9267813cbfb66e26c8e3eca5cc866f947e0";
            sha256 = "sha256-x48PGfnRk3GSUUYscCFsR/+yuWBekuymjXugUw5fddc=";
          };

          glmSrc = pkgs.fetchFromGitHub {
            owner = "g-truc";
            repo = "glm";
            rev = "1.0.1";
            sha256 = "sha256-GnGyzNRpzuguc3yYbEFtYLvG+KiCtRAktiN+NvbOICE=";
          };

          imguiSrc = pkgs.fetchFromGitHub {
            owner = "ocornut";
            repo = "imgui";
            rev = "v1.91.9";
            sha256 = "sha256-a9qmk7sN/ZZul5VvDoa11frblYDaZLcvfs/WQLZ2KJY=";
          };

          wasmCmakeFlags = [
            "-DFETCHCONTENT_SOURCE_DIR_SDL=${sdlSrc}"
            "-DFETCHCONTENT_SOURCE_DIR_SOKOL=${sokolSrc}"
            "-DFETCHCONTENT_SOURCE_DIR_SOKOL-TOOLS-BIN=${sokolToolsBinSrc}"
            "-DFETCHCONTENT_SOURCE_DIR_GLM=${glmSrc}"
            "-DFETCHCONTENT_SOURCE_DIR_IMGUI=${imguiSrc}"
          ];

          wasmSrc =
            let
              root = toString ./.;
              includeDirs = [
                "Assets"
                "ColorTools"
                "GameSDL"
                "MainCharacter"
                "Model"
                "ObjFac1"
                "ObjFacTools"
                "Util"
                "VideoServices"
              ];
            in
            lib.cleanSourceWith {
              src = ./.;
              filter = path: type:
                let
                  rel = lib.removePrefix "${root}/" (toString path);
                in
                rel == "CMakeLists.txt"
                || rel == "build-web.sh"
                || lib.any (dir: rel == dir || lib.hasPrefix "${dir}/" rel) includeDirs;
            };

          wasm = pkgs.stdenv.mkDerivation {
            pname = "hoverrace-wasm";
            version = "0.0.0";
            src = wasmSrc;

            nativeBuildInputs = [
              pkgs.cmake
              pkgs.emscripten
              pkgs.git
            ];

            cmakeFlags = wasmCmakeFlags;

            dontConfigure = true;

            buildPhase = ''
              runHook preBuild
              emcmake cmake -B build-web -S . ${lib.escapeShellArgs wasmCmakeFlags}
              cmake --build build-web
              runHook postBuild
            '';

            installPhase = ''
              runHook preInstall
              mkdir -p $out
              cp Web/Client/public/hoverrace.* $out/
              runHook postInstall
            '';
          };

          client = pkgs.buildNpmPackage {
            pname = "hoverrace-client";
            version = "0.0.0";
            src = ./Web/Client;
            npmDepsHash = "sha256-Yn5JYPWhP8ca5YOHJtcB3mgqVXZnRoj2IUvf/hVGSjk=";

            preBuild = ''
              mkdir -p ../Server
              cp -r ${./Web/Server/src} ../Server/src
              cp ${wasm}/hoverrace.* public/
              export VITE_SERVER_URL=""
              export VITE_GAME_URL=""
            '';

            installPhase = ''
              runHook preInstall
              mkdir -p $out
              cp -r dist $out/
              runHook postInstall
            '';
          };

          server = pkgs.buildNpmPackage {
            pname = "hoverrace-server";
            version = "0.0.0";
            src = ./Web/Server;
            npmDepsHash = "sha256-wpnW+j09m++vcHpiSreoTAxkC2h/LkoxS+1iPeL6ar4=";

            nativeBuildInputs = [
              pkgs.python3
              pkgs.gnumake
              pkgs.gcc
              pkgs.pkg-config
            ];

            dontNpmBuild = true;

            installPhase = ''
              runHook preInstall
              mkdir -p $out
              cp -r . $out/
              runHook postInstall
            '';
          };

          nginxConf = pkgs.writeText "hoverrace-nginx.conf" ''
            user root;
            pid /tmp/nginx.pid;
            worker_processes auto;

            events {
                worker_connections 1024;
            }

            http {
                include ${pkgs.nginx}/conf/mime.types;
                default_type application/octet-stream;

                access_log /dev/stdout;
                error_log /dev/stderr warn;
                sendfile on;

                client_body_temp_path /tmp/nginx-client-body;
                proxy_temp_path /tmp/nginx-proxy;
                fastcgi_temp_path /tmp/nginx-fastcgi;
                uwsgi_temp_path /tmp/nginx-uwsgi;
                scgi_temp_path /tmp/nginx-scgi;

                upstream hoverrace_server {
                    server 127.0.0.1:3000;
                }

                server {
                    listen 3001;
                    server_name _;
                    root /app/Web/Client/dist;
                    index index.html;

                    location = /health {
                        proxy_pass http://hoverrace_server;
                        proxy_http_version 1.1;
                        proxy_set_header Host $host;
                        proxy_set_header X-Real-IP $remote_addr;
                        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
                        proxy_set_header X-Forwarded-Proto $scheme;
                    }

                    location /api/ {
                        proxy_pass http://hoverrace_server;
                        proxy_http_version 1.1;
                        proxy_buffering off;
                        proxy_cache off;
                        proxy_set_header Connection "";
                        proxy_set_header Host $host;
                        proxy_set_header X-Real-IP $remote_addr;
                        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
                        proxy_set_header X-Forwarded-Proto $scheme;
                    }

                    location / {
                        try_files $uri $uri/ /index.html;
                    }
                }
            }
          '';

          startScript = pkgs.writeShellScriptBin "hoverrace-start" ''
            set -euo pipefail

            shutdown() {
              if [ -n "''${server_pid:-}" ] && kill -0 "$server_pid" 2>/dev/null; then
                kill "$server_pid"
              fi
            }

            trap shutdown INT TERM

            ${pkgs.nodejs-slim}/bin/node /app/Web/Server/src/server.ts &
            server_pid="$!"

            ${pkgs.nginx}/bin/nginx -c /etc/hoverrace/nginx.conf -g 'daemon off;'
            status="$?"
            shutdown
            exit "$status"
          '';

          imageRoot = pkgs.runCommand "hoverrace-image-root" {} ''
            mkdir -p \
              $out/app/Web/Client \
              $out/app/Web/Server \
              $out/bin \
              $out/data \
              $out/etc \
              $out/etc/hoverrace \
              $out/var/log/nginx \
              $out/tmp

            cp -r ${client}/dist $out/app/Web/Client/dist
            cp -r ${server}/* $out/app/Web/Server/
            cp ${nginxConf} $out/etc/hoverrace/nginx.conf
            cp ${startScript}/bin/hoverrace-start $out/bin/hoverrace-start
            printf '%s\n' \
              'root:x:0:0:root:/root:/bin/sh' \
              'nobody:x:65534:65534:nobody:/var/empty:/bin/false' \
              > $out/etc/passwd
            printf '%s\n' \
              'root:x:0:' \
              'nobody:x:65534:' \
              > $out/etc/group
            chmod 1777 $out/tmp
          '';
        in
        lib.optionalAttrs pkgs.stdenv.isLinux rec {
          dockerImage = pkgs.dockerTools.streamLayeredImage {
            name = "hoverraceweb";
            tag = "nix";
            contents = [
              imageRoot
              pkgs.bash
              pkgs.coreutils
              pkgs.nginx
              pkgs.nodejs-slim
            ];
            config = {
              Cmd = [ "/bin/hoverrace-start" ];
              Env = [
                "NODE_ENV=production"
                "PORT=3000"
                "CLIENT_URL=http://localhost:3001"
                "DB_DIR=/data"
              ];
              ExposedPorts = {
                "3001/tcp" = {};
              };
              Volumes = {
                "/data" = {};
              };
              WorkingDir = "/app";
            };
          };

          default = dockerImage;
        });

      devShells = forAllSystems (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
          linuxLibs = pkgs.lib.optionals pkgs.stdenv.isLinux [
            pkgs.libGL
            pkgs.libx11
            pkgs.libxcursor
            pkgs.libxrandr
            pkgs.libxi
            pkgs.libxext
            pkgs.libxcb
            pkgs.wayland
            pkgs.libxkbcommon
            pkgs.alsa-lib
            pkgs.libpulseaudio
            pkgs.dbus
          ];
        in
        {
          default = pkgs.mkShell {
            packages = [
              # Web (Node server + React client)
              pkgs.nodejs

              # C++ build
              pkgs.cmake
              pkgs.gcc
              pkgs.gnumake
              pkgs.pkg-config

              # Emscripten (web/WASM build)
              pkgs.emscripten
            ] ++ linuxLibs;

            LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath linuxLibs;

            shellHook = ''
              echo "HoverRaceWeb dev shell — Node $(node --version), CMake $(cmake --version | head -1)"
            '';
          };
        });
    };
}
