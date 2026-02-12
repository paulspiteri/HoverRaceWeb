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
            ] ++ linuxLibs;

            LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath linuxLibs;

            shellHook = ''
              echo "HoverRaceWeb dev shell — Node $(node --version), CMake $(cmake --version | head -1)"
            '';
          };
        });
    };
}
