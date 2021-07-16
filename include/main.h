#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

#include <Zydis/Zydis.h>

class UILifter {
public:

  static UILifter &Get(llvm::Module &Module, bool Is64 = true, bool Debug = false) {
    static UILifter instance(Module, Is64, Debug);
    return instance;
  }

  llvm::Function *Lift(const std::vector<ZyanU8> &bytes) const;

  UILifter(UILifter const &)       = delete;
  void operator=(UILifter const &) = delete;

private:

  UILifter(llvm::Module &Module, bool Is64, bool IsDebug);

  std::string getDisassemblyString(const ZydisDecodedInstruction &instruction) const;

  bool isSupportedRegister(const ZydisRegister reg) const;

  size_t getContextIndex(const ZydisRegister reg) const;

  bool mIs64 = false;
  bool mDebug = false;

  ZydisDecoder mDecoder;
  ZydisMachineMode mMode;
  ZydisAddressWidth mWidth;

  llvm::Module &mModule;
  llvm::LLVMContext &mContext;
  llvm::Type *mInputTy = nullptr;
  llvm::FunctionType *mFunctionTy = nullptr;

};