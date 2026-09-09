
#ifndef QTSPELL_API_H
#define QTSPELL_API_H

#ifdef QTSPELL_STATIC_DEFINE
#  define QTSPELL_API
#  define QTSPELL_NO_EXPORT
#else
#  ifndef QTSPELL_API
#    ifdef qtspell_EXPORTS
        /* We are building this library */
#      define QTSPELL_API __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define QTSPELL_API __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef QTSPELL_NO_EXPORT
#    define QTSPELL_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef QTSPELL_DEPRECATED
#  define QTSPELL_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef QTSPELL_DEPRECATED_EXPORT
#  define QTSPELL_DEPRECATED_EXPORT QTSPELL_API QTSPELL_DEPRECATED
#endif

#ifndef QTSPELL_DEPRECATED_NO_EXPORT
#  define QTSPELL_DEPRECATED_NO_EXPORT QTSPELL_NO_EXPORT QTSPELL_DEPRECATED
#endif

/* NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if) */
#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef QTSPELL_NO_DEPRECATED
#    define QTSPELL_NO_DEPRECATED
#  endif
#endif

#endif /* QTSPELL_API_H */
