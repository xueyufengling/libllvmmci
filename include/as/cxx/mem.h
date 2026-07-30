#ifndef _AS_MEM
#define _AS_MEM

#include <memory>

#include <arch/c_array.h>

namespace llvm
{
class MemoryBuffer;
class raw_svector_ostream;
}

namespace as
{
extern std::unique_ptr<llvm::MemoryBuffer> as_membuffer(const void* buf, size_t len);

/**
 * @brief 将array内存包装成LLVM的MemoryBuffer
 */
inline std::unique_ptr<llvm::MemoryBuffer> as_membuffer(array* arr_ptr)
{
	return as_membuffer(arr_ptr->data, arr_ptr->size);
}

extern array* array_from_ostream(llvm::raw_svector_ostream& os, size_t offset = 0, size_t extra_len = 0);

}

#endif //_AS_MEM
