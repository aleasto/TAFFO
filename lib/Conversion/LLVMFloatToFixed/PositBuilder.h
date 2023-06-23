#include "FixedPointType.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"

namespace flttofix
{

class PositBuilder {
public:
  PositBuilder(llvm::IRBuilderBase &builder, const FixedPointType &metadata)
    : builder(builder)
    , C(this->builder.getContext())
    , M(this->builder.GetInsertBlock()->getParent()->getParent())
    , metadata(metadata)
    , llvmType(llvm::cast<llvm::StructType>(this->metadata.scalarToLLVMType(builder.getContext())))
  {}

  llvm::Value *CreateConstructor(llvm::Value *arg1, bool isSigned=true);
  llvm::Value *CreateCopy(llvm::Value* dst, llvm::Value* src, bool isVolatile=false);
  llvm::Value *CreateConv(llvm::Value* from, llvm::Type *dstType);
  llvm::Value *CreateBinOp(int opcode, llvm::Value* arg1, llvm::Value* arg2);
  llvm::Value *CreateUnaryOp(int opcode, llvm::Value* arg1);
  llvm::Value *CreateCmp(llvm::CmpInst::Predicate pred, llvm::Value* arg1, llvm::Value* arg2);
  llvm::Value *CreateFMA(llvm::Value *arg1, llvm::Value *arg2, llvm::Value *arg3);

private:
  llvm::IRBuilderBase &builder;
  llvm::LLVMContext &C;
  llvm::Module *M;

  const FixedPointType &metadata;
  llvm::StructType *llvmType;
};

}