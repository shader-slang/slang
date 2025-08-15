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
            # We must list clangd before the `clang` package to make sure it
            # comes earlier on the `PATH`, and we must get it from the
            # `clang-tools` package so that it is wrapped properly.
            (pkgs.linkFarm "clangd-21" [
              {
                name = "bin/clangd";
                # New enough to support `HeaderInsertion: Never` in `.clangd`.
                path = "${pkgs.llvmPackages_21.clang-tools}/bin/clangd";
              }
            ])
            (pkgs.linkFarm "clang-format-17" [
              {
                name = "bin/clang-format";
                # Match the clang-format version used in CI.
                path = "${pkgs.llvmPackages_17.clang-tools}/bin/clang-format";
              }
            ])

            pkgs.clang
            pkgs.cmake
            pkgs.gersemi
            pkgs.lldb
            pkgs.ninja
            pkgs.nixfmt-rfc-style
            pkgs.prettier
            pkgs.python3
            pkgs.shfmt
            pkgs.vulkan-loader # Ensure this gets built to use in library path.
            pkgs.xorg.libX11
          ];
          LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [
            # In addition to this, running the Vulkan tests on Linux distros
            # other than NixOS may require the use of nixGL:
            # https://github.com/nix-community/nixGL
            pkgs.vulkan-loader
          ];
        };
      }
    );
}
