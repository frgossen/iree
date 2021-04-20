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

#ifndef IREE_MODULES_VMVX_INTERFACE_H_
#define IREE_MODULES_VMVX_INTERFACE_H_

#include "iree/base/api.h"
#include "iree/modules/vmvx/buffer.h"
#include "iree/vm/api.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

// Read-only interface shared across all dispatch workgroup invocations.
// Though this is a ref object it is expected to be allocated on the stack or
// from an arena and must be manually deinitialized.
typedef struct {
  iree_vm_ref_object_t ref_object;

  // Total number of available 4 byte push constant values in |push_constants|.
  uint16_t push_constant_count;

  // Total number of binding base pointers in |binding_ptrs| and
  // |binding_lengths|. The set is packed densely based on which bindings are
  // used (known at compile-time).
  uint16_t binding_count;

  // |push_constant_count| values.
  const uint32_t* push_constants;

  // |binding_count| populated bindings defining what buffers are available to
  // the guest program.
  iree_vmvx_buffer_t* bindings;

  // A range of mutable bytes that can be used for transient allocations.
  // The exact usage of this memory is left up to the user. Contents are
  // undefined on entry and may contain data from previous guest invocations
  // (but _only_ that class of data - never anything that has not already run
  // through this code). The size of the memory must be at least that as
  // declared by the guest code as a requirement but may exceed it if more is
  // available.
  //
  // This is modeled as a buffer so that we can allow functions that act on
  // buffer references to also work with the scratch memory (vs requiring one
  // function using scratch and one using buffer).
  iree_vmvx_buffer_t scratch_buffer;
} iree_vmvx_interface_t;

// Initializes |out_interface| to its initial state and binds the
// |scratch_memory| range. Callers are expected to populate push constant and
// binding information.
void iree_vmvx_interface_initialize(iree_byte_span_t scratch_memory,
                                    iree_vmvx_interface_t* out_interface);

// Deinitializes |interface|.
void iree_vmvx_interface_deinitialize(iree_vmvx_interface_t* interface);

// No-op but here for convienence.
void iree_vmvx_interface_destroy(iree_vmvx_interface_t* interface);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // IREE_MODULES_VMVX_INTERFACE_H_
