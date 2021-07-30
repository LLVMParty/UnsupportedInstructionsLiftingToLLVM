#include <main.h>

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/Module.h>

#include <set>

// https://godbolt.org/z/jbP3cbTxc
// https://stackoverflow.com/questions/56432259/how-can-i-indicate-that-the-memory-pointed-to-by-an-inline-asm-argument-may-be

UILifter::UILifter(llvm::Module &Module, bool Is64, bool Debug) : mModule(Module), mContext(Module.getContext()), mIs64(Is64), mDebug(Debug) {
  // Initalise Zydis
  mMode = mIs64 ? ZYDIS_MACHINE_MODE_LONG_64 : ZYDIS_MACHINE_MODE_LONG_COMPAT_32;
  mWidth = mIs64 ? ZYDIS_ADDRESS_WIDTH_64 : ZYDIS_ADDRESS_WIDTH_32;
  if (!ZYAN_SUCCESS(ZydisDecoderInit(&mDecoder, mMode, mWidth)))
    llvm::report_fatal_error(std::string() + __func__ + ": failed to initialise the Zydis decoder!");
  // Generate the assembly register types
  std::vector<llvm::Type *> WType{ llvm::IntegerType::get(mContext, (mIs64 ? 64 : 32)) };
  mRegWordTy = llvm::StructType::create(mContext, WType, "RegisterW");
  std::vector<llvm::Type *> BType;
  for (size_t i = 0; i < (mIs64 ? 8 : 4); i++)
    BType.push_back(llvm::IntegerType::get(mContext, 8));
  mRegByteTy = llvm::StructType::create(mContext, BType, "RegisterB");
  std::vector<llvm::Type *> RType{ mRegWordTy };
  mRegFullTy = llvm::StructType::create(mContext, mRegWordTy, "RegisterR");
  // Generate the assembly context type
  std::vector<llvm::Type *> InputTypes;
  for (size_t i = 0; i < (mIs64 ? 16 : 8); i++)
    InputTypes.push_back(mRegFullTy);
  mInputTy = llvm::StructType::create(mContext, InputTypes, "ContextTy");
  // Generate the function type
  std::vector<llvm::Type *> ArgumentsTypes{ mInputTy->getPointerTo() };
  mFunctionTy = llvm::FunctionType::get(llvm::Type::getVoidTy(mContext), ArgumentsTypes, false);
}

std::string UILifter::getDisassemblyString(const ZydisDecodedInstruction &instruction, size_t address) const {

  std::string disassembled;

  ZydisFormatter formatter;

  if (!ZYAN_SUCCESS(ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL)) ||
    !ZYAN_SUCCESS(ZydisFormatterSetProperty(&formatter, ZYDIS_FORMATTER_PROP_FORCE_SEGMENT, ZYAN_TRUE)) ||
    !ZYAN_SUCCESS(ZydisFormatterSetProperty(&formatter, ZYDIS_FORMATTER_PROP_FORCE_SIZE, ZYAN_TRUE)))
  {
    llvm::report_fatal_error(std::string() + __func__ + ": failed to initialise the Zydis formatter!");
  }

  ZyanU8 buffer[256];
  const ZydisFormatterToken *token;

  if (!ZYAN_SUCCESS(ZydisFormatterTokenizeInstruction(&formatter, &instruction, buffer, sizeof(buffer), address, &token)))
    llvm::report_fatal_error(std::string() + __func__ + ": failed to tokenize the Zydis instruction!");

  ZyanStatus status = ZYAN_STATUS_SUCCESS;
  while (ZYAN_SUCCESS(status)) {
    ZydisTokenType type;
    ZyanConstCharPointer value;
    if (!ZYAN_SUCCESS(ZydisFormatterTokenGetValue(token, &type, &value)))
      llvm::report_fatal_error(std::string() + __func__ + ": failed to get token value!");
    disassembled += value;
    status = ZydisFormatterTokenNext(&token);
  }

  return disassembled;
}

