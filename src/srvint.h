#ifndef SRVINT_H
#define SRVINT_H

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif

#include <stdint.h>

#include "libsrvint_export.h"

#ifdef __cplusplus
#define SRVINT_BEGIN_DECLS extern "C" {
#define SRVINT_END_DECLS   }
#else
#define SRVINT_BEGIN_DECLS
#define SRVINT_END_DECLS
#endif  // __cplusplus

SRVINT_BEGIN_DECLS
SRVINT_END_DECLS

#endif  // SRVINT_H
