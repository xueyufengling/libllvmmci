#include <llvmmci/cxx/mem.h>

#include <malloc.h>

#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>

std::unique_ptr<llvm::MemoryBuffer> llvmmci::as_membuffer(const void* buf, size_t len)
{
	return llvm::MemoryBuffer::getMemBuffer(llvm::StringRef((const char*)buf, len), "", false);
}

array* llvmmci::array_from_ostream(llvm::raw_svector_ostream& os, size_t offset, size_t extra_len)
{
	llvm::StringRef buf = os.str();
	size_t buf_size = buf.size();
	array* arr = alloc_array(buf_size + extra_len);
	memcpy(arr->data + offset, buf.data(), buf_size);
	return arr;
}
