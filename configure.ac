AC_INIT([libDwmCredence],m4_esyscmd_s([./getvers.sh -v]),[dwmcrobb@me.com],[Mcweather],[http://www.mcplex.net])
dnl AC_CONFIG_HEADER([classes/include/DwmPortability.hh])

AC_PROG_CC
AC_PROG_CXX
AC_CANONICAL_TARGET

dnl  ###  Read some generally useful macros.
builtin(include, ./dwm.m4)  dnl
builtin(include, ./ax_check_compile_flag.m4)  dnl

dnl  Set variables for building shared library
DWM_SET_CXX_SHARED_FLAGS

dnl  Set variables for pthreads
DWM_SET_PTHREADFLAGS

OSLIBS=""
KVMLIB="-lkvm"
LIBTOOL="libtool"

case $host_os in
  freebsd[[89]]*)
    CXXFLAGS="-O2"
    LDFLAGS="-static"
    ;;
  freebsd1[[0123]]*)
    CXXFLAGS="-O2"
    LDFLAGS="-static"
    ;;
  linux*)
    CXXFLAGS="-O2"
    KVMLIB=""
    LDFLAGS="$LDFLAGS"
    DEBARCH=`dpkg --print-architecture`
    AC_SUBST(DEBARCH)
    ;;
  darwin*)
    CC="clang++"
    CXX="clang++"
    CXXFLAGS="${CXXFLAGS} -O2 -stdlib=libc++"
    KVMLIB=""
    LDFLAGS="${LDFLAGS} -O2 -stdlib=libc++ -std=c++17"
    LIBTOOL="glibtool"
    ;;
esac

echo "host_os ${host_os}"

dnl  Check for C++20
DWM_CHECK_CPLUSPLUS_20

DWM_CHECK_NEED_LIBATOMIC

AC_SUBST(KVMLIB)
AC_SUBST(LIBTOOL)
AC_SUBST(OSLIBS)

DWM_REQUIRES_DWMPKG(libDwm,0.9.22)
DWM_REQUIRES_DWMPKG(libDwmCredence,0.1.27)
DWM_REQUIRES_DWMPKG(libDwmWebUtils,0.1.24)

DWMDIR=`pkg-config --variable=prefix libDwm`
DWMINCS=`pkg-config --cflags-only-I libDwmWebUtils libDwmCredence libDwm`
DWMLIBS=`pkg-config --libs libDwmWebUtils libDwmCredence libDwm`
AC_SUBST(DWMINCS)
AC_SUBST(DWMLIBS)
AC_SUBST(DWMDIR)

DWM_CHECK_BOOSTASIO
DWM_CHECK_NEED_LIBIBVERBS

AC_SUBST(LDFLAGS)

DWM_CHECK_STRTOF()

DWM_PREREQ_PATH(htmlman,share/htmlman,[HTML manpage path])

DWM_SET_PKGVARS(Mcweather,[Mcweather-0.0.0])
DWM_GET_TAG([Mcweather])

AC_CHECK_PROG([MANDOC], [mandoc], [mandoc], [])

case $host_os in
  freebsd1*)  
    DWM_CHECK_SQLITE3()
    AC_SUBST(SQLITEDIR)
    ;;
esac

AC_SUBST(CXXFLAGS)

DWM_CHECK_LIBSTDCPPFS

BUILD_DOCS=""
AC_ARG_ENABLE([docs],[AS_HELP_STRING([--enable-docs],[build documentation])],
	      [BUILD_DOCS="yes"], [])
AC_SUBST(BUILD_DOCS)

AC_CONFIG_FILES([Makefile.vars packaging/debcontrol
                 packaging/fbsd_manifest packaging/Mcweather.pc
		 classes/include/DwmMcweatherVersion.hh])
AC_OUTPUT
