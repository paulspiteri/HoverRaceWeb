# Builds the React/Vite client to a static bundle, with the WASM engine copied
# into public/ so Vite emits it at the dist root. Same-origin URLs:
#   VITE_GAME_URL=""   -> index.html loads /hoverrace.js
#   VITE_SERVER_URL="" -> API calls hit /api/... (Caddy proxies to Node)
{ pkgs, src, wasm }:
let
  inherit (pkgs) lib;

  # The client's src/types.ts imports shared types from ../../Server/src, so the
  # Server sources must be present when `tsc -b` type-checks. These are type-only
  # imports and are erased from the runtime bundle.
  clientSrc = lib.fileset.toSource {
    root = src;
    fileset = lib.fileset.unions [
      (src + "/Web/Client")
      (src + "/Web/Server/src")
    ];
  };
in
pkgs.buildNpmPackage {
  pname = "hoverrace-client";
  version = "0.0.0";
  src = clientSrc;
  sourceRoot = "source/Web/Client";

  npmDepsHash = "sha256-Yn5JYPWhP8ca5YOHJtcB3mgqVXZnRoj2IUvf/hVGSjk=";

  # Drop the WASM artifacts into public/ before `vite build` copies public/* -> dist/.
  preBuild = ''
    cp ${wasm}/hoverrace.js ${wasm}/hoverrace.wasm ${wasm}/hoverrace.data public/
  '';

  VITE_GAME_URL = "";
  VITE_SERVER_URL = "";

  installPhase = ''
    runHook preInstall
    mkdir -p $out
    cp -r dist/. $out/
    runHook postInstall
  '';
}
