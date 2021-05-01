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

#include <stdio.h>

#include "iree/runtime/api.h"

// Loads a compiled IREE module from the file system.
static const char* demo_file_path = NULL;
iree_status_t iree_runtime_demo_load_module(iree_runtime_session_t* session) {
  return iree_runtime_session_append_bytecode_module_from_file(session,
                                                               demo_file_path);
}

// See session_demo.c for the main demo.
int iree_runtime_demo_main();

// Takes the first argument on the command line as a file path and loads it.
int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: session_demo module_file.vmfb\n");
    return 1;
  }
  demo_file_path = argv[1];
  return iree_runtime_demo_main();
}
