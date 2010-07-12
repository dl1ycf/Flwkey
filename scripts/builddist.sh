# build file to generate the distribution binary tarball

make clean
./configure --prefix=/tmp/flwkey-build --enable-static
make install-strip
tar czf flwkey-$1.bin.tgz -C /tmp/flwkey-build/bin flwkey

make clean

./configure $CROSSCFG $PKGCFG FLTK_CONFIG=$PREFIX/bin/fltk-config --with-ptw32=$PREFIX/ptw32 XMLRPC_C_CONFIG=$PREFIX/bin/xmlrpc-c-config
make
i586-mingw32msvc-strip src/flwkey.exe
make nsisinst
mv src/*setup*exe .

make clean

# build the distribution tarball
./configure
make distcheck
make clean
