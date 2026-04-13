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

llvm::object::ObjectFile* llvmmci::architecture_context::object_file_from(const void* o, size_t len)
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

llvmmci::obj_file::obj_file(architecture_context* as_ctx, const void* o, size_t len) :
		obj(as_ctx->object_file_from(o, len))
{
}

llvmmci::obj_file::~obj_file()
{
	delete obj;
}

std::vector<llvm::object::SectionRef> llvmmci::obj_file::sections()
{
	std::vector<llvm::object::SectionRef> secs;
	for(const llvm::object::SectionRef& sec : obj->sections())
	{
		secs.push_back(sec);
	}
	return secs;
}

void llvmmci::obj_file::parse_sec_addr_symbols(llvm::object::ObjectFile* obj, const char* sec_name, std::unordered_map<uint64_t, obj_symbol>& sec_symbols)
{
	for(const llvm::object::SymbolRef& sym : obj->symbols())
	{
		auto sec = sym.getSection();
		if(!sec || *sec == obj->section_end())
		{
			continue;	//符号没有段或不在有效的段内则直接返回
		}
		auto sym_sec_name = (*sec)->getName();	//获取符号段名
		if(sym_sec_name)
		{
			const char* sym_sec = (*sym_sec_name).data();
			if(!strcmp(sym_sec, sec_name))
			{
				//先获取符号地址，只有有地址的符号才收集
				//该地址实际上是段内偏移量
				auto addr = sym.getAddress();
				if(!addr)
					continue;
				uint64_t sym_addr = *addr;
				llvmmci::obj_symbol& obj_sym = sec_symbols[sym_addr];
				obj_sym.sec = sec_name;
				obj_sym.sec_offset = sym_addr;
				auto type = sym.getType();
				if(type)
					obj_sym.type = (llvmmci::symbol_type)*type;
				auto name = sym.getName();
				if(name)
					obj_sym.name = (*name).data();
				auto value = sym.getValue();
				if(value)
					obj_sym.value = *value;
				auto flags = sym.getFlags();
				if(flags)
					obj_sym.flags = *flags;
			}
		}
	}
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

bool llvmmci::disassembler::disasm_text(llvm::raw_ostream& out, const void* img_text_base, size_t total_size, std::unordered_map<uint64_t, llvmmci::obj_symbol>* sec_symbols, uint64_t load_base_addr)
{
	size_t offset = 0;	//相对于img_text_base的偏移量，即段内偏移量
	/**
	 * offset仅仅是当前img_base中的偏移量，img_base不一定是运行时加载的库的基址，它也能是从文件系统读取so库的内容基地址
	 * 而load_base_addr是运行时加载目标so库的实际基地址。
	 * 如果img_base刚好是运行时加载的库指针，那么img_base与load_base_addr就相等。
	 * 如果img_base是从文件读取的，那么load_base_addr就必须是运行时加载的库指针。
	 * load_base_addr关系到寻址的计算，即一些汇编指令是位置相关的，必须在指定的地址加载才能正确工作。
	 */
	llvm::MCInst inst;	//当前解析的指令
	while(offset < total_size)
	{
		uint64_t inst_len;
		llvm::ArrayRef<uint8_t> inst_buf((uint8_t*)img_text_base + offset, total_size - offset);	//当前未解析的指令缓冲
		//传入getInstruction()的ArrayRef<>必须第一个字节就是待解析的指令，后面的load_base_addr + offset是用于汇编解析而非指令数据读取的
		uint64_t inst_addr = load_base_addr + offset;
		if(dis_asm->getInstruction(inst, inst_len, inst_buf, inst_addr, llvm::nulls()) == llvm::MCDisassembler::Success)	//取未解析的第一条指令
		{
			if(sec_symbols)	//如果有符号信息则先打印符号
			{
				const llvmmci::obj_symbol& sym = sec_symbols->operator[](offset);	//symbols储存的是符号的段内偏移量，因此使用offset索引
				if(sym.name)
				{
					out << sym.name << ":\n";
				}
			}
			inst_printer->printInst(&inst, inst_addr, "", *as_ctx->subtarget_info, out);	//打印指令
			out << '\n';
			offset += inst_len;
		}
		else
		{
			return false;	//存在无效指令则直接返回空指针
		}
	}
	return true;
}

void llvmmci::disassembler::dump_data_sec_hex(llvm::raw_ostream& out, const llvm::object::SectionRef& sec, size_t data_align)
{
	auto contents = sec.getContents();
	if(!contents)
		return;
	llvm::StringRef contents_buf = *contents;
	uint64_t addr = sec.getAddress();
	size_t offset = 0;
	size_t total_size = contents_buf.size();
	while(offset < total_size)
	{
		out << llvm::format_hex(addr + offset, 8) << ":\t";	//.data段的地址宽8字符
		for(size_t idx = 0; idx < data_align; ++idx)
		{
			size_t data_addr = offset + idx;
			if(data_addr < total_size)
				out << llvm::format_hex((uint8_t)contents_buf[data_addr], 2) << " ";	//按照一行data_align个字节输出.data段数据
			else
				return;
		}
		out << "\n";
		offset += data_align;
	}
}

bool llvmmci::disassembler::disasm_text_sec(llvm::raw_ostream& out, const llvm::object::SectionRef& sec, std::unordered_map<uint64_t, llvmmci::obj_symbol>* sec_symbols)
{
	auto contents = sec.getContents();
	if(!contents)
		return true;
	llvm::StringRef contents_buf = *contents;
	//SectionRef.getContents().data()获取到的指针始终是该段的基址
	return disasm_text(out, contents_buf.data(), contents_buf.size(), sec_symbols, sec.getAddress());
}

array* llvmmci::disassembler::disassemble_text(const void* text, size_t len)
{
	llvm::SmallString<__OS_PAGE_SIZE__> buf;
	llvm::raw_svector_ostream asm_out(buf);
	if(!disasm_text(asm_out, text, len))
		return nullptr;
	array* asm_src = llvmmci::array_from_ostream(asm_out, 0, 1);
	asm_src->data[asm_src->length - 1] = '\0';
	return asm_src;
}

array* llvmmci::disassembler::disassemble_o(const void* o, size_t len, size_t data_align)
{
	llvmmci::obj_file obj(as_ctx, o, len);
	if(!obj)
	{
		return nullptr;
	}
	std::unordered_map<uint64_t, llvmmci::obj_symbol> symbols;
	obj.parse_sec_addr_symbols(".text", symbols);	//收集所有有地址的符号，用于在对应地址处打印符号名称
	llvm::SmallString<__OS_PAGE_SIZE__> buf;
	llvm::raw_svector_ostream asm_out(buf);
	for(const llvm::object::SectionRef sec : obj.sections())
	{
		asm_out << *(sec.getName()) << ":\n";	//section名称，以'.'开头
		if(sec.isText())
		{
			if(!disasm_text_sec(asm_out, sec, &symbols))
				return nullptr;
		}
		else if(sec.isData())
		{
			dump_data_sec_hex(asm_out, sec, data_align);
		}
		else
		{
			dump_data_sec_hex(asm_out, sec, data_align);
		}
	}
	array* asm_src = llvmmci::array_from_ostream(asm_out, 0, 1);
	asm_src->data[asm_src->length - 1] = '\0';
	return asm_src;
}
