#ifndef POSIT_BUILDER_RISCV
#define POSIT_BUILDER_RISCV

#include "PositBuilder.h"

namespace flttofix
{

class PositBuilderRISCV : public PositBuilder {
public:
  using PositBuilder::PositBuilder;

  llvm::Value *CreateConstructor(llvm::Value *arg1, const FixedPointType *srcMetadata = nullptr) override;
  llvm::Value *CreateConv(llvm::Value *from, llvm::Type *dstType, const FixedPointType *dstMetadata = nullptr) override;
  llvm::Value *CreateBinOp(int opcode, llvm::Value *arg1, llvm::Value *arg2) override;
  llvm::Value *CreateUnaryOp(int opcode, llvm::Value *arg1) override;
  llvm::Value *CreateCmp(llvm::CmpInst::Predicate pred, llvm::Value *arg1, llvm::Value *arg2) override;
  llvm::Value *CreateFMA(llvm::Value *arg1, llvm::Value *arg2, llvm::Value *arg3) override;
};

}

#endif