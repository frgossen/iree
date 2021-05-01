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

#include "iree/runtime/api.h"
#include "iree/runtime/testdata/mul_vmla_module_c.h"

// Loads the bytecode module directly from memory.
// Embedding the compiled output into your binary is not always possible (or
// recommended) but is a fairly painless way to get things working on a variety
// of targets without worrying about how to deploy files or pass flags.
//
// In cases like this the module file is in .rodata and does not need to be
// freed; if the memory needs to be released when the module is unloaded then a
// custom allocator can be provided to get a callback instead.
iree_status_t iree_runtime_demo_load_module(iree_runtime_session_t* session) {
  const iree_file_toc_t* module_file = mul_vmla_module_c_create();
  return iree_runtime_session_append_bytecode_module_from_memory(
      session, iree_make_const_byte_span(module_file->data, module_file->size),
      iree_allocator_null());
}

// See session_demo.c for the main demo.
int iree_runtime_demo_main();

int main(int argc, char** argv) { return iree_runtime_demo_main(); }
