// Copyright 2021 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "iree/compiler/Dialect/Modules/VMVX/Conversion/StandardToVMVX/ConvertStandardToVMVX.h"

#include "iree/compiler/Dialect/IREE/IR/IREETypes.h"
#include "iree/compiler/Dialect/Modules/VMVX/IR/VMVXDialect.h"
#include "iree/compiler/Dialect/Modules/VMVX/IR/VMVXOps.h"
#include "iree/compiler/Dialect/Modules/VMVX/IR/VMVXTypes.h"
#include "iree/compiler/Dialect/Shape/IR/Builders.h"
#include "iree/compiler/Dialect/Shape/IR/ShapeOps.h"
#include "iree/compiler/Dialect/Shape/IR/ShapeTypes.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Linalg/IR/LinalgOps.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/IR/AffineExpr.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Matchers.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
namespace iree_compiler {

namespace {

// Returns the number of bytes an element of the given type occupies
// post-conversion. For example, the size of i1 would be '1 byte'.
static int32_t getRoundedElementByteWidth(Type type) {
  return (type.getIntOrFloatBitWidth() + 8 - 1) / 8;
}

// Returns the offset, in bytes, of an index within a linearized dense buffer.
// Expects that the |memrefValue| has been linearized already.
static Value getBufferOffset(Location loc, Value memrefValue,
                             ValueRange indices,
                             ConversionPatternRewriter &rewriter) {
  auto memrefType = memrefValue.getType().cast<ShapedType>();
  if (memrefType.getRank() == 0) {
    // Rank 0 buffers (like memref<i32>) have only a single valid offset at 0.
    return rewriter.createOrFold<ConstantIndexOp>(loc, 0);
  }
  assert(memrefType.getRank() == 1 && "memrefs should have been flattened");

  // Element type byte length as the base.
  auto elementType = memrefType.getElementType();
  auto scalingExpr = getAffineBinaryOpExpr(
      AffineExprKind::Mul, getAffineSymbolExpr(0, rewriter.getContext()),
      getAffineConstantExpr(getRoundedElementByteWidth(elementType),
                            rewriter.getContext()));

  // Rank 1 memrefs are just offset by their element width by the offset.
  return rewriter.createOrFold<AffineApplyOp>(loc, scalingExpr,
                                              ArrayRef<Value>{indices.front()});
}

class ConvertMemRefLoadOp : public OpConversionPattern<memref::LoadOp> {
 public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      memref::LoadOp op, ArrayRef<Value> rawOperands,
      ConversionPatternRewriter &rewriter) const override {
    memref::LoadOpAdaptor operands(rawOperands);
    auto offset =
        getBufferOffset(op.getLoc(), op.memref(), operands.indices(), rewriter);
    rewriter.replaceOpWithNewOp<IREE::VMVX::BufferLoadOp>(
        op, getTypeConverter()->convertType(op.result().getType()),
        operands.memref(), offset);
    return success();
  }
};

class ConvertMemRefStoreOp : public OpConversionPattern<memref::StoreOp> {
 public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      memref::StoreOp op, ArrayRef<Value> rawOperands,
      ConversionPatternRewriter &rewriter) const override {
    memref::StoreOpAdaptor operands(rawOperands);
    auto offset =
        getBufferOffset(op.getLoc(), op.memref(), operands.indices(), rewriter);
    rewriter.replaceOpWithNewOp<IREE::VMVX::BufferStoreOp>(
        op, operands.memref(), offset, operands.value());
    return success();
  }
};

/// Pattern to lower operations that become a no-ops at this level.
template <typename OpTy>
struct FoldAsNoOp final : public OpConversionPattern<OpTy> {
  using OpConversionPattern<OpTy>::OpConversionPattern;
  LogicalResult matchAndRewrite(
      OpTy op, ArrayRef<Value> operands,
      ConversionPatternRewriter &rewriter) const override {
    rewriter.replaceOp(op, operands);
    return success();
  }
};

/// Removes unrealized_conversion_cast ops introduced during progressive
/// lowering when possible.
struct RemoveIdentityConversionCast final
    : public OpConversionPattern<UnrealizedConversionCastOp> {
  using OpConversionPattern::OpConversionPattern;
  LogicalResult matchAndRewrite(
      UnrealizedConversionCastOp op, ArrayRef<Value> operands,
      ConversionPatternRewriter &rewriter) const override {
    if (op->getNumOperands() == 1 && op->getNumResults() == 1 &&
        operands.front().getType() == op->getResultTypes().front()) {
      rewriter.replaceOp(op, operands);
      return success();
    }

    return failure();
  }
};

}  // namespace

void populateStandardToVMVXPatterns(MLIRContext *context,
                                    OwningRewritePatternList &patterns,
                                    TypeConverter &typeConverter) {
  patterns.insert<ConvertMemRefLoadOp>(typeConverter, context);
  patterns.insert<ConvertMemRefStoreOp>(typeConverter, context);

  patterns
      .insert<FoldAsNoOp<linalg::ReshapeOp>, FoldAsNoOp<memref::BufferCastOp>>(
          typeConverter, context);
  patterns.insert<RemoveIdentityConversionCast>(typeConverter, context);
}

}  // namespace iree_compiler
}  // namespace mlir
