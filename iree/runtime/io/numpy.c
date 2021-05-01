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

#include "iree/runtime/io/numpy.h"

//===----------------------------------------------------------------------===//
// Utilities
//===----------------------------------------------------------------------===//

// TODO(benvanik): dtype mapping to HAL types.
// TODO(benvanik): read/write headers.

//===----------------------------------------------------------------------===//
// .npy (single value)
//===----------------------------------------------------------------------===//

IREE_API_EXPORT iree_status_t iree_runtime_io_numpy_load_ndarray_from_file(
    iree_runtime_io_numpy_load_options_t options, const char* file_path,
    iree_hal_allocator_t* device_allocator,
    iree_hal_buffer_view_t** out_buffer_view) {
  return iree_make_status(IREE_STATUS_UNIMPLEMENTED, "TODO: numpy IO");
}

IREE_API_EXPORT iree_status_t iree_runtime_io_numpy_save_ndarray_to_file(
    iree_runtime_io_numpy_save_options_t options,
    iree_hal_buffer_view_t* buffer_view, const char* file_path) {
  return iree_make_status(IREE_STATUS_UNIMPLEMENTED, "TODO: numpy IO");
}

//===----------------------------------------------------------------------===//
// .npz (multiple values, possibly compressed)
//===----------------------------------------------------------------------===//

IREE_API_EXPORT iree_status_t iree_runtime_io_numpy_load_ndarrays_from_file(
    iree_runtime_io_numpy_load_options_t options, const char* file_path,
    iree_hal_allocator_t* device_allocator, iree_vm_list_t** out_list) {
  return iree_make_status(IREE_STATUS_UNIMPLEMENTED, "TODO: numpy IO");
}

IREE_API_EXPORT iree_status_t iree_runtime_io_numpy_save_ndarrays_to_file(
    iree_runtime_io_numpy_save_options_t options, iree_vm_list_t* list,
    const char* file_path) {
  if (iree_all_bits_set(options, IREE_RUNTIME_IO_NUMPY_SAVE_OPTION_COMPRESS)) {
    return iree_make_status(IREE_STATUS_UNIMPLEMENTED,
                            "npz compression not yet implemented");
  }
  return iree_make_status(IREE_STATUS_UNIMPLEMENTED, "TODO: numpy IO");
}
