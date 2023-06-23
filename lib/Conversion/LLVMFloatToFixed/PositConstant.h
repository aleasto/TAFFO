#include "FixedPointType.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"


namespace PositConstant {
  llvm::Constant *getData(llvm::Module *M, llvm::LLVMContext &C, const flttofix::FixedPointType &fixpt, double floatVal);
  llvm::Constant *get(llvm::Module *M, llvm::LLVMContext &C, const flttofix::FixedPointType &fixpt, double floatVal);
  llvm::Constant *FoldBinOp(llvm::Module *M, llvm::LLVMContext &C, const flttofix::FixedPointType &fixpt,
                            int opcode, llvm::Constant *c1, llvm::Constant *c2);
  llvm::Constant *FoldUnaryOp(llvm::Module *M, llvm::LLVMContext &C, const flttofix::FixedPointType &fixpt,
                            int opcode, llvm::Constant *c);
  llvm::Constant *FoldConv(llvm::Module *M, llvm::LLVMContext &C, const flttofix::FixedPointType &fixpt,
                           llvm::Constant *src, llvm::Type *dstType);
}
