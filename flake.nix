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
        # For debugging with LLDB, Clang seems to behave better than GCC about
        # respecting breakpoint locations and not having missing variables.
        devShell = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
          buildInputs = [
            pkgs.cmake
            pkgs.gersemi
            pkgs.lldb
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
