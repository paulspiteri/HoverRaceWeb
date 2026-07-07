# Assembles the single OCI image: Caddy fronts the static client + WASM on :80
# and reverse-proxies /api (SSE) to the Node server on 127.0.0.1:3001, which the
# entrypoint starts in the background.
{ pkgs, nodejs, client, server }:
let
  # Generated here so `root` points straight at the client bundle's store path
  # (no /srv symlink needed). Caddy serves application/wasm and SPA-falls-back.
  # `handle` blocks are mutually exclusive and evaluated in source order, so
  # /api/* is proxied and excluded from the SPA fallback. (A bare `try_files`
  # would otherwise rewrite /api/* to /index.html before reverse_proxy runs,
  # because Caddy orders rewrites ahead of reverse_proxy.)
  caddyfile = pkgs.writeText "Caddyfile" ''
    {
    	admin off
    }
    :80 {
    	encode gzip

    	handle /api/* {
    		reverse_proxy 127.0.0.1:3001
    	}

    	handle {
    		root * ${client}
    		try_files {path} /index.html
    		file_server
    	}
    }
  '';

  entrypoint = pkgs.writeShellScriptBin "start-hoverrace" ''
    set -eu
    cd ${server}/app
    ${nodejs}/bin/node ${server}/app/src/server.ts &
    exec ${pkgs.caddy}/bin/caddy run --config ${caddyfile} --adapter caddyfile
  '';
in
pkgs.dockerTools.buildLayeredImage {
  name = "hoverraceweb";
  tag = "latest";

  contents = [
    pkgs.caddy
    nodejs
    pkgs.bashInteractive
    pkgs.coreutils
    pkgs.cacert
    entrypoint
  ];

  # Writable /tmp (Caddy data/config via XDG below) and a /data mount point for
  # the optional SQLite leaderboard DB.
  extraCommands = ''
    mkdir -p tmp && chmod 1777 tmp
    mkdir -p data
  '';

  config = {
    Cmd = [ "${entrypoint}/bin/start-hoverrace" ];
    ExposedPorts = { "80/tcp" = { }; };
    # All overridable at `docker run`. DB_DIR is intentionally unset so the app
    # runs without a volume; set `-e DB_DIR=/data -v hrdata:/data` for leaderboards.
    Env = [
      "PORT=3001"
      "CLIENT_URL=http://localhost:8080"
      "NODE_ENV=production"
      "HOME=/tmp"
      "XDG_DATA_HOME=/tmp"
      "XDG_CONFIG_HOME=/tmp"
      "SSL_CERT_FILE=${pkgs.cacert}/etc/ssl/certs/ca-bundle.crt"
    ];
  };
}