size_t UILifter::getRegisterOffset(const ZydisRegister reg) const {
  switch (reg) {
    case ZYDIS_REGISTER_AL:
    case ZYDIS_REGISTER_AX:
    case ZYDIS_REGISTER_EAX:
    case ZYDIS_REGISTER_RAX:
    case ZYDIS_REGISTER_BL:
    case ZYDIS_REGISTER_BX:
    case ZYDIS_REGISTER_EBX:
    case ZYDIS_REGISTER_RBX:
    case ZYDIS_REGISTER_CL:
    case ZYDIS_REGISTER_CX:
    case ZYDIS_REGISTER_ECX:
    case ZYDIS_REGISTER_RCX:
    case ZYDIS_REGISTER_DL:
    case ZYDIS_REGISTER_DX:
    case ZYDIS_REGISTER_EDX:
    case ZYDIS_REGISTER_RDX:
    case ZYDIS_REGISTER_SIL:
    case ZYDIS_REGISTER_SI:
    case ZYDIS_REGISTER_ESI:
    case ZYDIS_REGISTER_RSI:
    case ZYDIS_REGISTER_DIL:
    case ZYDIS_REGISTER_DI:
    case ZYDIS_REGISTER_EDI:
    case ZYDIS_REGISTER_RDI:
    case ZYDIS_REGISTER_SPL:
    case ZYDIS_REGISTER_SP:
    case ZYDIS_REGISTER_ESP:
    case ZYDIS_REGISTER_RSP:
    case ZYDIS_REGISTER_BPL:
    case ZYDIS_REGISTER_BP:
    case ZYDIS_REGISTER_EBP:
    case ZYDIS_REGISTER_RBP:
    case ZYDIS_REGISTER_R8B:
    case ZYDIS_REGISTER_R8W:
    case ZYDIS_REGISTER_R8D:
    case ZYDIS_REGISTER_R8:
    case ZYDIS_REGISTER_R9B:
    case ZYDIS_REGISTER_R9W:
    case ZYDIS_REGISTER_R9D:
    case ZYDIS_REGISTER_R9:
    case ZYDIS_REGISTER_R10B:
    case ZYDIS_REGISTER_R10W:
    case ZYDIS_REGISTER_R10D:
    case ZYDIS_REGISTER_R10:
    case ZYDIS_REGISTER_R11B:
    case ZYDIS_REGISTER_R11W:
    case ZYDIS_REGISTER_R11D:
    case ZYDIS_REGISTER_R11:
    case ZYDIS_REGISTER_R12B:
    case ZYDIS_REGISTER_R12W:
    case ZYDIS_REGISTER_R12D:
    case ZYDIS_REGISTER_R12:
    case ZYDIS_REGISTER_R13B:
    case ZYDIS_REGISTER_R13W:
    case ZYDIS_REGISTER_R13D:
    case ZYDIS_REGISTER_R13:
    case ZYDIS_REGISTER_R14B:
    case ZYDIS_REGISTER_R14W:
    case ZYDIS_REGISTER_R14D:
    case ZYDIS_REGISTER_R14:
    case ZYDIS_REGISTER_R15B:
    case ZYDIS_REGISTER_R15W:
    case ZYDIS_REGISTER_R15D:
    case ZYDIS_REGISTER_R15:
      return 0;
    case ZYDIS_REGISTER_AH:
    case ZYDIS_REGISTER_BH:
    case ZYDIS_REGISTER_CH:
    case ZYDIS_REGISTER_DH:
      return 1;
    default:
      llvm::report_fatal_error(std::string() + __func__ + ": unknown register!");
  }
}

