/* These are normally defined in the Amiga's types library, but are
   defined here instead to ensure portability with minimal changes to the
   original Amiga source-code
*/

#ifndef EXEC_TYPES_H
#define EXEC_TYPES_H

typedef unsigned short		uint16;
typedef unsigned char		uint8;
typedef short			int16;
typedef char			int8;
typedef unsigned int		uint32;
typedef int			int32;
typedef double			float64;
typedef unsigned char		TEXT;
typedef short			BOOL;
typedef long			LONG;
typedef unsigned long		ULONG;
#define FALSE 0
#define TRUE 1
#define CONST const

#endif
