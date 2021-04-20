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

#include "iree/compiler/Dialect/Modules/VMVX/Conversion/HALToVMVX/ConvertHALToVMVX.h"

#include "iree/compiler/Dialect/HAL/IR/HALOps.h"
#include "iree/compiler/Dialect/IREE/IR/IREETypes.h"
#include "iree/compiler/Dialect/Modules/VMVX/IR/VMVXDialect.h"
#include "iree/compiler/Dialect/Modules/VMVX/IR/VMVXOps.h"
#include "iree/compiler/Dialect/Modules/VMVX/IR/VMVXTypes.h"
#include "mlir/Dialect/StandardOps/IR/Ops.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Matchers.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
namespace iree_compiler {

// Ordered indices of arguments to the entry point function.
// This is what the VM will receive at runtime from the HAL.
enum EntryArgOrdinals {
  kEntryArgInterface,
  kEntryArgScratchpad,
  kEntryArgWorkgroupX,
  kEntryArgWorkgroupY,
  kEntryArgWorkgroupZ,
  kEntryArgWorkgroupSizeX,
  kEntryArgWorkgroupSizeY,
  kEntryArgWorkgroupSizeZ,
  kEntryArgWorkgroupCountX,
  kEntryArgWorkgroupCountY,
  kEntryArgWorkgroupCountZ,
};

/// Rewrites entry functions to have a vmvx.interface, scratchpad, and an XYZ
/// workgroup ID. The runtime will provide these values during invocation.
///
/// Source:
///   func @entry()
///
/// Target:
///   func @entry(
///       %interface: !vmvx.interface,
///       %scratchpad: !vmvx.buffer,
///       %workgroup_x: index,
///       %workgroup_y: index,
///       %workgroup_z: index,
///       %workgroup_size_x: index,
///       %workgroup_size_y: index,
///       %workgroup_size_z: index,
///       %workgroup_count_x: index,
///       %workgroup_count_y: index,
///       %workgroup_count_z: index
///   )
LogicalResult updateHALToVMVXEntryFuncOp(FuncOp funcOp,
                                         TypeConverter &typeConverter) {
  auto originalType = funcOp.getType();
  if (originalType.getNumInputs() != 0 || originalType.getNumResults() != 0) {
    return funcOp.emitError() << "exported functions must have no I/O";
  }

  auto interfaceType = IREE::VMVX::InterfaceType::get(funcOp.getContext());
  auto bufferType = IREE::VMVX::BufferType::get(funcOp.getContext());
  auto indexType = IndexType::get(funcOp.getContext());
  auto newType = FunctionType::get(funcOp.getContext(),
                                   {
                                       /*interface=*/interfaceType,
                                       /*scratchpad=*/bufferType,
                                       /*workgroup_x=*/indexType,
                                       /*workgroup_y=*/indexType,
                                       /*workgroup_z=*/indexType,
                                       /*workgroup_size_x=*/indexType,
                                       /*workgroup_size_y=*/indexType,
                                       /*workgroup_size_z=*/indexType,
                                       /*workgroup_count_x=*/indexType,
                                       /*workgroup_count_y=*/indexType,
                                       /*workgroup_count_z=*/indexType,
                                   },
                                   {});

  funcOp.setType(newType);
  funcOp.front().addArguments(newType.getInputs());

  return success();
}

namespace {

/// Rewrites hal.interface.workgroup.id to use the arguments injected onto the
/// function.
class ConvertHALInterfaceWorkgroupIDOp
    : public OpConversionPattern<IREE::HAL::InterfaceWorkgroupIDOp> {
 public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      IREE::HAL::InterfaceWorkgroupIDOp op, ArrayRef<Value> operands,
      ConversionPatternRewriter &rewriter) const override {
    uint64_t dim = op.dimension().getZExtValue();
    if (dim >= 3) {
      return op.emitOpError() << "out of bounds workgroup ID dimension";
    }

    // Get the argument to the function corresponding to the workgroup dim.
    auto workgroupDim = op->getParentOfType<mlir::FuncOp>().getArgument(
        kEntryArgWorkgroupX + dim);
    rewriter.replaceOp(op, workgroupDim);
    return success();
  }
};

/// Rewrites hal.interface.workgroup.size to use the arguments injected onto the
/// function.
class ConvertHALInterfaceWorkgroupSizeOp
    : public OpConversionPattern<IREE::HAL::InterfaceWorkgroupSizeOp> {
 public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      IREE::HAL::InterfaceWorkgroupSizeOp op, ArrayRef<Value> operands,
      ConversionPatternRewriter &rewriter) const override {
    uint64_t dim = op.dimension().getZExtValue();
    if (dim >= 3) {
      return op.emitOpError() << "out of bounds workgroup size dimension";
    }

    // Get the argument to the function corresponding to the workgroup dim.
    auto workgroupDim = op->getParentOfType<mlir::FuncOp>().getArgument(
        kEntryArgWorkgroupSizeX + dim);
    rewriter.replaceOp(op, workgroupDim);
    return success();
  }
};

