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
        # We want to use Clang instead of GCC because it seems to behave better
        # with LLDB, so we use `mkShellNoCC` here instead of `mkShell` because
        # the latter brings in GCC by default on Linux.
        devShell = pkgs.mkShellNoCC {
          buildInputs = [
            # Must put `clang-tools` before `clang` for clangd to work properly.
            # We use `llvmPackages_17.clang-tools` instead of just `clang-tools`
            # to match the `clang-format` version used in CI.
            pkgs.llvmPackages_17.clang-tools

            pkgs.clang
            pkgs.cmake
            pkgs.gersemi
            pkgs.lldb
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
