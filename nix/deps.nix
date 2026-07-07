# Vendored C++ dependencies that the root CMakeLists.txt normally pulls with
# FetchContent. In the Nix sandbox there is no network at configure time, so we
# prefetch each source (fixed-output derivation) and point CMake at it with
# -DFETCHCONTENT_SOURCE_DIR_<UPPERCASE_NAME>=<store path>.
{ pkgs }:
let
  inherit (pkgs) fetchFromGitHub;

  sources = {
    # -DFETCHCONTENT_SOURCE_DIR_SDL
    sdl = fetchFromGitHub {
      owner = "libsdl-org";
      repo = "SDL";
      rev = "release-3.2.22";
      hash = "sha256-4jGfw2hNZTGuae2DMLz8xJBtfNu5abIN5GlNIKDOUpw=";
    };
    # -DFETCHCONTENT_SOURCE_DIR_SOKOL
    sokol = fetchFromGitHub {
      owner = "floooh";
      repo = "sokol";
      rev = "55af0a43deff7e9907ffeb5112b5c6ed1480bfb7";
      hash = "sha256-eU+UmhFeI2xLwYTDtuD5KpM/rACAJKCqH9/F0DKa5nM=";
    };
    # -DFETCHCONTENT_SOURCE_DIR_GLM
    glm = fetchFromGitHub {
      owner = "g-truc";
      repo = "glm";
      rev = "1.0.1";
      hash = "sha256-GnGyzNRpzuguc3yYbEFtYLvG+KiCtRAktiN+NvbOICE=";
    };
    # -DFETCHCONTENT_SOURCE_DIR_IMGUI
    imgui = fetchFromGitHub {
      owner = "ocornut";
      repo = "imgui";
      rev = "v1.91.9";
      hash = "sha256-a9qmk7sN/ZZul5VvDoa11frblYDaZLcvfs/WQLZ2KJY=";
    };
  };

  # Prebuilt sokol-shdc shader compiler. It is a dynamically-linked ELF that is
  # invoked at build time by VideoServices/CMakeLists.txt to turn the .glsl
  # shaders into .h headers, so it must run inside the Nix sandbox. We patchelf
  # the host binary and hand CMake a copy of the tree with it swapped in via
  # -DFETCHCONTENT_SOURCE_DIR_SOKOL-TOOLS-BIN.
  sokolToolsBinSrc = fetchFromGitHub {
    owner = "floooh";
    repo = "sokol-tools-bin";
    rev = "90d2b9267813cbfb66e26c8e3eca5cc866f947e0";
    hash = "sha256-x48PGfnRk3GSUUYscCFsR/+yuWBekuymjXugUw5fddc=";
  };

  # bin/ subdir CMake selects for the host platform (see VideoServices/CMakeLists.txt).
  shdcPlatform =
    if pkgs.stdenv.hostPlatform.isAarch64 then "linux_arm64" else "linux";

  sokolToolsBin = pkgs.stdenv.mkDerivation {
    pname = "sokol-tools-bin-patched";
    version = "90d2b92";
    src = sokolToolsBinSrc;
    nativeBuildInputs = [ pkgs.autoPatchelfHook ];
    buildInputs = [ pkgs.stdenv.cc.cc.lib pkgs.zlib ];
    # autoPatchelfHook only fixes the ELF we actually use; the other platforms'
    # binaries are left untouched (ignored, not run).
    dontStrip = true;
    installPhase = ''
      runHook preInstall
      mkdir -p $out
      cp -r . $out/
      chmod -R u+w $out
      runHook postInstall
    '';
    # Skip patchelf failures on the non-host prebuilt binaries we never invoke.
    autoPatchelfIgnoreMissingDeps = true;
  };
in
sources // {
  inherit sokolToolsBin shdcPlatform;
}
