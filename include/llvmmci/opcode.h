#ifndef _LLVMMCI_OPCODE
#define _LLVMMCI_OPCODE

#include <llvmmci/llvm_mci.h>

namespace llvm
{
//架构信息
class Triple;
class Target;
class MCRegisterInfo;
class MCAsmInfo;
class MCInstrInfo;
class MCTargetOptions;
class MCSubtargetInfo;

//上下文
class MCContext;

//汇编
class MCAsmBackend;
class MCCodeEmitter;
class MCAsmParser;
class MCTargetAsmParser;
class SourceMgr;
class raw_svector_ostream;
class MCStreamer;

//反汇编

}

namespace llvmmci
{
struct assembly_unit;

enum assembly_syntax : unsigned
{
	ASM_SYNTAX_ATT = 0,
	ASM_SYNTAX_INTEL = 1,
};

struct assembly_context
{
	const llvm::Triple* triple; //架构三元组信息
	const llvm::Target* target_arch; //架构
	const llvm::MCTargetOptions* options;
	const llvm::MCAsmInfo* asm_info;
	const llvm::MCRegisterInfo* reg_info;
	const llvm::MCInstrInfo* ins_info;
	const llvm::MCSubtargetInfo* subtarget_info; //指定CPU和架构特性

	assembly_context(const char* target_arch_triple, const char* target_cpu, const char* feature);

	assembly_context(const char* target_arch_triple) :
			assembly_context(target_arch_triple, "generic", "")
	{
	}

	assembly_context();

	/**
	 * 基于当前的架构和options创建一个新的上下文，不同汇编单元上下文各自独立。
	 * MCContext包含各个汇编单元独有的数据，汇编时不同单元的代码不能复用，但反汇编可以复用。
	 */
	llvm::MCContext* new_context(const llvm::SourceMgr* Mgr = nullptr, bool DoAutoReset = true, const char* Swift5ReflSegmentName = nullptr);

	//汇编

	llvm::MCAsmBackend* new_asm_backend();

	llvm::MCCodeEmitter* new_code_emitter(llvm::MCContext* ctx);

	llvm::MCStreamer* new_object_streamer(llvm::MCContext* ctx, llvm::MCAsmBackend* backend, llvm::raw_svector_ostream* o_stream, llvm::MCCodeEmitter* code_emitter);

	llvm::MCAsmParser* new_asm_parser(llvm::MCContext* ctx, llvm::SourceMgr* src_ctx, llvm::MCStreamer* o_streamer, unsigned CB = 0);

	llvm::MCTargetAsmParser* new_target_asm_parser(llvm::MCAsmParser* parser);

	assembly_unit* new_asm_unit(bool DoAutoReset = true, const char* Swift5ReflSegmentName = nullptr);

	//反汇编
};

extern assembly_context* host_assembly_context;

/**
 * @brief 汇编文件单元，该单元内的指令和状态是互相关联的
 */
struct assembly_unit
{
	assembly_context* as_ctx;
	llvm::SourceMgr* src_ctx; //汇编指令源码缓冲区
	llvm::MCContext* ctx; //不同单元的上下文不能复用

	assembly_unit(assembly_context* as_ctx, bool DoAutoReset = true, const char* Swift5ReflSegmentName = nullptr);

	~assembly_unit();

	void add_src_buffer(const char* src);

	array* assemble(bool PIC = true, bool LargeCodeModel = false, assembly_syntax syntax = assembly_syntax::ASM_SYNTAX_ATT);
};

}

#endif //_LLVMMCI_OPCODE
