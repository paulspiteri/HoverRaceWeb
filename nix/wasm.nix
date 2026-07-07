# Compiles the C++ game engine to WebAssembly with Emscripten, hermetically.
# Produces hoverrace.js / hoverrace.wasm / hoverrace.data (assets preloaded).
{ pkgs, src, deps }:
let
  inherit (pkgs) lib;

  # Only the C++/asset paths matter here — keeping the source narrow means edits
  # to the Web/ app or the other nix files don't invalidate this (slow) build.
  cppSrc = lib.fileset.toSource {
    root = src;
    fileset = lib.fileset.unions [
      (src + "/CMakeLists.txt")
      (src + "/Util")
      (src + "/ColorTools")
      (src + "/Model")
      (src + "/MainCharacter")
      (src + "/ObjFacTools")
      (src + "/ObjFac1")
      (src + "/VideoServices")
      (src + "/GameSDL")
      (src + "/Assets")
    ];
  };
in
pkgs.stdenv.mkDerivation {
  pname = "hoverrace-wasm";
  version = "0.1.0";
  src = cppSrc;

  nativeBuildInputs = [ pkgs.cmake pkgs.emscripten pkgs.python3 ];

  configurePhase = ''
    runHook preConfigure

    # Emscripten needs a writable cache/home; the store copy is read-only and
    # SDL3 is compiled from source, so system libs get built into the cache.
    export HOME=$TMPDIR
    export EM_CACHE=$TMPDIR/emcache
    mkdir -p $EM_CACHE
    if [ -d ${pkgs.emscripten}/share/emscripten/cache ]; then
      cp -r ${pkgs.emscripten}/share/emscripten/cache/. $EM_CACHE/
    fi
    chmod -R u+w $EM_CACHE

    # CMake writes the target into ${"\${CMAKE_SOURCE_DIR}"}/Web/Client/public
    # (see GameSDL/CMakeLists.txt); that path isn't in our narrowed source.
    mkdir -p Web/Client/public

    emcmake cmake -B build-web -S . \
      -DCMAKE_BUILD_TYPE=Release \
      -DFETCHCONTENT_SOURCE_DIR_SDL=${deps.sdl} \
      -DFETCHCONTENT_SOURCE_DIR_SOKOL=${deps.sokol} \
      -DFETCHCONTENT_SOURCE_DIR_GLM=${deps.glm} \
      -DFETCHCONTENT_SOURCE_DIR_IMGUI=${deps.imgui} \
      -DFETCHCONTENT_SOURCE_DIR_SOKOL-TOOLS-BIN=${deps.sokolToolsBin}

    runHook postConfigure
  '';

  buildPhase = ''
    runHook preBuild
    cmake --build build-web
    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall
    mkdir -p $out
    cp Web/Client/public/hoverrace.js \
       Web/Client/public/hoverrace.wasm \
       Web/Client/public/hoverrace.data \
       $out/
    runHook postInstall
  '';

  # Outputs are JS/WASM/data blobs, nothing to patchelf or strip.
  dontFixup = true;
}
