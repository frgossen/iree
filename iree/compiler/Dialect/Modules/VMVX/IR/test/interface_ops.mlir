// RUN: iree-opt -split-input-file %s | iree-opt -split-input-file | IreeFileCheck %s

// DO NOT SUBMIT
// CHECK-LABEL: @vmvx_interface_const
// CHECK-SAME: %[[INTERFACE:[a-zA-Z0-9$._-]+]]
func @vmvx_interface_const(%interface : !vmvx.interface) {
  // CHECK:      vmvx.interface.const %[[INTERFACE]]
  // CHECK-SAME: {offset = 3 : index} : i32
  vmvx.interface.const %interface {offset = 3 : index} : i32
  return
}

// -----

// CHECK-LABEL: @vmvx_interface_binding
// CHECK-SAME: %[[INTERFACE:[a-zA-Z0-9$._-]+]]
func @vmvx_interface_binding(%interface : !vmvx.interface) {
  // CHECK:      vmvx.interface.binding %[[INTERFACE]]
  // CHECK-SAME: {binding = 0 : i32, set = 0 : i32} : !vmvx.buffer
  vmvx.interface.binding %interface
                         {binding = 0 : i32, set = 0 : i32} : !vmvx.buffer
  return
}