size_t UILifter::getRegisterIndex(const ZydisRegister reg) const {
  switch (reg) {
    case ZYDIS_REGISTER_AL:
    case ZYDIS_REGISTER_AH:
    case ZYDIS_REGISTER_AX:
    case ZYDIS_REGISTER_EAX:
    case ZYDIS_REGISTER_RAX:
      return 0;
    case ZYDIS_REGISTER_BL:
    case ZYDIS_REGISTER_BH:
    case ZYDIS_REGISTER_BX:
    case ZYDIS_REGISTER_EBX:
    case ZYDIS_REGISTER_RBX:
      return 1;
    case ZYDIS_REGISTER_CL:
    case ZYDIS_REGISTER_CH:
    case ZYDIS_REGISTER_CX:
    case ZYDIS_REGISTER_ECX:
    case ZYDIS_REGISTER_RCX:
      return 2;
    case ZYDIS_REGISTER_DL:
    case ZYDIS_REGISTER_DH:
    case ZYDIS_REGISTER_DX:
    case ZYDIS_REGISTER_EDX:
    case ZYDIS_REGISTER_RDX:
      return 3;
    case ZYDIS_REGISTER_SIL:
    case ZYDIS_REGISTER_SI:
    case ZYDIS_REGISTER_ESI:
    case ZYDIS_REGISTER_RSI:
      return 4;
    case ZYDIS_REGISTER_DIL:
    case ZYDIS_REGISTER_DI:
    case ZYDIS_REGISTER_EDI:
    case ZYDIS_REGISTER_RDI:
      return 5;
    case ZYDIS_REGISTER_SPL:
    case ZYDIS_REGISTER_SP:
    case ZYDIS_REGISTER_ESP:
    case ZYDIS_REGISTER_RSP:
      return 6;
    case ZYDIS_REGISTER_BPL:
    case ZYDIS_REGISTER_BP:
    case ZYDIS_REGISTER_EBP:
    case ZYDIS_REGISTER_RBP:
      return 7;
    case ZYDIS_REGISTER_R8B:
    case ZYDIS_REGISTER_R8W:
    case ZYDIS_REGISTER_R8D:
    case ZYDIS_REGISTER_R8:
      return 8;
    case ZYDIS_REGISTER_R9B:
    case ZYDIS_REGISTER_R9W:
    case ZYDIS_REGISTER_R9D:
    case ZYDIS_REGISTER_R9:
      return 9;
    case ZYDIS_REGISTER_R10B:
    case ZYDIS_REGISTER_R10W:
    case ZYDIS_REGISTER_R10D:
    case ZYDIS_REGISTER_R10:
      return 10;
    case ZYDIS_REGISTER_R11B:
    case ZYDIS_REGISTER_R11W:
    case ZYDIS_REGISTER_R11D:
    case ZYDIS_REGISTER_R11:
      return 11;
    case ZYDIS_REGISTER_R12B:
    case ZYDIS_REGISTER_R12W:
    case ZYDIS_REGISTER_R12D:
    case ZYDIS_REGISTER_R12:
      return 12;
    case ZYDIS_REGISTER_R13B:
    case ZYDIS_REGISTER_R13W:
    case ZYDIS_REGISTER_R13D:
    case ZYDIS_REGISTER_R13:
      return 13;
    case ZYDIS_REGISTER_R14B:
    case ZYDIS_REGISTER_R14W:
    case ZYDIS_REGISTER_R14D:
    case ZYDIS_REGISTER_R14:
      return 14;
    case ZYDIS_REGISTER_R15B:
    case ZYDIS_REGISTER_R15W:
    case ZYDIS_REGISTER_R15D:
    case ZYDIS_REGISTER_R15:
      return 15;
    default:
      llvm::report_fatal_error(std::string() + __func__ + ": unknown register!");
  }
}

