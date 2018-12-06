with import <nixpkgs> {};

stdenv.mkDerivation {
  name = "gamagora-realtime";
  

  NIX_CFLAGS_COMPILE="-Iglad/include";
  buildInputs = [glfw glm];
}
