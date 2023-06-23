#include "PositBuilder.h"
#include "PositConstant.h"

using namespace llvm;
using namespace flttofix;

#define DEBUG_TYPE "taffo-conversion"

Value *PositBuilder::CreateConstructor(Value *arg1, bool isSigned) {
  const char* mangledName;
  Type* srcType = arg1->getType();

  switch (metadata.scalarBitsAmt()) {
  case 32:
    if (srcType->isFloatTy()) {
      mangledName = "_ZN5PositIiLi32ELi2EjL9PositSpec1EEC1Ef";
    } else if (srcType->isDoubleTy()) {
      mangledName = "_ZN5PositIiLi32ELi2EjL9PositSpec1EEC1Ed";
    } else if (srcType->isIntegerTy()) {
      // TODO: Support bigger integer types without overflowing sint32
      if (isSigned)
        arg1 = builder.CreateSExtOrTrunc(arg1, Type::getInt32Ty(C));
      else
        arg1 = builder.CreateZExtOrTrunc(arg1, Type::getInt32Ty(C));
      mangledName = "_ZN5PositIiLi32ELi2EjL9PositSpec1EEC1Ei";
    } else {
      llvm_unreachable("Unimplemented constructor from source type");
    }
    break;
  default:
    llvm_unreachable("Unimplemented Posit size");
  }

  Value *dst = builder.CreateAlloca(llvmType);
  FunctionType *fnType = FunctionType::get(
      Type::getVoidTy(C), /* Return type */
      { PointerType::get(llvmType, 0), arg1->getType() }, /* Arguments... */
      false /* isVarArg */
  );
  FunctionCallee ctorFun = M->getOrInsertFunction(mangledName, fnType);
  builder.CreateCall(ctorFun, {dst, arg1});
  return dst;
}

Value *PositBuilder::CreateBinOp(int opcode, Value *arg1, Value *arg2) {
  if (Constant *c1 = dyn_cast<Constant>(arg1)) {
    if (Constant *c2 = dyn_cast<Constant>(arg2)) {
      LLVM_DEBUG(dbgs() << "Attempting to fold constant Posit operation\n");
      Constant *res = PositConstant::FoldBinOp(M, C, metadata, opcode, c1, c2);
      if (res) {
        LLVM_DEBUG(dbgs() << "Folded in " << *res << "\n");
        return res;
      } else {
        LLVM_DEBUG(dbgs() << "Constant folding failed; falling back to runtime computation\n");
      }
    }
  }

  const char* mangledName;
  switch (metadata.scalarBitsAmt()) {
  case 32:
    switch (opcode) {
    case Instruction::FAdd:
      mangledName = "_ZN5PositIiLi32ELi2EjL9PositSpec1EEplERKS1_";
      break;
    case Instruction::FSub:
      mangledName = "_ZN5PositIiLi32ELi2EjL9PositSpec1EEmiERKS1_";
      break;
    case Instruction::FMul:
      mangledName = "_ZN5PositIiLi32ELi2EjL9PositSpec1EEmlERKS1_";
      break;
    case Instruction::FDiv:
      mangledName = "_ZN5PositIiLi32ELi2EjL9PositSpec1EEdvERKS1_";
      break;
    default:
      llvm_unreachable("Unimplemented Posit binary operation");
    }
    break;
  default:
    llvm_unreachable("Unimplemented Posit size");
  }

  FunctionType *fnType = FunctionType::get(
    llvmType->getTypeAtIndex(0U), /* Return type */
    { arg1->getType(), arg2->getType() }, /* Arguments... */
    false /* isVarArg */
  );
  FunctionCallee fun = M->getOrInsertFunction(mangledName, fnType);
  Value *dst = builder.CreateAlloca(llvmType);
  Value *ret = builder.CreateCall(fun, {arg1, arg2});
  Value *lea = builder.CreateStructGEP(llvmType, dst, 0);
  builder.CreateStore(ret, lea);
  return dst;
}

Value *PositBuilder::CreateUnaryOp(int opcode, Value *arg1) {
  if (Constant *c = dyn_cast<Constant>(arg1)) {
    LLVM_DEBUG(dbgs() << "Attempting to fold constant Posit operation\n");
    Constant *res = PositConstant::FoldUnaryOp(M, C, metadata, opcode, c);
    if (res) {
      LLVM_DEBUG(dbgs() << "Folded in " << *res << "\n");
      return res;
    } else {
      LLVM_DEBUG(dbgs() << "Constant folding failed; falling back to runtime computation\n");
    }
  }

  const char* mangledName;
  switch (metadata.scalarBitsAmt()) {
  case 32:
    switch (opcode) {
    case Instruction::FNeg:
      mangledName = "_ZNK5PositIiLi32ELi2EjL9PositSpec1EEngEv";
      break;
    default:
      llvm_unreachable("Unimplemented Posit unary operation");
    }
    break;
  default:
    llvm_unreachable("Unimplemented Posit size");
  }

  FunctionType *fnType = FunctionType::get(
    llvmType->getTypeAtIndex(0U), /* Return type */
    { arg1->getType() }, /* Arguments... */
    false /* isVarArg */
  );
  FunctionCallee fun = M->getOrInsertFunction(mangledName, fnType);
  Value *dst = builder.CreateAlloca(llvmType);
  Value *ret = builder.CreateCall(fun, {arg1});
  Value *lea = builder.CreateStructGEP(llvmType, dst, 0);
  builder.CreateStore(ret, lea);
  return dst;
}

