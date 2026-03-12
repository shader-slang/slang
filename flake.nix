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
        llvmPackages = pkgs.llvmPackages_21;
        # We want to use Clang instead of GCC because it seems to behave better
        # with LLDB, so we use `mkShell` with the LLVM stdenv
        mkShell = pkgs.mkShell.override { stdenv = llvmPackages.stdenv; };
        # We must use the clangd from `clang-tools` package so that it is
        # wrapped properly. This is harder than it seems becase there is a
        # clangd in clang-unwrapped, which would normally come first thanks to
        # the cc-wrapper/setup-hook adding ${clang-unwrapped}/bin to PATH very
        # early in `mkDerivation` setup. We work around this using a shell hook
        # (below) as that executes very late in shell instantiation and can
        # therefore override cc-wrapper.
        #
        # See https://github.com/NixOS/nixpkgs/issues/76486 for the upstream bug.
        clangd-only = (
          pkgs.linkFarm "clangd-only" [
            {
              name = "bin/clangd";
              # New enough to support `HeaderInsertion: Never` in `.clangd`.
              path = "${llvmPackages.clang-tools}/bin/clangd";
            }
          ]
        );
      in
      {
        devShell = mkShell {
          buildInputs = [
            # Pull in only clang-format from clang-tools 17. This matches the
            # version used in CI.
            (pkgs.linkFarm "clang-format-17" [
              {
                name = "bin/clang-format";
                path = "${pkgs.llvmPackages_17.clang-tools}/bin/clang-format";
              }
            ])

            pkgs.cmake
            pkgs.gersemi
            llvmPackages.lldb
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
            # Needed for the prebuilt LLVM
            pkgs.libz
            pkgs.zstd
            # Despite requiring this packages (slang) to be built with Clang,
            # the prebuilt libslang-llvm.so is actually linked against GCC's
            # libstdc++.so.6
            pkgs.stdenv.cc.cc.lib
          ];

          # Use a shell hook to make sure the wrapped clangd is in the path
          # before the unwrapped one included by llvmPackages.stdenv
          shellHook = ''
            PATH="${clangd-only}/bin:$PATH"
          '';
        };
      }
    );
}
