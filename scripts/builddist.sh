# build file to generate the distribution binary tarball
myscripts/cleanup

autoreconf

./configure --prefix=/tmp/flwkey-build --enable-static
make install-strip
tar czf flwkey-$1.bin.tgz -C /tmp/flwkey-build .

make clean

./configure \
  $PKGCFG \
  $CROSSCFG \
  --with-ptw32=/opt/mxe/usr/i686-pc-mingw32 \
  --enable-static \
  PTW32_LIBS="-lpthread -lpcreposix -lpcre -lregex" \
  FLTK_CONFIG=$PREFIX/bin/i686-pc-mingw32-fltk-config \

make
$PREFIX/bin/i686-pc-mingw32-strip src/flwkey.exe
make nsisinst
mv src/*setup*exe .

make clean

# build the distribution tarball
./configure
make distcheck
make clean
