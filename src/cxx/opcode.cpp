#include <llvmmci/cxx/opcode.h>

#include <llvm/MC/MCAsmBackend.h>
#include <llvm/MC/MCParser/MCAsmParser.h>
#include <llvm/MC/MCCodeEmitter.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/MC/MCContext.h>
#include <llvm/MC/MCInstrInfo.h>
#include <llvm/MC/MCObjectWriter.h>
#include <llvm/MC/MCParser/MCAsmParser.h>
#include <llvm/MC/MCParser/MCTargetAsmParser.h>
#include <llvm/MC/MCRegisterInfo.h>
#include <llvm/MC/MCStreamer.h>
#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/MC/MCTargetOptions.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Timer.h>
#include <llvm/TargetParser/Host.h>

#include <llvm/Object/ObjectFile.h>
#include <llvm/MC/MCDisassembler/MCDisassembler.h>
#include <llvm/MC/MCInst.h>
#include <llvm/MC/MCInstPrinter.h>

#include <llvmmci/cxx/arch.h>
#include <llvmmci/cxx/mem.h>

llvmmci::architecture_context* llvmmci::host_architecture_context = nullptr;
llvmmci::disassembler* llvmmci::host_att_disassembler = nullptr;

__attribute__((constructor, used)) static void __init_llvm_mc()
{
#if defined(__ARCH_X86__)
	LLVMInitializeX86TargetInfo();
	LLVMInitializeX86Target();
	LLVMInitializeX86TargetMC();
	//汇编
	LLVMInitializeX86AsmParser();
	//反汇编
	LLVMInitializeX86Disassembler();
	LLVMInitializeX86AsmPrinter();
#endif
	llvmmci::host_architecture_context = new llvmmci::architecture_context();
	llvmmci::host_att_disassembler = new llvmmci::disassembler(llvmmci::host_architecture_context, llvmmci::assembly_syntax::ASM_SYNTAX_ATT);
}

//源码缓冲，新建一个汇编单元上下文，不同汇编单元上下文各自独立
llvmmci::assembler::assembler(architecture_context* as_ctx, bool DoAutoReset, const char* Swift5ReflSegmentName) :
		as_ctx(as_ctx), src_ctx(new llvm::SourceMgr()), ctx(as_ctx->new_context(src_ctx, DoAutoReset, Swift5ReflSegmentName))
{
}

llvmmci::assembler::~assembler()
{
	delete src_ctx;
	delete ctx;
}

void llvmmci::assembler::add_src(const char* src)
{
	src_ctx->AddNewSourceBuffer(llvm::MemoryBuffer::getMemBuffer(src), llvm::SMLoc());
}

array* llvmmci::assembler::assemble(bool PIC, bool LargeCodeModel, assembly_syntax syntax)
{
	llvm::MCAsmBackend* backend = as_ctx->new_asm_backend();
	llvm::MCCodeEmitter* code_emitter = as_ctx->new_code_emitter(ctx);
	llvm::MCObjectFileInfo* o_info = new llvm::MCObjectFileInfo();
	o_info->initMCObjectFileInfo(*ctx, PIC, LargeCodeModel); //设置目标.o文件信息
	ctx->setObjectFileInfo(o_info);
	llvm::SmallString<__OS_PAGE_SIZE__> o_buf; //栈上分配机器码缓冲
	llvm::raw_svector_ostream mc_out(o_buf);
	llvm::MCStreamer* o_streamer = as_ctx->new_object_streamer(ctx, backend, &mc_out, code_emitter);
	o_streamer->setUseAssemblerInfoForParsing(true); //显示设置使用backend的设置来解析，实际上默认值也是true
	llvm::MCAsmParser* parser = as_ctx->new_asm_parser(ctx, src_ctx, o_streamer); //asm parser外壳
	llvm::MCTargetAsmParser* target_parser = as_ctx->new_target_asm_parser(parser); //实际对应目标平台的汇编器
	parser->setTargetParser(*target_parser); //设置当前的目标平台汇编器，在Run()之前必须设置
	//设置汇编语法
	parser->setAssemblerDialect(syntax);
	if(parser->Run(false))
	{
		llvm::errs() << "run asm parser failed\n";
		return nullptr;
	}
	array* code_buf = llvmmci::array_from_ostream(mc_out);
	//释放指针。MCAsmBackend*和MCCodeEmitter*都是智能指针，在delete o_streamer时会自动释放，不能再delete
	delete target_parser;
	delete parser;
	delete o_streamer;
	return code_buf;
}

llvmmci::architecture_context::architecture_context(const char* target_arch_triple, const char* target_cpu, const char* feature)
{
	triple = new llvm::Triple(llvm::Triple::normalize(target_arch_triple)); //规范triple字符串，包含CPU架构-供应商-操作系统
	std::string err;
	target_arch = llvm::TargetRegistry::lookupTarget(triple->getTriple(), err);
	if(!target_arch)
	{
		llvm::errs() << "lookup target '" << target_arch_triple << "' faield: " << err << "\n";
	}
	options = new llvm::MCTargetOptions();
	// 目标架构的寄存器信息
	reg_info = target_arch->createMCRegInfo(triple->getTriple());
	// 目标架构的汇编信息
	asm_info = target_arch->createMCAsmInfo(*reg_info, triple->getTriple(), *options);
	inst_info = target_arch->createMCInstrInfo();
	subtarget_info = target_arch->createMCSubtargetInfo(triple->getTriple(), target_cpu, feature);
}