Value *PositBuilder::CreateCmp(CmpInst::Predicate pred, Value *arg1, Value *arg2) {
  assert((pred >= CmpInst::FIRST_ICMP_PREDICATE && pred <= CmpInst::LAST_ICMP_PREDICATE) &&
      "Please provide an integer comparison predicate");

  const char* mangledName;
  switch (metadata.scalarBitsAmt()) {
  case 32:
    switch (pred) {
    case CmpInst::ICMP_EQ:
      mangledName = "_ZN5PositIiLi32ELi2EjL9PositSpec1EEeqERKS1_";
      break;
    case CmpInst::ICMP_NE:
      mangledName = "_ZN5PositIiLi32ELi2EjL9PositSpec1EEneERKS1_";
      break;
    case CmpInst::ICMP_SGT:
      mangledName = "_ZN5PositIiLi32ELi2EjL9PositSpec1EEgtERKS1_";
      break;
    case CmpInst::ICMP_SGE:
      mangledName = "_ZN5PositIiLi32ELi2EjL9PositSpec1EEgeERKS1_";
      break;
    case CmpInst::ICMP_SLT:
      mangledName = "_ZN5PositIiLi32ELi2EjL9PositSpec1EEltERKS1_";
      break;
    case CmpInst::ICMP_SLE:
      mangledName = "_ZN5PositIiLi32ELi2EjL9PositSpec1EEleERKS1_";
      break;
    default:
      llvm_unreachable("Unimplemented Posit comparison operation");
    }
    break;
  default:
    llvm_unreachable("Unimplemented Posit size");
  }

  FunctionType *fnType = FunctionType::get(
    Type::getInt1Ty(C), /* Return type */
    { arg1->getType(), arg2->getType() }, /* Arguments... */
    false /* isVarArg */
  );

  FunctionCallee fun = M->getOrInsertFunction(mangledName, fnType);
  return builder.CreateCall(fun, {arg1, arg2});
}

Value *PositBuilder::CreateCopy(Value *dst, Value *src, bool isVolatile) {
  const DataLayout &dl = M->getDataLayout();
  uint64_t sizeInBytes = dl.getTypeAllocSize(llvmType);
  return builder.CreateMemCpy(dst, dst->getPointerAlignment(dl), src, src->getPointerAlignment(dl), sizeInBytes, isVolatile);
}

Value *PositBuilder::CreateConv(Value *from, Type *dstType) {
  if (Constant *c = dyn_cast<Constant>(from)) {
    LLVM_DEBUG(dbgs() << "Attempting to fold constant Posit conversion\n");
    Constant *res = PositConstant::FoldConv(M, C, metadata, c, dstType);
    if (res) {
      LLVM_DEBUG(dbgs() << "Folded in " << *res << "\n");
      return res;
    } else {
      LLVM_DEBUG(dbgs() << "Constant folding failed; falling back to runtime computation\n");
    }
  }

  const char* mangledName;
  Type* callDstType = dstType;

  // TODO implement casting to bigger or smaller Posit
  switch (metadata.scalarBitsAmt()) {
  case 32:
    if (dstType->isFloatTy()) {
      mangledName = "_ZNK5PositIiLi32ELi2EjL9PositSpec1EEcvfEv";
    } else if (dstType->isDoubleTy()) {
      mangledName = "_ZNK5PositIiLi32ELi2EjL9PositSpec1EEcvdEv";
    } else if (dstType->isFloatingPointTy()) {
      mangledName = "_ZNK5PositIiLi32ELi2EjL9PositSpec1EEcvdEv";
      callDstType = Type::getDoubleTy(C);
    } else if (dstType->isIntegerTy()) {
      mangledName = "_ZNK5PositIiLi32ELi2EjL9PositSpec1EEcviEv";
      callDstType = Type::getInt32Ty(C);
    } else {
      llvm_unreachable("Unimplemented conversion from Posit32 to other numeric type");
    }
    break;
  default:
    llvm_unreachable("Unimplemented Posit size");
  }

  FunctionType *fnType = FunctionType::get(
    callDstType, /* Return type */
    { PointerType::get(llvmType, 0) }, /* Arguments... */
    false /* isVarArg */
  );
  FunctionCallee convFun = M->getOrInsertFunction(mangledName, fnType);
  Value* ret = builder.CreateCall(convFun, { from });

  if (dstType->isFloatingPointTy() && !dstType->isDoubleTy() && !dstType->isFloatTy()) {
    ret = builder.CreateFPTrunc(ret, dstType);
  }

  // TODO: Support bigger integer types without losing information
  // Currently values are clamped to [MIN_INT32, MAX_INT32]
  if (dstType->isIntegerTy()) {
    ret = builder.CreateSExtOrTrunc(ret, dstType);
  }

  return ret;
}

Value *PositBuilder::CreateFMA(Value *arg1, Value *arg2, Value *arg3) {
  const char* mangledName;
  switch (metadata.scalarBitsAmt()) {
  case 32:
    mangledName = "_ZN5PositIiLi32ELi2EjL9PositSpec1EE3fmaERKS1_S3_";
    break;
  default:
    llvm_unreachable("Unimplemented Posit size");
  }

  FunctionType *fnType = FunctionType::get(
    llvmType->getTypeAtIndex(0U), /* Return type */
    { arg1->getType(), arg2->getType(), arg3->getType() }, /* Arguments... */
    false /* isVarArg */
  );

  FunctionCallee fun = M->getOrInsertFunction(mangledName, fnType);
  Value *dst = builder.CreateAlloca(llvmType);
  Value *ret = builder.CreateCall(fun, {arg1, arg2, arg3});
  Value *lea = builder.CreateStructGEP(llvmType, dst, 0);
  builder.CreateStore(ret, lea);
  return dst;
}
