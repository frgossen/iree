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

#ifndef IREE_MODULES_VMVX_BUFFER_H_
#define IREE_MODULES_VMVX_BUFFER_H_

#include "iree/base/api.h"
#include "iree/vm/api.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

typedef uint32_t iree_vmvx_size_t;

// Defines the access permissions on a buffer.
// Note that this is only used to verify guest access and may be a subset of the
// actual allowed host access to the memory; for example, we may pass in buffers
// allocated in writeable memory but only allow the guest to read them. Our
// guarantee here in VMVX is that the guest cannot access memory outside of
// these buffers and that it must observe the defined access.
typedef enum {
  // Default no access allowed value.
  IREE_VMVX_BUFFER_ACCESS_NONE = 0,
  // Guest is allowed to read from the buffer.
  IREE_VMVX_BUFFER_ACCESS_READ = 1u << 0,
  // Guest is allowed to write to the buffer.
  // If multiple guest invocations are running concurrently then behavior is
  // platform-dependent (don't cross the streams!).
  IREE_VMVX_BUFFER_ACCESS_WRITE = 1u << 1,
  // Optionally paired with IREE_VMVX_BUFFER_ACCESS_WRITE to indicate that the
  // existing contents of the memory are not to be read and may contain
  // undefined contents. The guest is required to populate all bytes within the
  // buffer range. Compatible with IREE_VMVX_BUFFER_ACCESS_READ but if the
  // guest reads before writing then behavior is undefined.
  IREE_VMVX_BUFFER_ACCESS_DISCARD = 1u << 2,

  IREE_VMVX_BUFFER_ACCESS_VALID_BITMASK = 0x7,
} iree_vmvx_buffer_access_t;

// Returns a string literal representing the given access bitfield.
const char* iree_vmvx_access_string(iree_vmvx_buffer_access_t access);

// External buffer reference.
// This is an on-stack wrapper around a pointer that can be round-tripped
// through VM IR. By populating this once and just passing around its reference
// we avoid the need to perform lookups on each access and can trust these
// values as they cannot be modified by the guest program.
typedef struct {
  // NOTE: this is allocated on the stack and never freed.
  iree_vm_ref_object_t ref_object;

  // Base buffer pointer.
  // Alignment is up to the buffer constraints specified during compilation.
  uint8_t* ptr;

  // Total length in bytes of the valid data starting at |ptr|.
  iree_vmvx_size_t length;

  // Access rights of the buffer memory.
  iree_vmvx_buffer_access_t access;
} iree_vmvx_buffer_t;

// No-op but here for convenience.
void iree_vmvx_buffer_destroy(iree_vmvx_buffer_t* buffer);

// Verifies that a requested access of a range of a buffer is valid.
iree_status_t iree_vmvx_buffer_verify_access(
    const char* buffer_name, const iree_vmvx_buffer_t* buffer,
    iree_vmvx_buffer_access_t requested_access,
    iree_vmvx_size_t requested_offset, iree_vmvx_size_t requested_length);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // IREE_MODULES_VMVX_BUFFER_H_
