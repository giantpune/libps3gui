#ifndef RSXMEM_H
#define RSXMEM_H

#include <vector>
#include <map>
#include <tiny3d.h>

#include <sys/thread.h>

#define RSX_TEXTURE_MEM_SIZE ( 64*1024*1024 )

//since tiny3d doesnt provide a way to allocate and free memory for drawing ( only can allocate )
//here is a class to act as a proxy to that memory and handle allocations
namespace RsxMem
{

	//! init the memory heap.
	// defaults to 64MB
	bool Init( u32 size = RSX_TEXTURE_MEM_SIZE );

	//there is no de-init because pslight doesnt allow to free memory from rsx (yet).
	//and when it does, this class will be useless

	//! allocate memory
	//this function autamatically handles alignment
	// returns a pointer to a memory buffer
	// if off is not NULL, it will copy the offset for rsx drawing into that u32
	u8 *Alloc( u32 bytes, u32* off = NULL );

	//! free memory
	void Free( u8 *p );

	//! get an offset to a buffer to pass when drawing a texture that is stored inside the heap
	u32 Offset( u8 *p );

	//debugging shit
	void PrintInfo( bool verbose = false );

}

#endif // RSXMEM_H
