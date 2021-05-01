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

#include "iree/runtime/status_util.h"

IREE_API_EXPORT void iree_status_fprint(FILE* file, iree_status_t status) {
  // TODO(benvanik): move to iree/base/ with better support for colors/etc.
  // TODO(benvanik): do this without allocation by streaming the status.
  char* status_buffer = NULL;
  iree_host_size_t status_buffer_length = 0;
  iree_status_to_string(status, &status_buffer, &status_buffer_length);
  fprintf(file, "%.*s\n", (int)status_buffer_length, status_buffer);
  free(status_buffer);
  fflush(file);
}
