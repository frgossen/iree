// RUN: iree-opt -split-input-file -pass-pipeline='iree-hal-transformation-pipeline{serialize-executables=false},canonicalize' -iree-hal-target-backends=vmvx %s | IreeFileCheck %s

flow.executable @simpleMath_ex_dispatch_0 {
  flow.dispatch.entry @simpleMath_rgn_dispatch_0 attributes {
    workload = 4 : index
  }
  module {
    func @simpleMath_rgn_dispatch_0(%arg0: tensor<4xf32>) -> tensor<4xf32> {
      // DO NOT SUBMIT
      %0 = mhlo.add %arg0, %arg0 : tensor<4xf32>
      return %0 : tensor<4xf32>
    }
  }
}

// CHECK-LABEL: hal.executable @simpleMath_ex_dispatch_0
//  CHECK-NEXT:   hal.interface @legacy_io {
//  CHECK-NEXT:     hal.interface.binding @arg0, set=0, binding=0, type="StorageBuffer", access="Read"
//  CHECK-NEXT:     hal.interface.binding @ret0, set=0, binding=1, type="StorageBuffer", access="Write|Discard"
//  CHECK-NEXT:   }
//  CHECK-NEXT:   hal.executable.target @vmvx, filter="vmvx" {
//  CHECK-NEXT:     hal.executable.entry_point @simpleMath_rgn_dispatch_0 attributes {interface = @legacy_io, ordinal = 0 : index, signature = (tensor<4xf32>) -> tensor<4xf32>}
//  CHECK-NEXT:     module {
//  CHECK-NEXT:       vm.module @module {
//  CHECK-DAG:         vm.import @vmvx.interface.binding(%interface : !vm.ref<!vmvx.interface>, %set : i32, %binding : i32) -> !vm.ref<!vmvx.buffer>
//  CHECK-DAG:         vm.import @vmvx.buffer.alloc(%byte_length : i32) -> !vm.ref<!vmvx.buffer>
//  CHECK-DAG:         vm.import @vmvx.buffer.view(%src : !vm.ref<!vmvx.buffer>, %byte_offset : i32, %byte_length : i32) -> !vm.ref<!vmvx.buffer>
//  CHECK-DAG:         vm.import @vmvx.buffer.copy(%src : !vm.ref<!vmvx.buffer>, %src_byte_offset : i32, %dst : !vm.ref<!vmvx.buffer>, %dst_byte_offset : i32, %byte_length : i32)
//  CHECK-DAG:         vm.import @vmvx.add.f32(%lhs : !vm.ref<!vmvx.buffer>, %rhs : !vm.ref<!vmvx.buffer>, %dst : !vm.ref<!vmvx.buffer>)
//  CHECK:         vm.func @simpleMath_rgn_dispatch_0(%arg0: !vm.ref<!vmvx.interface>, %arg1: !vm.ref<!vmvx.buffer>, %arg2: i32, %arg3: i32, %arg4: i32) {
//  DO NOT SUBMIT
//  CHECK-NEXT:           vm.return
//  CHECK-NEXT:         }
//  CHECK-NEXT:         vm.export @simpleMath_rgn_dispatch_0
