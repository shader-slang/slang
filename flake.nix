{
  description = "The Slang Shading Language and Compiler";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShell = pkgs.mkShell {
          buildInputs = [
            pkgs.cmake
            pkgs.gersemi
            pkgs.llvm
            pkgs.llvmPackages_17.clang-tools
            pkgs.ninja
            pkgs.nixfmt-rfc-style
            pkgs.prettier
            pkgs.python3
            pkgs.shfmt
            pkgs.xorg.libX11
          ];
        };
      }
    );
}
