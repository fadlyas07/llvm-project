//===- LegalizeForLLVMExport.cpp - Prepare X86Vector for LLVM translation -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/X86Vector/Transforms.h"

#include "mlir/Conversion/LLVMCommon/ConversionTarget.h"
#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/X86Vector/X86VectorDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/PatternMatch.h"

using namespace mlir;
using namespace mlir::x86vector;

/// Extracts the "main" vector element type from the given X86Vector operation.
template <typename OpTy>
static Type getSrcVectorElementType(OpTy op) {
  return cast<VectorType>(op.getSrc().getType()).getElementType();
}
template <>
Type getSrcVectorElementType(Vp2IntersectOp op) {
  return cast<VectorType>(op.getA().getType()).getElementType();
}

namespace {

/// Base conversion for AVX512 ops that can be lowered to one of the two
/// intrinsics based on the bitwidth of their "main" vector element type. This
/// relies on the to-LLVM-dialect conversion helpers to correctly pack the
/// results of multi-result intrinsic ops.
template <typename OpTy, typename Intr32OpTy, typename Intr64OpTy>
struct LowerToIntrinsic : public OpConversionPattern<OpTy> {
  explicit LowerToIntrinsic(const LLVMTypeConverter &converter)
      : OpConversionPattern<OpTy>(converter, &converter.getContext()) {}

  const LLVMTypeConverter &getTypeConverter() const {
    return *static_cast<const LLVMTypeConverter *>(
        OpConversionPattern<OpTy>::getTypeConverter());
  }

  LogicalResult
  matchAndRewrite(OpTy op, typename OpTy::Adaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    Type elementType = getSrcVectorElementType<OpTy>(op);
    unsigned bitwidth = elementType.getIntOrFloatBitWidth();
    if (bitwidth == 32)
      return LLVM::detail::oneToOneRewrite(
          op, Intr32OpTy::getOperationName(), adaptor.getOperands(),
          op->getAttrs(), getTypeConverter(), rewriter);
    if (bitwidth == 64)
      return LLVM::detail::oneToOneRewrite(
          op, Intr64OpTy::getOperationName(), adaptor.getOperands(),
          op->getAttrs(), getTypeConverter(), rewriter);
    return rewriter.notifyMatchFailure(
        op, "expected 'src' to be either f32 or f64");
  }
};

struct MaskCompressOpConversion
    : public ConvertOpToLLVMPattern<MaskCompressOp> {
  using ConvertOpToLLVMPattern<MaskCompressOp>::ConvertOpToLLVMPattern;

  LogicalResult
  matchAndRewrite(MaskCompressOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto opType = adaptor.getA().getType();

    Value src;
    if (op.getSrc()) {
      src = adaptor.getSrc();
    } else if (op.getConstantSrc()) {
      src = rewriter.create<arith::ConstantOp>(op.getLoc(), opType,
                                               op.getConstantSrcAttr());
    } else {
      auto zeroAttr = rewriter.getZeroAttr(opType);
      src = rewriter.create<arith::ConstantOp>(op->getLoc(), opType, zeroAttr);
    }

    rewriter.replaceOpWithNewOp<MaskCompressIntrOp>(op, opType, adaptor.getA(),
                                                    src, adaptor.getK());

    return success();
  }
};

struct DotBF16OpConversion : public ConvertOpToLLVMPattern<DotBF16Op> {
  using ConvertOpToLLVMPattern<DotBF16Op>::ConvertOpToLLVMPattern;

  LogicalResult
  matchAndRewrite(DotBF16Op op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto typeA = dyn_cast<VectorType>(op.getA().getType());
    unsigned elemBitWidth = typeA.getElementTypeBitWidth();
    unsigned opBitWidth = typeA.getShape()[0] * elemBitWidth;

    auto opType = adaptor.getSrc().getType();
    auto opSrc = adaptor.getSrc();
    auto opA = adaptor.getA();
    auto opB = adaptor.getB();

    switch (opBitWidth) {
    case 128: {
      rewriter.replaceOpWithNewOp<DotBF16Ps128IntrOp>(op, opType, opSrc, opA,
                                                      opB);
      break;
    }
    case 256: {
      rewriter.replaceOpWithNewOp<DotBF16Ps256IntrOp>(op, opType, opSrc, opA,
                                                      opB);
      break;
    }
    case 512: {
      rewriter.replaceOpWithNewOp<DotBF16Ps512IntrOp>(op, opType, opSrc, opA,
                                                      opB);
      break;
    }
    default: {
      return rewriter.notifyMatchFailure(op,
                                         "unsupported AVX512-BF16 dot variant");
    }
    }

    return success();
  }
};

struct CvtPackedF32ToBF16Conversion
    : public ConvertOpToLLVMPattern<CvtPackedF32ToBF16Op> {
  using ConvertOpToLLVMPattern<CvtPackedF32ToBF16Op>::ConvertOpToLLVMPattern;

  LogicalResult
  matchAndRewrite(CvtPackedF32ToBF16Op op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto typeA = dyn_cast<VectorType>(op.getA().getType());
    unsigned elemBitWidth = typeA.getElementTypeBitWidth();
    unsigned opBitWidth = typeA.getShape()[0] * elemBitWidth;

    auto opType = op.getDst().getType();
    auto opA = op.getA();

    switch (opBitWidth) {
    case 256: {
      rewriter.replaceOpWithNewOp<CvtNeF32ToBF16Ps256IntrOp>(op, opType, opA);
      break;
    }
    case 512: {
      rewriter.replaceOpWithNewOp<CvtNeF32ToBF16Ps512IntrOp>(op, opType, opA);
      break;
    }
    default: {
      return rewriter.notifyMatchFailure(
          op, "unsupported AVX512-BF16 packed f32 to bf16 variant");
    }
    }

    return success();
  }
};

struct RsqrtOpConversion : public ConvertOpToLLVMPattern<RsqrtOp> {
  using ConvertOpToLLVMPattern<RsqrtOp>::ConvertOpToLLVMPattern;

