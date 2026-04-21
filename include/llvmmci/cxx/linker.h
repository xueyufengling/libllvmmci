#ifndef _LLVMMCI_LINKER
#define _LLVMMCI_LINKER

#include <llvmmci/struct.h>

namespace llvm
{
namespace orc
{
class ExecutionSession;
class RTDyldObjectLinkingLayer;
class JITDylib;
}
}

namespace llvmmci
{
struct dynamic_linker;

struct dynamic_lib_target
{
	dynamic_linker* linker; //链接该库的动态链接器
	llvm::orc::JITDylib* lib; //该链接器所链接的库

	dynamic_lib_target(dynamic_linker* linker, const char* lib_name);
	~dynamic_lib_target();

	/**
	 * @brief 添加.o文件
	 */
	void add_o(void* o, size_t len);

	inline void add_o(array* o)
	{
		add_o(o->data, o->size);
	}

	/**
	 * @brief 查找符号
	 */
	void* lookup(const char* sym_name);
};

/**
 * @brief 运行时的动态链接、加载器，链接得到的结果是已经加载到内存的结果而非原始的so库。
 * 		  使用LLVM ORC API进行，该API主要用于JIT。但本链接器没有IR优化和汇编层，只有链接层。
 */
struct dynamic_linker
{
	llvm::orc::ExecutionSession* exec_session;
	llvm::orc::RTDyldObjectLinkingLayer* o_link_layer; //.o链接层

	dynamic_linker();
	~dynamic_linker();

	/**
	 * @brief 查找或分配指定名称的JITDylib库
	 */
	llvm::orc::JITDylib* jit_dylib(const char* lib_name) const;

	/**
	 * @brief 从ExecutionSession中移除JITDylib库
	 */
	void rm_jit_dylib(llvm::orc::JITDylib* lib) const;

	dynamic_lib_target* link_target(const char* lib_name);

	void add_o(llvm::orc::JITDylib* lib, void* o, size_t len);

	inline void add_o(llvm::orc::JITDylib* lib, array* o)
	{
		add_o(lib, o->data, o->size);
	}

	/**
	 * @brief 查找符号，lib必须是本linker所创建的，否则无法找到符号
	 */
	void* lookup(llvm::orc::JITDylib* lib, const char* sym_name);
};

extern dynamic_linker* global_dynamic_linker;
}

#endif //_LLVMMCI_LINKER
