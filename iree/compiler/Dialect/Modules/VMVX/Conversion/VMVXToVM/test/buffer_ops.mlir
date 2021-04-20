// RUN: iree-opt -split-input-file -iree-convert-vmvx-to-vm -cse %s | IreeFileCheck %s

// DO NOT SUBMIT
// CHECK-LABEL: vm.func @bufferImport
func @bufferImport() -> !vmla.buffer {
  %c0 = std.constant 1 : index
  // CHECK: = vm.call @vmla.buffer.alloc(%c1) : (i32) -> !vm.ref<!vmla.buffer>
  %0 = vmla.buffer.alloc byte_length = %c0 : !vmla.buffer
  return %0 : !vmla.buffer
}
