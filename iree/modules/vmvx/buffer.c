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

#include "iree/modules/vmvx/buffer.h"

const char* iree_vmvx_access_string(iree_vmvx_buffer_access_t access) {
  static const char* literals[] = {
      /*0x00=*/"none",  // NONE
      /*0x01=*/"R",     // READ
      /*0x02=*/"W",     // WRITE
      /*0x03=*/"RW",    // READ+WRITE
      /*0x04=*/"X",     // DISCARD
      /*0x05=*/"XR",    // DISCARD+READ (invalid but requires coverage here)
      /*0x06=*/"XW",    // DISCARD+WRITE
      /*0x07=*/"XRW",   // DISCARD+READ+WRITE
  };
  return literals[access & IREE_VMVX_BUFFER_ACCESS_VALID_BITMASK];
}

void iree_vmvx_buffer_destroy(iree_vmvx_buffer_t* buffer) {
  // No-op; stack allocated or embedded in another struct.
}

iree_status_t iree_vmvx_buffer_verify_access(
    const char* buffer_name, const iree_vmvx_buffer_t* buffer,
    iree_vmvx_buffer_access_t requested_access,
    iree_vmvx_size_t requested_offset, iree_vmvx_size_t requested_length) {
  if (IREE_UNLIKELY(!iree_all_bits_set(buffer->access, requested_access))) {
    return iree_make_status(
        IREE_STATUS_PERMISSION_DENIED,
        "attempted to access %s buffer with access %s as %s", buffer_name,
        iree_vmvx_access_string(buffer->access),
        iree_vmvx_access_string(requested_access));
  }
  size_t end = requested_offset + requested_length - 1;
  if (IREE_UNLIKELY(end >= buffer->length)) {
    return iree_make_status(IREE_STATUS_OUT_OF_RANGE,
                            "attempted to access %s buffer of length %u out "
                            "of range: [%u, %u] (%ub)",
                            buffer_name, (uint32_t)buffer->length,
                            (uint32_t)requested_offset, (uint32_t)end,
                            (uint32_t)requested_length);
  }
  return iree_ok_status();
}
