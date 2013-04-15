#ifndef _MPILOTE_DEBUG__
#define _MPILOTE_DEBUG__

#ifndef log
#ifdef DEBUG
#define log printf
#define error(format,...) fprintf(stderr, format, ## __VA_ARGS__)
#else
#define log(...)
#define error(format,...) fprintf(stderr, format, ## __VA_ARGS__)
#endif
#endif

#endif
