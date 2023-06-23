#include "PositConstant.h"
#include "llvm/Analysis/ConstantFolding.h"
#include <posit.h>

#define DEBUG_TYPE "taffo-conversion"

using namespace llvm;
using namespace flttofix;

template <class T,int totalbits, int esbits, class FT, PositSpec positspec>
static Constant *getData(Module *M, LLVMContext &C, const FixedPointType &fixpt, Posit<T,totalbits,esbits,FT,positspec> posit) {
  assert(totalbits == fixpt.scalarBitsAmt() && "Mismatching arguments");

  Constant *innerRepr;
  switch (fixpt.scalarBitsAmt()) {
  case 32:
    innerRepr = ConstantInt::getSigned(Type::getInt32Ty(C), posit.v);
    break;
  default:
    llvm_unreachable("Unimplemented Posit size");
  }

  StructType *type = cast<llvm::StructType>(fixpt.scalarToLLVMType(C));
  return ConstantStruct::get(type, { innerRepr });
}

template <class T,int totalbits, int esbits, class FT, PositSpec positspec>
static Constant *get(Module *M, LLVMContext &C, const FixedPointType &fixpt, Posit<T,totalbits,esbits,FT,positspec> posit) {
  Constant *structInit = getData(M, C, fixpt, posit);
  return new GlobalVariable(*M, structInit->getType(), true /* isConstant */, GlobalVariable::LinkageTypes::PrivateLinkage, structInit);
}

template <class T,int totalbits, int esbits, class FT, PositSpec positspec>
static Constant *FoldBinOp(Module *M, LLVMContext &C, const FixedPointType &fixpt, int opcode,
                           Posit<T,totalbits,esbits,FT,positspec> x,
                           Posit<T,totalbits,esbits,FT,positspec> y) {
  Posit<T,totalbits,esbits,FT,positspec> res;
  switch (opcode) {
  case Instruction::FAdd:
    res = x + y;
    break;
  case Instruction::FSub:
    res = x - y;
    break;
  case Instruction::FMul:
    res = x * y;
    break;
  case Instruction::FDiv:
    res = x / y;
    break;
  default:
    LLVM_DEBUG(dbgs() << "Unimplemented constant Posit binary operation\n");
    return nullptr;
  }

  return get(M, C, fixpt, res);
}

template <class T,int totalbits, int esbits, class FT, PositSpec positspec>
static Constant *FoldUnaryOp(Module *M, LLVMContext &C, const FixedPointType &fixpt, int opcode,
                           Posit<T,totalbits,esbits,FT,positspec> x) {
  Posit<T,totalbits,esbits,FT,positspec> res;
  switch (opcode) {
  case Instruction::FNeg:
    res = -x;
    break;
  default:
    LLVM_DEBUG(dbgs() << "Unimplemented constant Posit unary operation");
    return nullptr;
  }

  return get(M, C, fixpt, res);
}

template <class T,int totalbits, int esbits, class FT, PositSpec positspec>
static Constant *FoldConv(Module *M, LLVMContext &C, const FixedPointType &fixpt, Posit<T,totalbits,esbits,FT,positspec> src, Type *dstType) {
  if (dstType->isDoubleTy() || dstType->isFloatTy()) {
    return ConstantFP::get(dstType, (double)src);
  } else if (dstType->isFloatingPointTy()) {
    // Convert to double then fold to dest type
    Constant *DblRes = ConstantFP::get(Type::getDoubleTy(C), (double)src);
    return ConstantFoldCastOperand(Instruction::FPTrunc, DblRes, dstType, M->getDataLayout());
  } else if (dstType->isIntegerTy()) {
    return ConstantInt::get(dstType, (int)src, true /* IsSigned */);
  } else {
    LLVM_DEBUG(dbgs() << "Unimplemented constant Posit conversion\n");
    return nullptr;
  }
}

Constant *PositConstant::getData(Module *M, LLVMContext &C, const FixedPointType &fixpt, double floatVal) {
  switch (fixpt.scalarBitsAmt()) {
  case 32:
    {
      Posit<int32_t, 32, 2, uint32_t, PositSpec::WithInf> posit(floatVal);
      return getData(M, C, fixpt, posit);
    }
  default:
    llvm_unreachable("Unimplemented Posit size");
  }
}

Constant *PositConstant::get(Module *M, LLVMContext &C, const FixedPointType &fixpt, double floatVal) {
  Constant *structInit = getData(M, C, fixpt, floatVal);
  return new GlobalVariable(*M, structInit->getType(), true /* isConstant */, GlobalVariable::LinkageTypes::PrivateLinkage, structInit);
}

Constant *PositConstant::FoldBinOp(Module *M, LLVMContext &C, const FixedPointType &fixpt, int opcode, Constant *c1, Constant *c2) {
  GlobalVariable *g1 = dyn_cast<GlobalVariable>(c1);
  GlobalVariable *g2 = dyn_cast<GlobalVariable>(c2);
  assert((g1 && g2) && "Expected two global variables; that's the only way we store Posit constants");

  ConstantInt *v1 = dyn_cast<ConstantInt>(g1->getInitializer()->getAggregateElement(0U));
  ConstantInt *v2 = dyn_cast<ConstantInt>(g2->getInitializer()->getAggregateElement(0U));
  assert((v1 && v2) && "Expected two Posit structs");

  switch (fixpt.scalarBitsAmt()) {
  case 32:
    {
      Posit<int32_t, 32, 2, uint32_t, PositSpec::WithInf> x({}, (int32_t)v1->getSExtValue());
      Posit<int32_t, 32, 2, uint32_t, PositSpec::WithInf> y({}, (int32_t)v2->getSExtValue());
      return FoldBinOp(M, C, fixpt, opcode, x, y);
    }
  default:
    llvm_unreachable("Unimplemented Posit size");
  }
}

Constant *PositConstant::FoldUnaryOp(Module *M, LLVMContext &C, const FixedPointType &fixpt, int opcode, Constant *c) {
  GlobalVariable *g = dyn_cast<GlobalVariable>(c);
  assert(g && "Expected a global variable; that's the only way we store Posit constants");

  ConstantInt *v = dyn_cast<ConstantInt>(g->getInitializer()->getAggregateElement(0U));
  assert(v && "Expected a Posit struct");

  switch (fixpt.scalarBitsAmt()) {
  case 32:
    {
      Posit<int32_t, 32, 2, uint32_t, PositSpec::WithInf> x({}, (int32_t)v->getSExtValue());
      return FoldUnaryOp(M, C, fixpt, opcode, x);
    }
  default:
    llvm_unreachable("Unimplemented Posit size");
  }
}

Constant *PositConstant::FoldConv(Module *M, LLVMContext &C, const FixedPointType &fixpt, Constant *src, Type *dstType) {
  GlobalVariable *g = dyn_cast<GlobalVariable>(src);
  assert(g && "Expected a global variable; that's the only way we store Posit constants");

  ConstantInt *v = dyn_cast<ConstantInt>(g->getInitializer()->getAggregateElement(0U));
  assert(v && "Expected a Posit struct");

  switch (fixpt.scalarBitsAmt()) {
  case 32:
    {
      Posit<int32_t, 32, 2, uint32_t, PositSpec::WithInf> x({}, (int32_t)v->getSExtValue());
      return FoldConv(M, C, fixpt, x, dstType);
    }
  default:
    llvm_unreachable("Unimplemented Posit size");
  }
}