  LogicalResult
  matchAndRewrite(RsqrtOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto opType = adaptor.getA().getType();
    rewriter.replaceOpWithNewOp<RsqrtIntrOp>(op, opType, adaptor.getA());
    return success();
  }
};

struct DotOpConversion : public ConvertOpToLLVMPattern<DotOp> {
  using ConvertOpToLLVMPattern<DotOp>::ConvertOpToLLVMPattern;

  LogicalResult
  matchAndRewrite(DotOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    auto opType = adaptor.getA().getType();
    Type llvmIntType = IntegerType::get(&getTypeConverter()->getContext(), 8);
    // Dot product of all elements, broadcasted to all elements.
    auto attr = rewriter.getI8IntegerAttr(static_cast<int8_t>(0xff));
    Value scale =
        rewriter.create<LLVM::ConstantOp>(op.getLoc(), llvmIntType, attr);
    rewriter.replaceOpWithNewOp<DotIntrOp>(op, opType, adaptor.getA(),
                                           adaptor.getB(), scale);
    return success();
  }
};

/// An entry associating the "main" AVX512 op with its instantiations for
/// vectors of 32-bit and 64-bit elements.
template <typename OpTy, typename Intr32OpTy, typename Intr64OpTy>
struct RegEntry {
  using MainOp = OpTy;
  using Intr32Op = Intr32OpTy;
  using Intr64Op = Intr64OpTy;
};

/// A container for op association entries facilitating the configuration of
/// dialect conversion.
template <typename... Args>
struct RegistryImpl {
  /// Registers the patterns specializing the "main" op to one of the
  /// "intrinsic" ops depending on elemental type.
  static void registerPatterns(const LLVMTypeConverter &converter,
                               RewritePatternSet &patterns) {
    patterns
        .add<LowerToIntrinsic<typename Args::MainOp, typename Args::Intr32Op,
                              typename Args::Intr64Op>...>(converter);
  }

  /// Configures the conversion target to lower out "main" ops.
  static void configureTarget(LLVMConversionTarget &target) {
    target.addIllegalOp<typename Args::MainOp...>();
    target.addLegalOp<typename Args::Intr32Op...>();
    target.addLegalOp<typename Args::Intr64Op...>();
  }
};

using Registry = RegistryImpl<
    RegEntry<MaskRndScaleOp, MaskRndScalePSIntrOp, MaskRndScalePDIntrOp>,
    RegEntry<MaskScaleFOp, MaskScaleFPSIntrOp, MaskScaleFPDIntrOp>,
    RegEntry<Vp2IntersectOp, Vp2IntersectDIntrOp, Vp2IntersectQIntrOp>>;

} // namespace

/// Populate the given list with patterns that convert from X86Vector to LLVM.
void mlir::populateX86VectorLegalizeForLLVMExportPatterns(
    const LLVMTypeConverter &converter, RewritePatternSet &patterns) {
  Registry::registerPatterns(converter, patterns);
  patterns
      .add<MaskCompressOpConversion, DotBF16OpConversion,
           CvtPackedF32ToBF16Conversion, RsqrtOpConversion, DotOpConversion>(
          converter);
}

void mlir::configureX86VectorLegalizeForExportTarget(
    LLVMConversionTarget &target) {
  Registry::configureTarget(target);
  target.addLegalOp<MaskCompressIntrOp>();
  target.addIllegalOp<MaskCompressOp>();
  target.addLegalOp<DotBF16Ps128IntrOp>();
  target.addLegalOp<DotBF16Ps256IntrOp>();
  target.addLegalOp<DotBF16Ps512IntrOp>();
  target.addIllegalOp<DotBF16Op>();
  target.addLegalOp<CvtNeF32ToBF16Ps256IntrOp>();
  target.addLegalOp<CvtNeF32ToBF16Ps512IntrOp>();
  target.addIllegalOp<CvtPackedF32ToBF16Op>();
  target.addLegalOp<RsqrtIntrOp>();
  target.addIllegalOp<RsqrtOp>();
  target.addLegalOp<DotIntrOp>();
  target.addIllegalOp<DotOp>();
}
