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

#include "iree/runtime/call.h"

#include "iree/runtime/session.h"

IREE_API_EXPORT iree_status_t iree_runtime_call_initialize(
    iree_runtime_session_t* session, iree_vm_function_t function,
    iree_runtime_call_t* out_call) {
  IREE_ASSERT_ARGUMENT(session);
  IREE_ASSERT_ARGUMENT(out_call);
  memset(out_call, 0, sizeof(*out_call));

  // Query the signature of the function to determine the sizes of the lists.
  iree_vm_function_signature_t signature =
      iree_vm_function_signature(&function);
  iree_string_view_t arguments;
  iree_string_view_t results;
  IREE_RETURN_IF_ERROR(iree_vm_function_call_get_cconv_fragments(
      &signature, &arguments, &results));

  out_call->session = session;
  iree_runtime_session_retain(session);
  out_call->function = function;

  // Allocate the input and output lists with the required capacity.
  // A user wanting to avoid dynamic allocations could instead create on-stack
  // storage for these and use iree_vm_list_initialize instead. This high-level
  // API keeps things simple, though, and for the frequency of calls through
  // this interface a few small pooled malloc calls should be fine.
  iree_allocator_t host_allocator =
      iree_runtime_session_host_allocator(session);
  iree_status_t status = iree_vm_list_create(
      /*element_type=*/NULL, arguments.size, host_allocator, &out_call->inputs);
  if (iree_status_is_ok(status)) {
    status = iree_vm_list_create(
        /*element_type=*/NULL, results.size, host_allocator,
        &out_call->outputs);
  }

  if (!iree_status_is_ok(status)) {
    iree_runtime_call_deinitialize(out_call);
  }
  return status;
}

IREE_API_EXPORT iree_status_t iree_runtime_call_initialize_by_name(
    iree_runtime_session_t* session, iree_string_view_t full_name,
    iree_runtime_call_t* out_call) {
  iree_vm_function_t function;
  IREE_RETURN_IF_ERROR(
      iree_runtime_session_lookup_function(session, full_name, &function));
  return iree_runtime_call_initialize(session, function, out_call);
}

IREE_API_EXPORT void iree_runtime_call_deinitialize(iree_runtime_call_t* call) {
  IREE_ASSERT_ARGUMENT(call);
  iree_vm_list_release(call->inputs);
  iree_vm_list_release(call->outputs);
  iree_runtime_session_release(call->session);
}

IREE_API_EXPORT void iree_runtime_call_reset(iree_runtime_call_t* call) {
  IREE_ASSERT_ARGUMENT(call);
  iree_status_ignore(iree_vm_list_resize(call->inputs, 0));
  iree_status_ignore(iree_vm_list_resize(call->outputs, 0));
}

IREE_API_EXPORT iree_vm_list_t* iree_runtime_call_inputs(
    const iree_runtime_call_t* call) {
  IREE_ASSERT_ARGUMENT(call);
  return call->inputs;
}

IREE_API_EXPORT iree_vm_list_t* iree_runtime_call_outputs(
    const iree_runtime_call_t* call) {
  IREE_ASSERT_ARGUMENT(call);
  return call->outputs;
}

IREE_API_EXPORT iree_status_t iree_runtime_call_invoke(
    iree_runtime_call_t* call, iree_runtime_call_flags_t flags) {
  return iree_runtime_session_call(call->session, &call->function, call->inputs,
                                   call->outputs);
}
