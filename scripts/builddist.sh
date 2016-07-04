# build file to generate the distribution binary tarball

autoreconf

./configure \
  $PKGCFG \
  $CROSSCFG \
  --with-ptw32=$PREFIX/i686-w64-mingw32.static \
  --enable-static \
  PTW32_LIBS="-lpthread -lpcreposix -lpcre -lregex" \
  FLTK_CONFIG=$PREFIX/bin/i686-w64-mingw32.static-fltk-config

make clean
make
$PREFIX/bin/i686-w64-mingw32.static-strip src/flwkey.exe
make nsisinst
mv src/*setup*exe .

make clean

# build the distribution tarball
./configure
make distcheck
make clean
