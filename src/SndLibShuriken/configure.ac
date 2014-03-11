# Configuration script for sndlib (taken from Snd)

AC_INIT(sndlib_shuriken, 22, bil@ccrma.stanford.edu, ftp://ccrma-ftp.stanford.edu/pub/Lisp/sndlib.tar.gz)
SNDLIB_VERSION=22
AC_CONFIG_SRCDIR(io.c)
AC_CANONICAL_HOST
AC_CONFIG_FILES(makefile)
AC_CONFIG_FILES(sndlib.pc)
AC_CONFIG_FILES(sndins/Makefile)
AC_CONFIG_HEADERS(unix-config.h)
AC_CONFIG_FILES(sndlib-config, [chmod +x sndlib-config])

AC_PROG_CC
# AC_HEADER_STDC
AC_PROG_INSTALL

AC_C_BIGENDIAN
AC_CHECK_SIZEOF(void *)
AC_PATH_PROG(PKG_CONFIG, pkg-config, no)


#--------------------------------------------------------------------------------
# configuration options
#   --disable-shared      don't try to make a .so file
#   --with-alsa           use ALSA if possible
#   --with-audio          without-audio to stub out all audio
#   --with-forth          use Forth as extension language
#   --with-gsl            include GSL
#   --with-jack           use Jack
#   --with-oss            use OSS if possible
#   --with-portaudio      use portaudio
#   --with-ruby           use Ruby as the extension language
#   --with-s7             use S7 as the extension language (default = yes)
#--------------------------------------------------------------------------------

AC_ARG_ENABLE(shared,     [  --disable_shared 	  don't build or install the shared library])
AC_ARG_WITH(alsa,	[  --with-alsa		  use ALSA])
AC_ARG_WITH(audio,       [  --without-audio       don't include any audio functionality])
AC_ARG_WITH(forth,       [  --with-forth	  	  use Forth as the extension language])
AC_ARG_WITH(gsl,         [  --with-gsl		  use GSL, default=yes])
AC_ARG_WITH(jack,       [  --with-jack		  use JACK])
AC_ARG_WITH(oss,         [  --with-oss		  use OSS])
AC_ARG_WITH(portaudio,   [  --with-portaudio 	  use portaudio, default=no])
AC_ARG_WITH(ruby,        [  --with-ruby             use Ruby as the extension language])
AC_ARG_WITH(s7,          [  --with-s7  	          use s7, default=yes])


AUDIO_SYSTEM="None"
LIBS=""


#--------------------------------------------------------------------------------
# GSL
#--------------------------------------------------------------------------------

GSL_LIBS=""
GSL_CFLAGS=""
if test "$with_gsl" != no; then
  AC_MSG_CHECKING(for gsl)
  if test x$PKG_CONFIG != xno ; then
    if $PKG_CONFIG gsl --exists ; then
      GSL_LIBS="`$PKG_CONFIG gsl --libs`"
      GSL_CFLAGS="`$PKG_CONFIG gsl --cflags`"
      AC_DEFINE(HAVE_GSL)
      OPTIONAL_LIBRARIES="$OPTIONAL_LIBRARIES gsl"
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi
  fi
fi
AC_SUBST(GSL_LIBS)
AC_SUBST(GSL_CFLAGS)


#--------------------------------------------------------------------------------
# language
#--------------------------------------------------------------------------------

# language choice: ruby if --with-ruby and we can find one of ruby-2.0.pc, ruby-1.9.pc, or ruby-1.8.pc
#                  forth if --with-forth
#                  none if --without-extension-language
#                  s7 otherwise

XEN_LIBS=""
XEN_CFLAGS=""
ac_snd_extension_language=none
SNDLIB_LANGUAGE="None"


#--------------------------------------------------------------------------------
# Ruby
#--------------------------------------------------------------------------------

if test "$with_ruby" = yes ; then
  if test x$PKG_CONFIG != xno ; then
    m4_foreach([ruby_version], [[ruby-2.1], [ruby-2.0], [ruby], [ruby-1.9.3], [ruby-1.9], [ruby-1.8]],
      [
      if test "$ac_snd_extension_language" = none ; then
        if $PKG_CONFIG ruby_version --exists ; then
          AC_DEFINE(HAVE_RUBY)
          XEN_CFLAGS="`$PKG_CONFIG ruby_version --cflags`"
          XEN_LIBS="`$PKG_CONFIG ruby_version --libs`"
          LOCAL_LANGUAGE="Ruby"
          ac_snd_extension_language=Ruby
        fi
      fi
      ])
  fi
fi


#--------------------------------------------------------------------------------
# Forth
#--------------------------------------------------------------------------------

if test "$with_forth" = yes ; then
  AC_PATH_PROG([FTH], [fth], [no])
  AC_MSG_CHECKING([for Forth])
  if test "${FTH}" != no ; then
    XEN_CFLAGS=`${FTH} --no-init-file --eval .cflags`
    XEN_LIBS=`${FTH} --no-init-file --eval .libs`
    AC_MSG_RESULT([yes])
    AC_DEFINE(HAVE_FORTH)
    ac_snd_extension_language=Forth
    SNDLIB_LANGUAGE="Forth"
  else
    AC_MSG_RESULT([no])
  fi
fi


#--------------------------------------------------------------------------------
# s7 (the default)
#--------------------------------------------------------------------------------

if test "$with_s7" != no && test "$ac_snd_extension_language" = none ; then
  AC_DEFINE(HAVE_SCHEME)	
  ac_snd_extension_language=s7
  SNDLIB_LANGUAGE="s7"
  S7_LIB="s7.o"
  AC_SUBST(S7_LIB)
fi

AC_SUBST(XEN_LIBS)
AC_SUBST(XEN_CFLAGS)



#--------------------------------------------------------------------------------
# Audio library
#--------------------------------------------------------------------------------

AUDIO_LIB=""
JACK_LIBS=""
JACK_FLAGS=""

if test "$with_audio" != no ; then

  if test "$with_pulseaudio" = yes ; then
    AC_DEFINE(MUS_PULSEAUDIO)
    AUDIO_LIB="-lpulse-simple"
    AUDIO_SYSTEM=pulseaudio
  fi
  
  if test "$with_portaudio" = yes ; then
    AC_DEFINE(MUS_PORTAUDIO)
    AUDIO_SYSTEM=portaudio
    AUDIO_LIB="-lportaudio"
  fi

  if test "$with_jack" = yes ; then 
    AUDIO_SYSTEM=JACK
    AC_DEFINE(MUS_JACK)
    if test x$PKG_CONFIG != xno ; then
      if $PKG_CONFIG jack --exists ; then    
        JACK_LIBS="`$PKG_CONFIG jack --libs`"
        JACK_FLAGS="`$PKG_CONFIG jack --cflags`"
        if $PKG_CONFIG samplerate --exists ; then    
  	  JACK_LIBS="$JACK_LIBS `$PKG_CONFIG samplerate --libs`"
  	  JACK_FLAGS="$JACK_FLAGS `$PKG_CONFIG samplerate --cflags`"
        else
          JACK_LIBS="$JACK_LIBS -lsamplerate"
        fi
      else
        JACK_LIBS="-ljack -lsamplerate"
      fi
    else
      JACK_LIBS="-ljack -lsamplerate"
    fi
  fi

  if test "$with_alsa" = yes ; then
    AC_DEFINE(HAVE_ALSA)
    AUDIO_LIB="-lasound"
    AUDIO_SYSTEM=ALSA
  fi

  if test "$with_oss" = yes ; then
    AC_DEFINE(HAVE_OSS)
    AUDIO_SYSTEM=OSS
  fi

  if test "$AUDIO_SYSTEM" = None ; then
    case "$host" in
      *-*-linux*) 
        AUDIO_SYSTEM=ALSA
        AC_DEFINE(HAVE_ALSA)
        AUDIO_LIB="-lasound"
	;;
      *-*-sunos4*) 
	AUDIO_SYSTEM=Sun
        ;;
      *-*-solaris*) 
	AUDIO_SYSTEM=Sun
        ;;
      *-*-hpux*) 
	AUDIO_SYSTEM=Hpux
        ;;
      *-*-bsdi*) 
	AC_DEFINE(HAVE_OSS)
	AUDIO_SYSTEM=OSS
        ;;
      *-*-freebsd*)
	AC_DEFINE(HAVE_OSS)
	AUDIO_SYSTEM=OSS
	;;
      *-*-openbsd*) 
	AUDIO_SYSTEM=OpenBSD
        ;;
      *-*-netbsd*) 
	AUDIO_SYSTEM=NetBSD
        ;;
      *-*-cygwin*)
	if test "$with_jack" != yes ; then 
	  AUDIO_SYSTEM=Windows
	fi
	;;
      *-*-mingw*)
	audio_system=Windows
	;;
      *-apple-*)
	if test "$with_jack" != yes ; then 
	  AUDIO_SYSTEM=MacOSX
	  AUDIO_LIB="-framework CoreAudio -framework CoreFoundation -framework CoreMIDI"
        else
          AUDIO_SYSTEM=JACK
	  JACK_LIBS="-framework CoreAudio -framework CoreServices -framework AudioUnit -L/usr/local/lib -ljack -lsamplerate"
          JACK_FLAGS="-I/usr/local/include"
	fi
	;;
    esac
  fi
