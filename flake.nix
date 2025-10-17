{
  description = "ANYKS Language Model";

  inputs = {
    textapp-pkgs.url = "git+ssh://git@tsa04.isa.ru/textapp/textapp-pkgs";
  };

  outputs = { self, textapp-pkgs }:
    let pkgs = import textapp-pkgs.inputs.nixpkgs {
          system = "x86_64-linux";
          overlays = [ textapp-pkgs.overlays.default
                       self.overlays.default ];
        };
    in {
      overlays.default = final: prev: {
        cityhash = final.callPackage ./nix/cityhash {};
        alm = final.callPackage ./nix {src=self;};
      };

      packages.x86_64-linux = {
        inherit (pkgs)
          alm;
        default = pkgs.alm;
      };



      devShells.x86_64-linux.default =
        pkgs.mkShell {

          inputsFrom = [pkgs.alm];
          buildInputs = [
            pkgs.ccls
            pkgs.gdb
          ];

          shellHook = ''  echo "TEXTAPP_V3_ROOT=$TEXTAPP_V3_ROOT";
          if [ ! -z "$TEXTAPP_V3_ROOT" ] ; then 
            export CMAKE_LIBRARY_PATH=$TEXTAPP_V3_ROOT/libtextapp/build/src/lib/:$CMAKE_LIBRARY_PATH;
            export CMAKE_INCLUDE_PATH=$TEXTAPP_V3_ROOT/libtextapp/include/:$CMAKE_INCLUDE_PATH; 
          fi  
          ''; 
        };
    };
}

