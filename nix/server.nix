# Installs the Node/Express signaling+API server with its node_modules. There is
# no build step — the server runs `node src/server.ts` directly (Node's built-in
# TypeScript type-stripping). sqlite3 is a native addon, so it's compiled from
# source (the node-pre-gyp prebuilt download can't work in the sandbox).
{ pkgs, src, nodejs }:
pkgs.buildNpmPackage {
  pname = "hoverrace-server";
  version = "1.0.0";
  src = src + "/Web/Server";

  inherit nodejs;

  npmDepsHash = "sha256-wpnW+j09m++vcHpiSreoTAxkC2h/LkoxS+1iPeL6ar4=";

  nativeBuildInputs = [ pkgs.python3 pkgs.node-gyp pkgs.pkg-config ];

  # No "build" script in package.json; we only want node_modules assembled.
  dontNpmBuild = true;
  # Force sqlite3 to build from source rather than fetch a prebuilt binary.
  npm_config_build_from_source = "true";
  # Runtime needs only prod deps (express/cors/dotenv/uuid/sqlite3); drop the
  # dev toolchain (vitest/esbuild/rollup/typescript/eslint/...) from the image.
  npmInstallFlags = [ "--omit=dev" ];

  installPhase = ''
    runHook preInstall
    mkdir -p $out/app
    cp -r src package.json node_modules $out/app/
    # sqlite3 depends on node-gyp (build-only). Its Python scripts get their
    # shebangs patched to a concrete python3 store path, dragging python3 into
    # the runtime closure. Runtime only needs the compiled node_sqlite3.node,
    # so drop node-gyp (and the leftover build intermediates).
    find $out/app/node_modules -type d -name node-gyp -prune -exec rm -rf {} +
    # Keep only the compiled addon; the rest of sqlite3/build/ is gyp/autoconf
    # scaffolding (config.gypi, Makefiles, obj/) that records the python3 store
    # path and would drag python3 into the runtime closure.
    sq=$out/app/node_modules/sqlite3/build
    if [ -f $sq/Release/node_sqlite3.node ]; then
      mv $sq/Release/node_sqlite3.node $TMPDIR/node_sqlite3.node
      rm -rf $sq
      mkdir -p $sq/Release
      mv $TMPDIR/node_sqlite3.node $sq/Release/node_sqlite3.node
    fi
    # Drop symlinks left dangling by the prune (e.g. .bin/node-gyp).
    find $out/app/node_modules -xtype l -delete
    runHook postInstall
  '';
}