fi

AC_MSG_CHECKING([for audio system])
AC_MSG_RESULT($AUDIO_SYSTEM)

if test "$AUDIO_SYSTEM" != None ; then
  AC_DEFINE(WITH_AUDIO)
fi


AC_SUBST(AUDIO_LIB)
AC_SUBST(JACK_LIBS)
AC_SUBST(JACK_FLAGS)


#--------------------------------------------------------------------------------
# compiler/loader flags
#--------------------------------------------------------------------------------

LDSO_FLAGS=""
SO_FLAGS=""
SO_LD="ld"
SO_INSTALL="install"
A_INSTALL="install"
# A_LD="ld"
# A_LD_FLAGS=""
A_LD="ar"
A_LD_FLAGS="-rc"
SO_NAME="libsndlib_shuriken.so"
RANLIB=":"
LD_FLAGS="-r"

case "$host" in
  *-*-linux*) 
    LDSO_FLAGS="-shared"
    LIBS="$LIBS -lm -ldl"
    if test "$GCC" = yes ; then
      SO_FLAGS="-fPIC $SO_FLAGS"
      SO_LD="$CC"
    fi
    if test "$with_jack" = yes ; then 
      A_INSTALL=":"
      A_LD=":"
    fi
    ;;

  *-*-sunos4*) 
    LIBS="$LIBS -lm"
    ;;

  *-*-solaris*) 
    LIBS="$LIBS -lm"
    LDSO_FLAGS="-G"
    ;;

  *-*-hpux*) 
    LDSO_FLAGS="+z -Ae +DA1.1"
    if test "$GCC" = yes ; then
      SO_FLAGS="-fPIC $SO_FLAGS"
    fi
    ;;

  *-*-bsdi*) 
    LIBS="$LIBS -lm"
    if test "$GCC" = yes ; then
      SO_FLAGS="-fPIC $SO_FLAGS"
    fi
    ;;

  *-*-freebsd*)
    LIBS="$LIBS -lm"
    if test "$GCC" = yes ; then
      SO_LD="$CC"
      SO_FLAGS="-fPIC $SO_FLAGS"
      LDSO_FLAGS="-shared"
    fi
    ;;

  *-*-openbsd*) 
    LIBS="$LIBS -lm"
    if test "$GCC" = yes ; then
      SO_LD="$CC"
      SO_FLAGS="-fPIC $SO_FLAGS"
      CFLAGS="-ftrampolines $CFLAGS"
      LDSO_FLAGS="-shared"
    fi
    ;;

  *-*-netbsd*) 
    LIBS="$LIBS -lm"
    if test "$GCC" = yes ; then
      SO_LD="$CC"
      SO_FLAGS="-fPIC $SO_FLAGS"
      LDSO_FLAGS="-shared"
    fi
    ;;

  *-*-cygwin*)
    A_LD_FLAGS="-rc"
    SO_INSTALL=":"
    SO_LD=":"
    ;;

   *-*-mingw*)
    LIBS="$LIBS -lwinmm -lwsock32"
    LDFLAGS="$LDFLAGS -mwindows"
    SO_INSTALL=":"
    SO_LD=":"
    ;;

  *-apple-*)
    SO_LD="$CC"	
    LDSO_FLAGS="-dynamic -bundle -undefined suppress -flat_namespace"
    ;;
esac

if test "$enable_shared" = no; then
  SO_LD=":"
  SO_INSTALL=":"
fi

AUDIO_CHOICE="$AUDIO_SYSTEM"
CFLAGS="-I. $CFLAGS"

AC_SUBST(LDSO_FLAGS)
AC_SUBST(SO_FLAGS)
AC_SUBST(SO_INSTALL)
AC_SUBST(A_INSTALL)
AC_SUBST(SO_LD)
AC_SUBST(A_LD)
AC_SUBST(A_LD_FLAGS)
AC_SUBST(LD_FLAGS)
AC_SUBST(SNDLIB_VERSION)
AC_SUBST(SNDLIB_LANGUAGE)
AC_SUBST(SO_NAME)
AC_SUBST(JACK_LIBS)
AC_SUBST(JACK_FLAGS)
AC_SUBST(RANLIB)
AC_SUBST(AUDIO_CHOICE)

AC_OUTPUT

