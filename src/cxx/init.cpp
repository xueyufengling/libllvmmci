#include <as/cxx/opcode.h>

#include <as/as.h>

#include <ppmp/linguistic.h>

as::architecture_context* as::host_architecture_context = nullptr;
as::assembler* as::host_assembler = nullptr;
as::disassembler* as::host_disassembler = nullptr;

architecture_context* host_architecture_context = nullptr;
assembler* host_assembler = nullptr;
disassembler* host_disassembler = nullptr;
dynamic_linker* global_dynamic_linker = nullptr;

static void init_llvm()
{
#if defined(__arch_x86__)
	LLVMInitializeX86TargetInfo();
	LLVMInitializeX86Target();
	LLVMInitializeX86TargetMC();
//汇编
	LLVMInitializeX86AsmParser();
//反汇编
	LLVMInitializeX86Disassembler();
	LLVMInitializeX86AsmPrinter();
#endif
}

// @formatter:off
__dynamic_init__(__init,
{
	// 初始化opcode
	init_llvm();
	as::host_architecture_context = new as::architecture_context();
	as::host_assembler = new as::assembler(as::host_architecture_context);
	as::host_disassembler = new as::disassembler(as::host_architecture_context, as::assembly_syntax::ASM_SYNTAX_ATT);
	// 初始化C API
	host_architecture_context = (architecture_context*)as::host_architecture_context;
	host_assembler = (assembler*)as::host_assembler;
	host_disassembler = (disassembler*)as::host_disassembler;
})
// @formatter:on
