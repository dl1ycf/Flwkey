AC_DEFUN([AC_FLWKEY_SH_DQ], [
  ac_sh_dq="\"`$1 | sed 's/"/\\\\"/g'`\""
])

AC_DEFUN([AC_FLWKEY_BUILD_INFO], [
# Define build flags and substitute in Makefile.in
# CPPFLAGS
  FLWKEY_BUILD_CPPFLAGS="-I\$(srcdir) -I\$(srcdir)/include"
  if test "x$ac_cv_flxmlrpc" != "xyes"; then
    FLWKEY_BUILD_CPPFLAGS="$FLWKEY_BUILD_CPPFLAGS -I\$(srcdir)/xmlrpcpp"
  fi
  if test "x$target_win32" = "xyes"; then
      FLWKEY_BUILD_CPPFLAGS="$FLWKEY_BUILD_CPPFLAGS -D_WINDOWS"
  fi
# CXXFLAGS
  FLWKEY_BUILD_CXXFLAGS="$FLTK_CFLAGS -I\$(srcdir) -I\$(srcdir)/include  \
$X_CFLAGS -pipe -Wall -fexceptions $OPT_CFLAGS $DEBUG_CFLAGS $PTW32_CFLAGS"
  if test "x$ac_cv_flxmlrpc" != "xyes"; then
    FLWKEY_BUILD_CXXFLAGS="-I\$(srcdir)/xmlrpcpp $FLWKEY_BUILD_CXXFLAGS"
  fi
  if test "x$target_mingw32" = "xyes"; then
      FLWKEY_BUILD_CXXFLAGS="-mthreads $FLWKEY_BUILD_CXXFLAGS"
  fi
# LDFLAGS
  FLWKEY_BUILD_LDFLAGS=
# LDADD
  FLWKEY_BUILD_LDADD="$FLTK_LIBS $X_LIBS $EXTRA_LIBS $PTW32_LIBS $FLXMLRPC_LIBS"

  if test "x$ac_cv_debug" = "xyes"; then
      FLWKEY_BUILD_CXXFLAGS="$FLWKEY_BUILD_CXXFLAGS -UNDEBUG"
      FLWKEY_BUILD_LDFLAGS="$FLWKEY_BUILD_LDFLAGS $RDYNAMIC"
  else
      FLWKEY_BUILD_CXXFLAGS="$FLWKEY_BUILD_CXXFLAGS -DNDEBUG"
  fi
  if test "x$target_mingw32" = "xyes"; then
      FLWKEY_BUILD_LDFLAGS="-mthreads $FLWKEY_BUILD_LDFLAGS"
  fi

  AC_SUBST([FLWKEY_BUILD_CPPFLAGS])
  AC_SUBST([FLWKEY_BUILD_CXXFLAGS])
  AC_SUBST([FLWKEY_BUILD_LDFLAGS])
  AC_SUBST([FLWKEY_BUILD_LDADD])

#define build variables for config.h
  AC_DEFINE_UNQUOTED([BUILD_BUILD_PLATFORM], ["$build"], [Build platform])
  AC_DEFINE_UNQUOTED([BUILD_HOST_PLATFORM], ["$host"], [Host platform])
  AC_DEFINE_UNQUOTED([BUILD_TARGET_PLATFORM], ["$target"], [Target platform])

  test "x$LC_ALL" != "x" && LC_ALL_saved="$LC_ALL"
  LC_ALL=C
  export LC_ALL

  AC_FLWKEY_SH_DQ([echo $ac_configure_args])
  AC_DEFINE_UNQUOTED([BUILD_CONFIGURE_ARGS], [$ac_sh_dq], [Configure arguments])

# Allow BUILD_DATE, BUILD_USER, BUILD_HOST to be externally overridden by
# environment variables.

  ac_sh_dq="\"$BUILD_DATE\""
  test "x$BUILD_DATE" = "x" && AC_FLWKEY_SH_DQ([date])
  AC_DEFINE_UNQUOTED([BUILD_DATE], [$ac_sh_dq], [Build date])

  ac_sh_dq="\"$BUILD_USER\""
  test "x$BUILD_USER" = "x" && AC_FLWKEY_SH_DQ([whoami])
  AC_DEFINE_UNQUOTED([BUILD_USER], [$ac_sh_dq], [Build user])

  ac_sh_dq="\"$BUILD_HOST\""
  test "x$BUILD_HOST" = "x" && AC_FLWKEY_SH_DQ([hostname])
  AC_DEFINE_UNQUOTED([BUILD_HOST], [$ac_sh_dq], [Build host])

  AC_FLWKEY_SH_DQ([$CXX -v 2>&1 | tail -1])
  AC_DEFINE_UNQUOTED([BUILD_COMPILER], [$ac_sh_dq], [Compiler])

  AC_FLWKEY_SH_DQ([echo $FLWKEY_BUILD_CPPFLAGS $FLWKEY_BUILD_CXXFLAGS])
  AC_DEFINE_UNQUOTED([FLWKEY_BUILD_CXXFLAGS], [$ac_sh_dq], [FLWKEY compiler flags])
  AC_FLWKEY_SH_DQ([echo $FLWKEY_BUILD_LDFLAGS $FLWKEY_BUILD_LDADD])
  AC_DEFINE_UNQUOTED([FLWKEY_BUILD_LDFLAGS], [$ac_sh_dq], [FLWKEY linker flags])

  if test "x$LC_ALL_saved" != "x"; then
      LC_ALL="$LC_ALL_saved"
      export LC_ALL
  fi
])
