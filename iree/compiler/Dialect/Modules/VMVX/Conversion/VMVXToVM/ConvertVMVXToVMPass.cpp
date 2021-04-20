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

#include "iree/compiler/Dialect/IREE/IR/IREEDialect.h"
#include "iree/compiler/Dialect/IREE/IR/IREETypes.h"
#include "iree/compiler/Dialect/Modules/VMVX/Conversion/VMVXToVM/ConvertVMVXToVM.h"
#include "iree/compiler/Dialect/Modules/VMVX/IR/VMVXDialect.h"
#include "iree/compiler/Dialect/Modules/VMVX/IR/VMVXOps.h"
#include "iree/compiler/Dialect/Modules/VMVX/IR/VMVXTypes.h"
#include "iree/compiler/Dialect/Modules/VMVX/vmvx.imports.h"
#include "iree/compiler/Dialect/VM/Conversion/ConversionTarget.h"
#include "iree/compiler/Dialect/VM/Conversion/ImportUtils.h"
#include "iree/compiler/Dialect/VM/Conversion/StandardToVM/ConvertStandardToVM.h"
#include "iree/compiler/Dialect/VM/Conversion/TypeConverter.h"
#include "iree/compiler/Dialect/VM/IR/VMOps.h"
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
namespace {

// A pass converting the IREE VMVX dialect into the IREE VM dialect.
class ConvertVMVXToVMPass
    : public PassWrapper<ConvertVMVXToVMPass, OperationPass<ModuleOp>> {
 public:
  explicit ConvertVMVXToVMPass(IREE::VM::TargetOptions targetOptions)
      : targetOptions_(targetOptions) {}

  void getDependentDialects(DialectRegistry &registry) const override {
    registry
        .insert<IREEDialect, IREE::VM::VMDialect, IREE::VMVX::VMVXDialect>();
  }

  void runOnOperation() override {
    auto *context = &getContext();

    VMConversionTarget conversionTarget(context);
    IREE::VM::TypeConverter typeConverter(targetOptions_);

    mlir::ModuleOp outerModuleOp, innerModuleOp;
    std::tie(outerModuleOp, innerModuleOp) =
        VMConversionTarget::nestModuleForConversion(getOperation());

    (void)appendImportModule(
        StringRef(vmvx_imports_create()->data, vmvx_imports_create()->size),
        innerModuleOp);

    OwningRewritePatternList conversionPatterns(&getContext());
    populateStandardToVMPatterns(context, typeConverter, conversionPatterns);

    SymbolTable importSymbols(innerModuleOp);
    populateVMVXToVMPatterns(context, typeConverter, importSymbols,
                             conversionPatterns);

    if (failed(applyPartialConversion(outerModuleOp, conversionTarget,
                                      std::move(conversionPatterns)))) {
      outerModuleOp.emitError() << "conversion to vm.module failed";
      return signalPassFailure();
    }
  }

 private:
  IREE::VM::TargetOptions targetOptions_;
};

}  // namespace

std::unique_ptr<OperationPass<ModuleOp>> createConvertVMVXToVMPass(
    IREE::VM::TargetOptions targetOptions) {
  return std::make_unique<ConvertVMVXToVMPass>(targetOptions);
}

static PassRegistration<ConvertVMVXToVMPass> pass(
    "iree-convert-vmvx-to-vm",
    "Convert the IREE VMVX dialect to the IREE VM dialect", [] {
      auto options = IREE::VM::getTargetOptionsFromFlags();
      return std::make_unique<ConvertVMVXToVMPass>(options);
    });

}  // namespace iree_compiler
}  // namespace mlir
