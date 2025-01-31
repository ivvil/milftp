{
  description = "Milftp flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    utils,
  }:
    utils.lib.eachDefaultSystem (system: let
      pkgs = import nixpkgs {
        inherit system;
      };

      libmill = pkgs.stdenv.mkDerivation rec {
        pname = "libmill";
        version = "1.18";
        src = pkgs.fetchFromGitHub {
          owner = "sustrik";
          repo = "libmill";
          sha256 = "sha256-iJCVtsf4mES/LLXf0AQoWdh7AU7EzYajNmxzG1MN6Jk=";
          rev = "e8937e624757663f5379018cae3f2b3e916afb6c";
        };

        # Add Autotools dependencies
        nativeBuildInputs = [
          pkgs.autoconf
          pkgs.automake
          pkgs.libtool
        ];

        # Configure, build, test, and install
        configurePhase = ''
          ./autogen.sh
          ./configure --prefix=$out
        '';

        buildPhase = ''
          make
        '';

        checkPhase = ''
          make check
        '';

        installPhase = ''
          make install
        '';
      };
    in {
      packages.default = pkgs.stdenv.mkDerivation {
        name = "milftp";
        src = ./.;

        buildInputs = [libmill];
      };

      devShells.default = pkgs.mkShell {
        packages = with pkgs; [
          libmill
          libcxx
          bear
        ];
      };
    });
}
