#include <llvmmci/cxx/opcode.h>
#include <llvmmci/cxx/linker.h>
#include <llvmmci/mci.h>

#include <ppmp/linguistic.h>

llvmmci::architecture_context* llvmmci::host_architecture_context = nullptr;
llvmmci::assembler* llvmmci::host_assembler = nullptr;
llvmmci::disassembler* llvmmci::host_disassembler = nullptr;

llvmmci::dynamic_linker* llvmmci::global_dynamic_linker = nullptr;

architecture_context* host_architecture_context = nullptr;
assembler* host_assembler = nullptr;
disassembler* host_disassembler = nullptr;
dynamic_linker* global_dynamic_linker = nullptr;

__dynamic_init__(__init,
		{
			// 初始化opcode
			llvmmci::init_llvm();
			llvmmci::host_architecture_context = new llvmmci::architecture_context();
			llvmmci::host_assembler = new llvmmci::assembler(llvmmci::host_architecture_context);
			llvmmci::host_disassembler = new llvmmci::disassembler(llvmmci::host_architecture_context, llvmmci::assembly_syntax::ASM_SYNTAX_ATT);
			// 初始化linker
			llvmmci::global_dynamic_linker = new llvmmci::dynamic_linker();
			// 初始化C API
			host_architecture_context = (architecture_context*)llvmmci::host_architecture_context;
			host_assembler = (assembler*)llvmmci::host_assembler;
			host_disassembler = (disassembler*)llvmmci::host_disassembler;
			global_dynamic_linker = (dynamic_linker*)llvmmci::global_dynamic_linker;
			return 0;
		}
)