llvm::Function *UILifter::Lift(const std::vector<ZyanU8> &bytes, size_t address) const {

  // Decode the instruction with Zydis

  ZydisDecodedInstruction instruction;
  if (!ZYAN_SUCCESS(ZydisDecoderDecodeBuffer(&mDecoder, bytes.data(), bytes.size(), &instruction)))
    llvm::report_fatal_error(std::string() + __func__ + ": failed to disassemble the bytes!");

  // Retrieve the instruction disassembly

  const auto &disassemblyString = getDisassemblyString(instruction, address);

  // Retrieve the implicitly|explicitly read|written registers

  std::set<ZydisRegister> errw; // explicitly read+written GPRs
  std::set<ZydisRegister> erw;  // explicitly written GPRs
  std::set<ZydisRegister> err;  // explicitly read GPRs
  std::set<ZydisRegister> irrw; // implicitly read+written GPRs
  std::set<ZydisRegister> irw;  // implicitly written GPRs
  std::set<ZydisRegister> irr;  // implicitly read GPRs
  std::vector<std::string> icf; // implicitly clobbered flags

  for (ZyanU8 i = 0; i < instruction.operand_count; i++) {
    const auto &op = instruction.operands[i];
    switch (op.type) {
      case ZYDIS_OPERAND_TYPE_REGISTER: {
        switch (ZydisRegisterGetClass(op.reg.value)) {
          case ZYDIS_REGCLASS_GPR8:
          case ZYDIS_REGCLASS_GPR16:
          case ZYDIS_REGCLASS_GPR32:
          case ZYDIS_REGCLASS_GPR64: {
            switch (op.visibility) {
              case ZYDIS_OPERAND_VISIBILITY_EXPLICIT: {
                switch (op.actions) {
                  case ZYDIS_OPERAND_ACTION_READ:
                  case ZYDIS_OPERAND_ACTION_CONDREAD: {
                    err.insert(op.reg.value);
                  } break;
                  case ZYDIS_OPERAND_ACTION_WRITE:
                  case ZYDIS_OPERAND_ACTION_CONDWRITE: {
                    erw.insert(op.reg.value);
                  } break;
                  case ZYDIS_OPERAND_ACTION_READWRITE:
                  case ZYDIS_OPERAND_ACTION_CONDREAD_CONDWRITE:
                  case ZYDIS_OPERAND_ACTION_READ_CONDWRITE:
                  case ZYDIS_OPERAND_ACTION_CONDREAD_WRITE: {
                    errw.insert(op.reg.value);
                  } break;
                }
              } break;
              case ZYDIS_OPERAND_VISIBILITY_HIDDEN:
              case ZYDIS_OPERAND_VISIBILITY_IMPLICIT: {
                if (op.reg.value != ZYDIS_REGISTER_RSP &&
                  op.reg.value != ZYDIS_REGISTER_ESP &&
                  op.reg.value != ZYDIS_REGISTER_SP)
                {
                  switch (op.actions) {
                    case ZYDIS_OPERAND_ACTION_READ:
                    case ZYDIS_OPERAND_ACTION_CONDREAD: {
                      irr.insert(op.reg.value);
                    } break;
                    case ZYDIS_OPERAND_ACTION_WRITE:
                    case ZYDIS_OPERAND_ACTION_CONDWRITE: {
                      irw.insert(op.reg.value);
                    } break;
                    case ZYDIS_OPERAND_ACTION_READWRITE:
                    case ZYDIS_OPERAND_ACTION_CONDREAD_CONDWRITE:
                    case ZYDIS_OPERAND_ACTION_READ_CONDWRITE:
                    case ZYDIS_OPERAND_ACTION_CONDREAD_WRITE: {
                      irrw.insert(op.reg.value);
                    } break;
                  }
                }
              } break;
              default: break;
            }
          } break;
          case ZYDIS_REGCLASS_FLAGS: {
            if (op.actions & ZYDIS_OPERAND_ACTION_MASK_WRITE)
              switch (op.reg.value) {
                case ZYDIS_REGISTER_FLAGS:
                case ZYDIS_REGISTER_EFLAGS:
                case ZYDIS_REGISTER_RFLAGS: {
                  icf.push_back("~{flags}");
                } break;
                default: break;
              }
          } break;
          default: {
            if (op.actions & ZYDIS_OPERAND_ACTION_MASK_WRITE)
              if (op.reg.value == ZYDIS_REGISTER_X87STATUS)
                icf.push_back("~{fpsr}");
          } break;
        }
      } break;
      case ZYDIS_OPERAND_TYPE_MEMORY: {
        switch (ZydisRegisterGetClass(op.mem.base)) {
          case ZYDIS_REGCLASS_GPR8:
          case ZYDIS_REGCLASS_GPR16:
          case ZYDIS_REGCLASS_GPR32:
          case ZYDIS_REGCLASS_GPR64: {
            switch (op.visibility) {
              case ZYDIS_OPERAND_VISIBILITY_EXPLICIT: {
                err.insert(op.mem.base);
              } break;
              case ZYDIS_OPERAND_VISIBILITY_HIDDEN:
              case ZYDIS_OPERAND_VISIBILITY_IMPLICIT: {
                if (op.mem.base != ZYDIS_REGISTER_RSP &&
                  op.mem.base != ZYDIS_REGISTER_ESP &&
                  op.mem.base != ZYDIS_REGISTER_SP)
                {
                  irr.insert(op.mem.base);
                }
              } break;
              default: break;
            }
          } break;
          default: break;
        }
        switch (ZydisRegisterGetClass(op.mem.index)) {
          case ZYDIS_REGCLASS_GPR8:
          case ZYDIS_REGCLASS_GPR16:
          case ZYDIS_REGCLASS_GPR32:
          case ZYDIS_REGCLASS_GPR64: {
            switch (op.visibility) {
              case ZYDIS_OPERAND_VISIBILITY_EXPLICIT: {
                err.insert(op.mem.index);
              } break;
              case ZYDIS_OPERAND_VISIBILITY_HIDDEN:
              case ZYDIS_OPERAND_VISIBILITY_IMPLICIT: {
                if (op.mem.index != ZYDIS_REGISTER_RSP &&
                  op.mem.index != ZYDIS_REGISTER_ESP &&
                  op.mem.index != ZYDIS_REGISTER_SP)
                {
                  irr.insert(op.mem.index);
                }
              } break;
              default: break;
            }
          } break;
          default: break;
        }
        switch (op.visibility) {
          case ZYDIS_OPERAND_VISIBILITY_HIDDEN:
          case ZYDIS_OPERAND_VISIBILITY_IMPLICIT: {
            icf.push_back("~{memory}");
          } break;
          default: break;
        }
      } break;
      case ZYDIS_OPERAND_TYPE_POINTER: {
        switch (op.visibility) {
          case ZYDIS_OPERAND_VISIBILITY_HIDDEN:
          case ZYDIS_OPERAND_VISIBILITY_IMPLICIT: {
            icf.push_back("~{memory}");
          } break;
          default: break;
        }
      } break;
      default: break;
    }
  }

  for (ZydisCPUFlag i = 0; (ZyanUSize)i < ZYAN_ARRAY_LENGTH(instruction.accessed_flags); i++) {
    switch (i) {
      case ZYDIS_CPUFLAG_DF: {
        switch (instruction.accessed_flags[i].action) {
          case ZYDIS_CPUFLAG_ACTION_TESTED_MODIFIED:
          case ZYDIS_CPUFLAG_ACTION_MODIFIED:
          case ZYDIS_CPUFLAG_ACTION_SET_0:
          case ZYDIS_CPUFLAG_ACTION_SET_1:
          case ZYDIS_CPUFLAG_ACTION_UNDEFINED: {
            icf.push_back("~{dirflag}");
          } break;
          default: break;
        }
      } break;
      default: break;
    }
  }

  // Retrieve the implicitly|explicitly read&written registers

  std::set_intersection(irr.begin(), irr.end(), irw.begin(), irw.end(),
    std::inserter(irrw, irrw.begin()));

  for (const auto reg : irrw) {
    irw.erase(reg);
    irr.erase(reg);
  }

  std::set_intersection(err.begin(), err.end(), erw.begin(), erw.end(),
    std::inserter(errw, errw.begin()));

  for (const auto reg : errw) {
    erw.erase(reg);
    err.erase(reg);
  }

  // Generate the format string for the operands

  std::vector<ZydisRegister> ExplicitArguments;
  std::vector<ZydisRegister> ClobberedRegisters;
  std::vector<ZydisRegister> OutputRegisters;
  std::vector<ZydisRegister> InputRegisters;
  std::string ArgumentsFormat;

  std::vector<ZydisRegister> errw_vec(errw.begin(), errw.end());

  size_t IndexOffset = 0;

  for (const auto reg : erw) {
    OutputRegisters.push_back(reg);
    ArgumentsFormat += ("={" + std::string(ZydisRegisterGetString(reg)) + "},");
    IndexOffset++;
  }

  for (const auto reg : irrw) {
    OutputRegisters.push_back(reg);
    InputRegisters.push_back(reg);
    ArgumentsFormat += ("={" + std::string(ZydisRegisterGetString(reg)) + "},");
    IndexOffset++;
  }

  for (const auto reg : irw) {
    OutputRegisters.push_back(reg);
    ArgumentsFormat += ("={" + std::string(ZydisRegisterGetString(reg)) + "},");
    IndexOffset++;
  }

  for (const auto reg : irrw) {
    ArgumentsFormat += ("{" + std::string(ZydisRegisterGetString(reg)) + "},");
    IndexOffset++;
  }

  for (const auto reg : irr) {
    InputRegisters.push_back(reg);
    ArgumentsFormat += ("{" + std::string(ZydisRegisterGetString(reg)) + "},");
  }

  for (const auto reg : errw) {
    OutputRegisters.push_back(reg);
    ArgumentsFormat += ("=r,");
  }

  for (const auto reg : err) {
    ExplicitArguments.push_back(reg);
    InputRegisters.push_back(reg);
    ArgumentsFormat += ("r,");
  }

  for (size_t i = 0; i < errw_vec.size(); i++) {
    const auto reg = errw_vec[i];
    ExplicitArguments.insert(ExplicitArguments.begin(), reg);
    InputRegisters.push_back(reg);
    ArgumentsFormat += (std::to_string(i) + ",");
  }

  for (const auto &cf : icf)
    ArgumentsFormat += (cf + ",");

  if (!ArgumentsFormat.empty())
    ArgumentsFormat.pop_back();

  // Generate the format string for the assembly

  std::string AssemblyFormat(disassemblyString);

  auto replaceAll = [](std::string &str, const std::string &from, const std::string &to) {
    if (from.empty())
      return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
      str.replace(start_pos, from.length(), to);
      start_pos += to.length();
    }
  };

  for (size_t i = 0; i < ExplicitArguments.size(); i++)
    replaceAll(AssemblyFormat, ZydisRegisterGetString(ExplicitArguments[i]), "$" + std::to_string(IndexOffset + i));

  // Debug print the information about the instruction

  if (mDebug) {
    llvm::outs() << "> " << disassemblyString << "\n";
    if (!irrw.empty()) {
      llvm::outs() << "[+] Implicitly read and written register(s):";
      for (const auto reg : irrw)
        llvm::outs() << " " << ZydisRegisterGetString(reg);
      llvm::outs() << "\n";
    }
    if (!irw.empty()) {
      llvm::outs() << "[+] Implicitly written register(s):";
      for (const auto &reg : irw)
        llvm::outs() << " " << ZydisRegisterGetString(reg);
      llvm::outs() << "\n";
    }
    if (!irr.empty()) {
      llvm::outs() << "[+] Implicitly read register(s):";
      for (const auto &reg : irr)
        llvm::outs() << " " << ZydisRegisterGetString(reg);
      llvm::outs() << "\n";
    }
    if (!errw.empty()) {
      llvm::outs() << "[+] Explicitly read and written register(s):";
      for (const auto &reg : errw)
        llvm::outs() << " " << ZydisRegisterGetString(reg);
      llvm::outs() << "\n";
    }
    if (!erw.empty()) {
      llvm::outs() << "[+] Explicitly written register(s):";
      for (const auto &reg : erw)
        llvm::outs() << " " << ZydisRegisterGetString(reg);
      llvm::outs() << "\n";
    }
    if (!err.empty()) {
      llvm::outs() << "[+] Explicitly read register(s):";
      for (const auto &reg : err)
        llvm::outs() << " " << ZydisRegisterGetString(reg);
      llvm::outs() << "\n";
    }
    if (!ExplicitArguments.empty()) {
      llvm::outs() << "[+] Explicit arguments list:";
      for (const auto &reg : ExplicitArguments)
        llvm::outs() << " " << ZydisRegisterGetString(reg);
      llvm::outs() << "\n";
    }
    llvm::outs() << "[+] Arguments format: " << ArgumentsFormat << "\n";
    llvm::outs() << "[+] IndexOffset: " << IndexOffset << "\n";
    llvm::outs() << "[+] AssemblyFormat format: " << AssemblyFormat << "\n";
  }

  // Generate the input type

  std::vector<llvm::Type *> InputTypes;
  for (const auto reg : InputRegisters)
    InputTypes.push_back(llvm::IntegerType::get(mContext, ZydisRegisterGetWidth(mMode, reg)));

  // Generate the output type

  llvm::Type *OutputTy = llvm::Type::getVoidTy(mContext);
  if (OutputRegisters.size() == 1) {
    OutputTy = llvm::IntegerType::get(mContext, ZydisRegisterGetWidth(mMode, OutputRegisters[0]));
  } else if (OutputRegisters.size() > 1) {
    std::vector<llvm::Type *> OutputTypes;
    for (const auto reg : OutputRegisters)
      OutputTypes.push_back(llvm::IntegerType::get(mContext, ZydisRegisterGetWidth(mMode, reg)));
    OutputTy = llvm::StructType::create(mContext, OutputTypes, "IAOutTy");
  }

  // Generate the inline assembly function type

  auto *InlineAsmTy = llvm::FunctionType::get(OutputTy, InputTypes, false);

  // Generate the function and the entry block

  const std::string FunctionName = "Unsupported_" + std::string(ZydisMnemonicGetString(instruction.mnemonic));
  auto *InlineAsmFunction = llvm::Function::Create(mFunctionTy, llvm::Function::ExternalLinkage, FunctionName, mModule);
  auto *InlineAsmBlock = llvm::BasicBlock::Create(mContext, "", InlineAsmFunction);
  InlineAsmFunction->addFnAttr(llvm::Attribute::AlwaysInline);

  // Load the input registers

  std::vector<llvm::Value *> Args;
  auto *InContext = InlineAsmFunction->getArg(0);
  for (const auto reg : InputRegisters) {
    auto *ArgTy = llvm::IntegerType::get(mContext, ZydisRegisterGetWidth(mMode, reg));
    auto *PtrTy = llvm::PointerType::get(ArgTy, 0);
    std::vector<llvm::Value *> Index{
      llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 64), 0),
      llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 32), getRegisterIndex(reg)),
      llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 32), 0)
    };
    std::vector<llvm::Value *> Offset{
      llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 64), 0),
      llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 32), getRegisterOffset(reg))
    };
    auto *Ptr0 = llvm::GetElementPtrInst::CreateInBounds(mInputTy, InContext, Index, "", InlineAsmBlock);
    auto *Bc0 = new llvm::BitCastInst(Ptr0, mRegByteTy->getPointerTo(), "", InlineAsmBlock);
    auto *Ptr1 = llvm::GetElementPtrInst::CreateInBounds(mRegByteTy, Bc0, Offset, "", InlineAsmBlock);
    auto *Bc1 = new llvm::BitCastInst(Ptr1, PtrTy, "", InlineAsmBlock);
    auto *Reg = new llvm::LoadInst(ArgTy, Bc1, "", InlineAsmBlock);
    Args.push_back(Reg);
  }

  // Select the proper inline assembly dialect

  auto Dialect = llvm::InlineAsm::AsmDialect::AD_Intel;
  switch (instruction.mnemonic) {
    case ZYDIS_MNEMONIC_CALL: {
      Dialect = llvm::InlineAsm::AsmDialect::AD_ATT;
    } break;
    default: break;
  }

  // Call the inline assembly instruction

  auto *InlineAsm = llvm::InlineAsm::get(InlineAsmTy, AssemblyFormat, ArgumentsFormat, true, false, Dialect);
  auto *Call = llvm::CallInst::Create(InlineAsm, Args, "", InlineAsmBlock);
  Call->addAttribute(llvm::AttributeList::FunctionIndex, llvm::Attribute::NoUnwind);

  // Store the output registers

  if (OutputRegisters.size() == 1) {
    const auto reg = OutputRegisters[0];
    auto *ArgTy = llvm::IntegerType::get(mContext, ZydisRegisterGetWidth(mMode, reg));
    auto *PtrTy = llvm::PointerType::get(ArgTy, 0);
    std::vector<llvm::Value *> Index{
      llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 64), 0),
      llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 32), getRegisterIndex(reg)),
      llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 32), 0)
    };
    std::vector<llvm::Value *> Offset{
      llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 64), 0),
      llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 32), getRegisterOffset(reg))
    };
    auto *Ptr0 = llvm::GetElementPtrInst::CreateInBounds(mInputTy, InContext, Index, "", InlineAsmBlock);
    auto *Bc0 = new llvm::BitCastInst(Ptr0, mRegByteTy->getPointerTo(), "", InlineAsmBlock);
    auto *Ptr1 = llvm::GetElementPtrInst::CreateInBounds(mRegByteTy, Bc0, Offset, "", InlineAsmBlock);
    auto *Bc1 = new llvm::BitCastInst(Ptr1, PtrTy, "", InlineAsmBlock);
    (void)new llvm::StoreInst(Call, Bc1, InlineAsmBlock);
  } else if (OutputRegisters.size() > 1) {
    for (unsigned int i = 0; i < OutputRegisters.size(); i++) {
      const auto reg = OutputRegisters[i];
      auto *ArgTy = llvm::IntegerType::get(mContext, ZydisRegisterGetWidth(mMode, reg));
      auto *PtrTy = llvm::PointerType::get(ArgTy, 0);
      std::vector<llvm::Value *> Index{
        llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 64), 0),
        llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 32), getRegisterIndex(reg)),
        llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 32), 0)
      };
      std::vector<llvm::Value *> Offset{
        llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 64), 0),
        llvm::ConstantInt::get(llvm::IntegerType::get(mContext, 32), getRegisterOffset(reg))
      };
      auto *Ptr0 = llvm::GetElementPtrInst::CreateInBounds(mInputTy, InContext, Index, "", InlineAsmBlock);
      auto *Bc0 = new llvm::BitCastInst(Ptr0, mRegByteTy->getPointerTo(), "", InlineAsmBlock);
      auto *Ptr1 = llvm::GetElementPtrInst::CreateInBounds(mRegByteTy, Bc0, Offset, "", InlineAsmBlock);
      auto *Bc1 = new llvm::BitCastInst(Ptr1, PtrTy, "", InlineAsmBlock);
      auto *Agg = llvm::ExtractValueInst::Create(Call, { i }, "", InlineAsmBlock);
      (void)new llvm::StoreInst(Agg, Bc1, InlineAsmBlock);
    }
  }

  // Return void

  llvm::ReturnInst::Create(mContext, InlineAsmBlock);

  // Return the function pointer

  return InlineAsmFunction;
}

