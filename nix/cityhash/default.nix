{
  stdenv,
  fetchgit,
}:

stdenv.mkDerivation {
  pname = "cityhash";
  version = "master";
  src = fetchgit {
    url = "https://github.com/google/cityhash";
    sha256=null;
    rev = "25fdf359711eb27e9e7ec0cfe19cc459ec6488d7";
  };

  nativeBuildInputs = [ make ];
}



