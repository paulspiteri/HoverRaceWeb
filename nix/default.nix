# Entry point for the container build. Wires the per-component derivations and
# assembles the final OCI image. Imported from flake.nix for Linux systems.
#
#   nix build .#          -> ./result is the loadable image tarball
#   nix build .#wasm      -> hoverrace.js/.wasm/.data (debug the engine build)
#   nix build .#client    -> static site bundle
#   nix build .#server    -> Node server + node_modules
{ pkgs, src }:
let
  # Node >= 23.6 strips TypeScript types by default, which the server relies on
  # (`node src/server.ts`).
  nodejs = pkgs.nodejs_24;

  deps = import ./deps.nix { inherit pkgs; };
  wasm = import ./wasm.nix { inherit pkgs src deps; };
  client = import ./client.nix { inherit pkgs src wasm; };
  server = import ./server.nix { inherit pkgs src nodejs; };
  image = import ./image.nix { inherit pkgs nodejs client server; };
in
{
  inherit wasm client server image;
  default = image;
}
