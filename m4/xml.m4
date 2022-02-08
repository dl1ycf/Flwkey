AC_DEFUN([AC_FL_XML], [
  AC_ARG_WITH([xml],
              AS_HELP_STRING([--without-xml:],
                             [compile without using XML (Rig control/logbook)]),
		[ac_cv_want_xml="$withval"],
		[ac_cv_want_xml=yes])


AM_CONDITIONAL([ENABLE_XML], [test "x$ac_cv_want_xml" = "xyes"])

])