llvmmci::architecture_context::architecture_context() :
		architecture_context(llvm::sys::getDefaultTargetTriple().c_str())
{
}

llvm::MCContext* llvmmci::architecture_context::new_context(const llvm::SourceMgr* Mgr, bool DoAutoReset, const char* Swift5ReflSegmentName)
{
	return new llvm::MCContext(*triple, asm_info, reg_info, subtarget_info, Mgr, options, DoAutoReset, Swift5ReflSegmentName);
}

llvm::MCAsmBackend* llvmmci::architecture_context::new_asm_backend()
{
	return target_arch->createMCAsmBackend(*subtarget_info, *reg_info, *options);
}

llvm::MCCodeEmitter* llvmmci::architecture_context::new_code_emitter(llvm::MCContext* ctx)
{
	return target_arch->createMCCodeEmitter(*inst_info, *ctx);
}

llvm::MCStreamer* llvmmci::architecture_context::new_object_streamer(llvm::MCContext* ctx, llvm::MCAsmBackend* backend, llvm::raw_svector_ostream* o_stream, llvm::MCCodeEmitter* code_emitter)
{
	// 根据triple创建对应类型的.o文件流对象，例如MCELFStreamer，所有平台架构都继承自MCObjectStreamer
	return target_arch->createMCObjectStreamer(*triple, *ctx, std::unique_ptr<llvm::MCAsmBackend>(backend), backend->createObjectWriter(*o_stream), std::unique_ptr<llvm::MCCodeEmitter>(code_emitter), *subtarget_info);
}

llvm::MCAsmParser* llvmmci::architecture_context::new_asm_parser(llvm::MCContext* ctx, llvm::SourceMgr* src_ctx, llvm::MCStreamer* o_streamer, unsigned CB)
{
	/*
	 * https://github.com/llvm/llvm-project/blob/llvmorg-22.1.3/llvm/lib/MC/MCParser/AsmParser.cpp#L6308
	 * 即 return new AsmParser(SM, C, Out, MAI, CB);
	 * https://github.com/llvm/llvm-project/blob/llvmorg-22.1.3/llvm/lib/MC/MCParser/AsmParser.cpp#L744
	 * 该方法返回的MCAsmParser仅仅是一个外壳，在Run()时实际调用的指针正是MCTargetAsmParser* TargetParser
	 */
	return llvm::createMCAsmParser(*src_ctx, *ctx, *o_streamer, *asm_info, CB);
}

llvm::MCTargetAsmParser* llvmmci::architecture_context::new_target_asm_parser(llvm::MCAsmParser* parser)
{
	// https://github.com/llvm/llvm-project/blob/llvmorg-22.1.3/llvm/include/llvm/MC/TargetRegistry.h#L515
	// 注意此函数与llvm::createMCAsmParser()同名，但实际上不是同一个函数
	return target_arch->createMCAsmParser(*subtarget_info, *parser, *inst_info, *options);
}

llvm::object::ObjectFile* llvmmci::architecture_context::object_file_from(void* o, size_t len)
{
	auto obj = llvm::object::ObjectFile::createObjectFile(as_membuffer(o, len)->getMemBufferRef());
	if(!obj)
	{
		llvm::errs() << "create object file from array failed: " << llvm::toString(obj.takeError()) << "\n";
		return nullptr;
	}
	return (*obj).release();	//获取原始指针且防止被unique_ptr<>析构
}

llvm::MCDisassembler* llvmmci::architecture_context::new_disassembler(llvm::MCContext* ctx)
{
	return target_arch->createMCDisassembler(*subtarget_info, *ctx);
}

llvm::MCInstPrinter* llvmmci::architecture_context::new_inst_printer(assembly_syntax dialect)
{
	return target_arch->createMCInstPrinter(*triple, dialect, *asm_info, *inst_info, *reg_info);
}

llvmmci::disassembler::disassembler(architecture_context* as_ctx, assembly_syntax syntax) :
		as_ctx(as_ctx), ctx(as_ctx->new_context(nullptr)), dis_asm(as_ctx->new_disassembler(ctx)), inst_printer(as_ctx->new_inst_printer(syntax))
{
}

llvmmci::disassembler::~disassembler()
{
	delete inst_printer;
	delete dis_asm;
}

array* llvmmci::disassembler::disassemble_text(void* text, size_t len)
{
	llvm::SmallString<__OS_PAGE_SIZE__> buf;
	llvm::raw_svector_ostream asm_out(buf);
	llvm::MCInst inst;	//当前解析的指令
	size_t offset = 0;	//当前的偏移量
	while(offset < len)
	{
		uint64_t inst_len;
		llvm::ArrayRef<uint8_t> inst_buf((uint8_t*)text + offset, len - offset);	//未解析的指令缓冲
		if(dis_asm->getInstruction(inst, inst_len, inst_buf, offset, llvm::nulls()) == llvm::MCDisassembler::Success)	//取未解析的第一条指令
		{
			inst_printer->printInst(&inst, offset, "", *as_ctx->subtarget_info, asm_out);	//打印指令
			asm_out << '\n';
			offset += inst_len;
		}
		else
		{
			return nullptr;	//存在无效指令则直接返回空指针
		}
	}
	array* asm_src = llvmmci::array_from_ostream(asm_out, 0, 1);
	asm_src->data[asm_src->length - 1] = '\0';
	return asm_src;
}
