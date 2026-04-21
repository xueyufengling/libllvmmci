#ifndef _LLVMMCI_STRUCT
#define _LLVMMCI_STRUCT

#include <stdint.h>

extern "C"
{
struct array
{
	size_t size;
	uint8_t data[1]; //数据起始地址，长度为length
};

__declspec(dllexport) extern array* alloc_array(size_t length);

__declspec(dllexport) extern void free_array(array* arr);
}

#endif //_LLVMMCI_STRUCT
