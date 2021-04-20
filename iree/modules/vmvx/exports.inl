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
//
//         ██     ██  █████  ██████  ███    ██ ██ ███    ██  ██████
//         ██     ██ ██   ██ ██   ██ ████   ██ ██ ████   ██ ██
//         ██  █  ██ ███████ ██████  ██ ██  ██ ██ ██ ██  ██ ██   ███
//         ██ ███ ██ ██   ██ ██   ██ ██  ██ ██ ██ ██  ██ ██ ██    ██
//          ███ ███  ██   ██ ██   ██ ██   ████ ██ ██   ████  ██████
//
//===----------------------------------------------------------------------===//
//
// This file matches the vmvx.imports.mlir in the compiler. It'd be nice to
// autogenerate this as the order of these functions must be sorted ascending by
// name in a way compatible with iree_string_view_compare.
//
// Users are meant to `#define EXPORT_FN` to be able to access the information.
// #define EXPORT_FN(name, arg_type, ret_type, target_fn)

// clang-format off

EXPORT_FN("buffer.copy", iree_vmvx_module_buffer_copy, ririi, v)
EXPORT_FN("buffer.load.1xf32", iree_vmvx_module_buffer_load_1xf32, ri, f)
EXPORT_FN("buffer.load.1xi32", iree_vmvx_module_buffer_load_1xi32, ri, i)
EXPORT_FN("buffer.store.1xf32", iree_vmvx_module_buffer_store_1xf32, rif, v)
EXPORT_FN("buffer.store.1xi32", iree_vmvx_module_buffer_store_1xi32, rii, v)

EXPORT_FN("interface.binding", iree_vmvx_module_interface_binding, ri, r)
EXPORT_FN("interface.constant", iree_vmvx_module_interface_constant, ri, i)

// clang-format on
