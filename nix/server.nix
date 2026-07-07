# Installs the Node/Express signaling+API server with its node_modules. There is
# no build step — the server runs `node src/server.ts` directly (Node's built-in
# TypeScript type-stripping). Persistence uses Node's built-in `node:sqlite`, so
# there are no native addons to compile and node_modules is pure JS.
{ pkgs, src, nodejs }:
pkgs.buildNpmPackage {
  pname = "hoverrace-server";
  version = "1.0.0";
  src = src + "/Web/Server";

  inherit nodejs;

  npmDepsHash = "sha256-lWp5Q5zQODWgLPawoCFXp4X+gnJ3cPU9yRwtDKbDaO8=";

  # No "build" script in package.json; we only want node_modules assembled.
  dontNpmBuild = true;
  # Runtime needs only prod deps (express/cors/dotenv/uuid); drop the dev
  # toolchain (vitest/esbuild/rollup/typescript/eslint/...) from the image.
  npmInstallFlags = [ "--omit=dev" ];

  installPhase = ''
    runHook preInstall
    mkdir -p $out/app
    cp -r src package.json node_modules $out/app/
    runHook postInstall
  '';
}
