#ifndef MATHLIBRARY_GLOBAL_H
#define MATHLIBRARY_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef MATHLIBRARY_LIB
# define MATHLIBRARY_EXPORT Q_DECL_EXPORT
#else
# define MATHLIBRARY_EXPORT Q_DECL_IMPORT
#endif

#endif // MATHLIBRARY_GLOBAL_H