int main() {

  llvm::LLVMContext Context;
  llvm::Module Module("Module", Context);

  const auto &UIL = UILifter::Get(Module);

  UIL.Lift({ 0x5C });
  UIL.Lift({ 0xFD });
  UIL.Lift({ 0x00, 0xDC });
  UIL.Lift({ 0xD9, 0xFA });
  UIL.Lift({ 0xD9, 0xFB });
  UIL.Lift({ 0xDD, 0x18 });
  UIL.Lift({ 0x01, 0xD8 });
  UIL.Lift({ 0x0F, 0xA2 });
  UIL.Lift({ 0x0F, 0x31 });
  UIL.Lift({ 0x48, 0xF7, 0xF1 });
  UIL.Lift({ 0x48, 0x01, 0xD8 });
  UIL.Lift({ 0x48, 0x0F, 0xCB });
  UIL.Lift({ 0x48, 0xF7, 0xF0 });
  UIL.Lift({ 0x48, 0x89, 0x7C, 0x56, 0x08 });
  UIL.Lift({ 0x48, 0x8B, 0x74, 0x76, 0x08 });
  UIL.Lift({ 0xE8, 0x8C, 0x07, 0x00, 0x00 }, 0x1400016CF);
  UIL.Lift({ 0x48, 0xC7, 0xC4, 0x00, 0x10, 0x00, 0x00 });
  UIL.Lift({ 0x48, 0x81, 0xC4, 0x00, 0x10, 0x00, 0x00 });

  Module.dump();

  return 0;
}