// RUN: iree-opt -split-input-file %s | iree-opt -split-input-file | IreeFileCheck %s

// DO NOT SUBMIT
// CHECK-LABEL: vmla_buffer_const
// CHECK-SAME: %[[VALUE:[a-zA-Z0-9$._-]+]]
func @vmla_buffer_const(%value : !iree.byte_buffer) {
  // CHECK: vmla.buffer.const %[[VALUE]] : !iree.byte_buffer -> !vmla.buffer
  %result = vmla.buffer.const %value : !iree.byte_buffer -> !vmla.buffer
  return
}
