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

#include "iree/modules/vmvx/interface.h"

void iree_vmvx_interface_initialize(iree_byte_span_t scratch_memory,
                                    iree_vmvx_interface_t* out_interface) {
  memset(out_interface, 0, sizeof(*out_interface));
  iree_atomic_ref_count_init(&out_interface->ref_object.counter);

  iree_vmvx_buffer_t* scratch_buffer = &out_interface->scratch_buffer;
  iree_atomic_ref_count_init(&scratch_buffer->ref_object.counter);
  scratch_buffer->ptr = scratch_memory.data;
  scratch_buffer->length = scratch_memory.data_length;
  scratch_buffer->access = IREE_VMVX_BUFFER_ACCESS_READ |
                           IREE_VMVX_BUFFER_ACCESS_WRITE |
                           IREE_VMVX_BUFFER_ACCESS_DISCARD;
}

void iree_vmvx_interface_deinitialize(iree_vmvx_interface_t* interface) {
  // NOTE: we could assert that the ref count is 0 here on all bindings and
  // the scratch memory.
}

void iree_vmvx_interface_destroy(iree_vmvx_interface_t* interface) {
  // No-op; stack allocated or embedded in another struct.
}
