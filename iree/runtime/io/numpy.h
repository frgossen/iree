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

//===----------------------------------------------------------------------===//
// numpy-compatible(ish) IO
//===----------------------------------------------------------------------===//
//
// Provides C variants of the numpy.load/numpy.save routines:
// https://numpy.org/doc/stable/reference/routines.io.html
// which uses the given file format:
// https://numpy.org/doc/stable/reference/generated/numpy.lib.format.html
//
// Pickled objects are not supported (similar to using `allow_pickle=False`) and
// not all dtypes are supported.
//
// .npy and uncompressed .npz files can be mapped into host memory with
// IREE_RUNTIME_IO_NUMPY_LOAD_OPTION_MAP_FILE if the HAL device allocator
// supports using such memory. On devices with discrete memory the contents will
// be loaded into host memory and copied to the device.
//
// This current implementation is very basic; in the future it'd be nice to
// support an iree_io_stream_t to allow for externalizing the file access.
//
// NOTE: this implementation is optimized for code size and is not intended to
// be used in performance-sensitive situations. If you are wanting to run
// through thousands of arrays and GB of data then you're going to want
// something more sophisticated (delay loading with async IO, etc).
//
// TODO(benvanik): conditionally enable compression when zlib is present. For
// now to reduce dependencies we don't support loading compressed npz files or
// compression on save.

#ifndef IREE_RUNTIME_IO_NUMPY_H_
#define IREE_RUNTIME_IO_NUMPY_H_

#include "iree/base/api.h"
#include "iree/hal/api.h"
#include "iree/vm/api.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

//===----------------------------------------------------------------------===//
// Options
//===----------------------------------------------------------------------===//

// Options controlling npy and npz load behavior.
enum iree_runtime_io_numpy_load_options_e {
  IREE_RUNTIME_IO_NUMPY_LOAD_OPTION_DEFAULT = 0,

  // Tries to map the file into memory and use the contents directly from the
  // file system. Only available if the HAL device supports accessing mapped
  // data and the file is uncompressed.
  // Like providing `mmap_mode` to `numpy.load`.
  IREE_RUNTIME_IO_NUMPY_LOAD_OPTION_MAP_FILE = 1u << 0,
};
typedef uint32_t iree_runtime_io_numpy_load_options_t;

// Options controlling npy and npz save behavior.
enum iree_runtime_io_numpy_save_options_e {
  IREE_RUNTIME_IO_NUMPY_SAVE_OPTION_DEFAULT = 0,

  // Compresses the npz contents while saving.
  // Switches from the default uncompressed behavior matching `numpy.savez` to
  // the compressed behavior of `numpy.savez_compressed`.
  IREE_RUNTIME_IO_NUMPY_SAVE_OPTION_COMPRESS = 1u << 0,
};
typedef uint32_t iree_runtime_io_numpy_save_options_t;

//===----------------------------------------------------------------------===//
// .npy (single value)
//===----------------------------------------------------------------------===//

// Loads a .npy from |file_path| into a buffer view.
// On success |out_buffer_view| will have buffer view matching the parameters
// in the npy file allocated from the given |device_allocator|.
//
// If IREE_RUNTIME_IO_NUMPY_LOAD_OPTION_MAP_FILE is set and the
// |device_allocator| supports mapping then the file will be mapped into the
// host process. Otherwise the file will be loaded into a new allocation.
//
// Fails if the contents of the npy cannot be mapped to IREE HAL types or is
// not an ndarray.
//
// See `numpy.load`:
// https://numpy.org/doc/stable/reference/generated/numpy.load.html
IREE_API_EXPORT iree_status_t iree_runtime_io_numpy_load_ndarray_from_file(
    iree_runtime_io_numpy_load_options_t options, const char* file_path,
    iree_hal_allocator_t* device_allocator,
    iree_hal_buffer_view_t** out_buffer_view);

// Saves |buffer_view| to a .npy file at |file_path|.
//
// See `numpy.save`:
// https://numpy.org/doc/stable/reference/generated/numpy.save.html
IREE_API_EXPORT iree_status_t iree_runtime_io_numpy_save_ndarray_to_file(
    iree_runtime_io_numpy_save_options_t options,
    iree_hal_buffer_view_t* buffer_view, const char* file_path);

//===----------------------------------------------------------------------===//
// .npz (multiple values, possibly compressed)
//===----------------------------------------------------------------------===//

// Loads the contents of the .npz file at |file_path| into a new |out_list|.
// |device_allocator| will be used to allocate any buffers required.
// Fails if the contents of the file cannot be mapped to IREE HAL or VM types.
//
// See `numpy.load`:
// https://numpy.org/doc/stable/reference/generated/numpy.load.html
IREE_API_EXPORT iree_status_t iree_runtime_io_numpy_load_ndarrays_from_file(
    iree_runtime_io_numpy_load_options_t options, const char* file_path,
    iree_hal_allocator_t* device_allocator, iree_vm_list_t** out_list);

// Saves the conents of the given |list| to a .npz file at |file_path|.
// Fails if any element of the list cannot be mapped to numpy types.
//
// See `numpy.savez` and `numpy.savez_compressed`:
// https://numpy.org/doc/stable/reference/generated/numpy.savez.html
// https://numpy.org/doc/stable/reference/generated/numpy.savez_compressed.html
IREE_API_EXPORT iree_status_t iree_runtime_io_numpy_save_ndarrays_to_file(
    iree_runtime_io_numpy_save_options_t options, iree_vm_list_t* list,
    const char* file_path);

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

#endif  // IREE_RUNTIME_IO_NUMPY_H_
