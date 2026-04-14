#include <llvmmci/cxx/linker.h>

#include <llvm/ExecutionEngine/Orc/SelfExecutorProcessControl.h>
#include <llvm/ExecutionEngine/Orc/Core.h>
#include <llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/SectionMemoryManager.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/TargetSelect.h>

#include <llvmmci/cxx/mem.h>
#include <llvmmci/cxx/init_order.h>

llvmmci::dynamic_linker* llvmmci::global_dynamic_linker = nullptr;

__init_module__(linker)
{
	llvmmci::global_dynamic_linker = new llvmmci::dynamic_linker();
}

llvmmci::dynamic_lib_target::dynamic_lib_target(llvmmci::dynamic_linker* linker, const char* lib_name) :
		linker(linker), lib(linker->jit_dylib(lib_name))
{
}

llvmmci::dynamic_lib_target::~dynamic_lib_target()
{
	linker->rm_jit_dylib(lib);
}

llvmmci::dynamic_linker::dynamic_linker()
{
	auto epc = llvm::orc::SelfExecutorProcessControl::Create(); //执行JIT的是当前进程
	if(!epc)
	{
		llvm::errs() << "create llvm::orc::SelfExecutorProcessControl for dynamic linker faield: " << llvm::toString(epc.takeError()) << "\n";
	}
	exec_session = new llvm::orc::ExecutionSession(std::move(*epc));
	// 链接层的创建方法参见 https://github.com/llvm/llvm-project/blob/llvmorg-22.1.3/llvm/lib/ExecutionEngine/Orc/LLJIT.cpp#L952
	// 只创建.o链接层，忽略GetMemoryManagerFunction的ObjBuffer参数，即代表对每个.o都创建一个新的SectionMemoryManager
	o_link_layer = new llvm::orc::RTDyldObjectLinkingLayer(
			*exec_session,
			[](const llvm::MemoryBuffer& ObjBuffer) -> std::unique_ptr<llvm::RuntimeDyld::MemoryManager>
					{
						return std::make_unique<llvm::SectionMemoryManager>();
					});
}

llvmmci::dynamic_linker::~dynamic_linker()
{
	delete o_link_layer;
	delete exec_session;
}

llvm::orc::JITDylib* llvmmci::dynamic_linker::jit_dylib(const char* lib_name) const
{
	llvm::orc::JITDylib* target = exec_session->getJITDylibByName(lib_name); //先获取可能已经存在的同名库
	if(!target)
	{
		auto new_target = exec_session->createJITDylib(lib_name); //如果不存在则创建一个Expected<JITDylib &>
		if(!new_target)
		{
			llvm::errs() << "create llvm::orc::JITDylib for dynamic linker faield: " << llvm::toString(new_target.takeError()) << "\n";
		}
		target = &new_target.get();
	}
	return target;
}

void llvmmci::dynamic_linker::rm_jit_dylib(llvm::orc::JITDylib* lib) const
{
	if(llvm::Error err = exec_session->removeJITDylib(*lib))
	{
		llvm::errs() << "remove lib in dynamic linker faield: " << llvm::toString(std::move(err)) << "\n";
	}
}

llvmmci::dynamic_lib_target* llvmmci::dynamic_linker::link_target(const char* lib_name)
{
	return new llvmmci::dynamic_lib_target(this, lib_name);
}

void llvmmci::dynamic_linker::add_o(llvm::orc::JITDylib* lib, void* o, size_t len)
{
	//MemoryBuffer::getMemBuffer()的RequiresNullTerminator参数必须是false，因为这是字节数据而非字符数据，字节数据不以'\0'结尾
	if(llvm::Error err = o_link_layer->add(*lib, llvmmci::as_membuffer(o, len)))
	{
		llvm::errs() << "add .o in dynamic linker faield: " << llvm::toString(std::move(err)) << "\n";
	}
}

void llvmmci::dynamic_lib_target::add_o(void* o, size_t len)
{
	linker->add_o(lib, o, len);
}

void* llvmmci::dynamic_linker::lookup(llvm::orc::JITDylib* lib, const char* sym_name)
{
	llvm::orc::JITDylibSearchOrder order; //查找顺序
	order.push_back(std::make_pair(lib, llvm::orc::JITDylibLookupFlags::MatchAllSymbols)); //在所有符号中查找
	auto sym = exec_session->lookup(order, exec_session->intern(sym_name)); // Expected<ExecutorSymbolDef>
	if(sym)
	{
		return (void*)(*sym).getAddress().getValue();
	}
	else
	{
		llvm::errs() << "lookup symbol '" << sym_name << "' in dynamic linker faield: " << llvm::toString(sym.takeError()) << "\n";
		return nullptr;
	}
}

void* llvmmci::dynamic_lib_target::lookup(const char* sym_name)
{
	return linker->lookup(lib, sym_name);
}