/// Rewrites hal.interface.workgroup.count to use the arguments injected onto
/// the function.
class ConvertHALInterfaceWorkgroupCountOp
    : public OpConversionPattern<IREE::HAL::InterfaceWorkgroupCountOp> {
 public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      IREE::HAL::InterfaceWorkgroupCountOp op, ArrayRef<Value> operands,
      ConversionPatternRewriter &rewriter) const override {
    uint64_t dim = op.dimension().getZExtValue();
    if (dim >= 3) {
      return op.emitOpError() << "out of bounds workgroup count dimension";
    }

    // Get the argument to the function corresponding to the workgroup dim.
    auto workgroupDim = op->getParentOfType<mlir::FuncOp>().getArgument(
        kEntryArgWorkgroupCountX + dim);
    rewriter.replaceOp(op, workgroupDim);
    return success();
  }
};

/// Rewrites hal.interface.load.constant to ops loading from the ABI structs.
class ConvertHALInterfaceLoadConstantOp
    : public OpConversionPattern<IREE::HAL::InterfaceLoadConstantOp> {
 public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      IREE::HAL::InterfaceLoadConstantOp op, ArrayRef<Value> operands,
      ConversionPatternRewriter &rewriter) const override {
    // Find the vmvx.interface argument to the function.
    auto interfaceArg =
        op->getParentOfType<mlir::FuncOp>().getArgument(kEntryArgInterface);
    assert(interfaceArg &&
           interfaceArg.getType().isa<IREE::VMVX::InterfaceType>() &&
           "entry point not conforming to requirements");

    rewriter.replaceOpWithNewOp<IREE::VMVX::InterfaceConstantOp>(
        op, op.result().getType(), interfaceArg, op.offset());
    return success();
  }
};

/// Rewrites hal.interface.binding.subspan to ops loading from the ABI structs.
class ConvertHALInterfaceBindingSubspanOp
    : public OpConversionPattern<IREE::HAL::InterfaceBindingSubspanOp> {
 public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      IREE::HAL::InterfaceBindingSubspanOp op, ArrayRef<Value> operands,
      ConversionPatternRewriter &rewriter) const override {
    // Find the vmvx.interface argument to the function.
    auto interfaceArg =
        op->getParentOfType<mlir::FuncOp>().getArgument(kEntryArgInterface);
    assert(interfaceArg &&
           interfaceArg.getType().isa<IREE::VMVX::InterfaceType>() &&
           "entry point not conforming to requirements");

    // Lookup the source interface binding.
    auto interfaceBindingOp = op.queryBindingOp();

    // TODO(benvanik): compact the indices - the bindings we have on the ABI
    // interface are dense.
    assert(interfaceBindingOp.set().getZExtValue() == 0 &&
           "sparse bindings not yet implemented");

    rewriter.replaceOpWithNewOp<IREE::VMVX::InterfaceBindingOp>(
        op, getTypeConverter()->convertType(op.result().getType()),
        interfaceArg, interfaceBindingOp.bindingAttr());
    return success();
  }
};

/// Removes the hal.interface from the IR - it's not used after conversion.
class RemoveHALInterfaceOpPattern
    : public OpConversionPattern<IREE::HAL::InterfaceOp> {
 public:
  using OpConversionPattern::OpConversionPattern;

  LogicalResult matchAndRewrite(
      IREE::HAL::InterfaceOp op, ArrayRef<Value> operands,
      ConversionPatternRewriter &rewriter) const override {
    rewriter.eraseOp(op);
    return success();
  }
};

}  // namespace

void populateHALToVMVXPatterns(MLIRContext *context,
                               OwningRewritePatternList &patterns,
                               TypeConverter &typeConverter) {
  patterns.insert<ConvertHALInterfaceWorkgroupIDOp>(typeConverter, context);
  patterns.insert<ConvertHALInterfaceWorkgroupSizeOp>(typeConverter, context);
  patterns.insert<ConvertHALInterfaceWorkgroupCountOp>(typeConverter, context);
  patterns.insert<ConvertHALInterfaceLoadConstantOp>(typeConverter, context);
  patterns.insert<ConvertHALInterfaceBindingSubspanOp>(typeConverter, context);
  patterns.insert<RemoveHALInterfaceOpPattern>(typeConverter, context);
}

}  // namespace iree_compiler
}  // namespace mlir
