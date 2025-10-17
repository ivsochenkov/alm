{
  src,
  stdenv,
  cmake,
  pkg-config,
  boost,
  nlohmann_json,
  python311,
  zlib,
  openssl
}:
stdenv.mkDerivation {
  pname = "alm";
  version = "0.0.1";
  inherit src;

  hardeningDisable = [ "all" ];

  nativeBuildInputs = [cmake pkg-config stdenv.cc];
  buildInputs=[boost nlohmann_json python311 zlib openssl];

  enableParallelBuilding=true;

  doCheck = true;
  checkPhase = ''
    runHook preCheck
    ctest -j 1
    runHook postCheck
  '';
}
