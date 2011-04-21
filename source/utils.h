/* 
   TINY3D sample / (c) 2010 Hermes  <www.elotrolado.net>

*/

#ifndef UTILS_H
#define UTILS_H

#include <tiny3d.h>
#include <sys/cond.h>
#include <sys/mutex.h>
#include <sys/thread.h>

#define MIN( x, y ) ( ( x ) < ( y ) ? ( x ) : ( y ) )
#define MAX( x, y ) ( ( x ) > ( y ) ? ( x ) : ( y ) )
//#define RU(x,n)	(-(-(x) & -(n)))
#define RU(N, S) ((((N) + (S) - 1) / (S)) * (S))

#define le16(i) ((((u16) ((i) & 0xFF)) << 8) | ((u16) (((i) & 0xFF00) >> 8)))
#define le32(i) ((((u32)le16((i) & 0xFFFF)) << 16) | ((u32)le16(((i) & 0xFFFF0000) >> 16)))
#define le64(i) ((((u64)le32((i) & 0xFFFFFFFFLL)) << 32) | ((u64)le32(((i) & 0xFFFFFFFF00000000LL) >> 32)))


#define PI 				3.14159265f
#define FROM_ANGLE( x ) ( -( PI * x )/180.0f )


#define THREAD_SLEEP 100

#define KiB				1024
#define MiB				1048576
#define GiB				1073741824

#define INC_FILE( x ) \
	extern const u8 x [];\
	extern const u32 x##_size;

template <typename T>
class ForeachContainer {
public:
	inline ForeachContainer(const T& t) : c(t), brk(0), i(c.begin()), e(c.end()) { }
	const T c;
	int brk;
	typename T::const_iterator i, e;
};

//apparently this macro doesnt work well if you try to break from inside it
#define FOREACH(variable, container)                                \
for (ForeachContainer<__typeof__(container)> _container_(container); \
	 !_container_.brk && _container_.i != _container_.e;              \
	 __extension__  ({ ++_container_.brk; ++_container_.i; }))                       \
	for (variable = *_container_.i;; __extension__ ({--_container_.brk; break;}))

void hexdump( const void *d, int len );
void mutex_init( sys_mutex_t *m );
void cond_init( sys_cond_t *c, sys_mutex_t *m );

bool IsInRange( int num, int low, int high );



#endif

