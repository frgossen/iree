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

static iree_status_t iree_runtime_demo_run_session(
    iree_runtime_instance_t* instance);
static iree_status_t iree_runtime_demo_perform_mul(
    iree_runtime_session_t* session);

// Loads the user module into the session.
// See the iree/runtime/session_demo_* files for more information.
iree_status_t iree_runtime_demo_load_module(iree_runtime_session_t* session);

//===----------------------------------------------------------------------===//
// 1. Entry point / shared iree_runtime_instance_t setup
//===----------------------------------------------------------------------===//
// Applications should create and share a single instance across all sessions.

// This would live in your application startup/shutdown code or scoped to the
// usage of IREE. Creating and destroying instances is expensive and should be
// avoided.
int iree_runtime_demo_main() {
  IREE_TRACE_ZONE_BEGIN(z0);

  // Set up the shared runtime instance.
  // An application should usually only have one of these and share it across
  // all of the sessions it has. The instance is thread-safe, while the
  // sessions are only thread-compatible (you need to lock if its required).
  iree_runtime_instance_options_t instance_options;
  iree_runtime_instance_options_initialize(IREE_API_VERSION_LATEST,
                                           &instance_options);
  iree_runtime_instance_options_use_all_available_drivers(&instance_options);
  iree_runtime_instance_t* instance = NULL;
  iree_status_t status = iree_runtime_instance_create(
      &instance_options, iree_allocator_system(), &instance);

  // Run the demo.
  // A real application would load its models (at startup, on-demand, etc) and
  // retain them somewhere to be reused. Startup time and likelihood of failure
  // varies across different HAL backends; the synchronous CPU backend is nearly
  // instantaneous and will never fail (unless out of memory) while the Vulkan
  // backend may take significantly longer and fail if there are not supported
  // devices.
  if (iree_status_is_ok(status)) {
    status = iree_runtime_demo_run_session(instance);
  }

  // Release the shared instance - it will be deallocated when all sessions
  // using it have been released (here it is deallocated immediately).
  iree_runtime_instance_release(instance);

  if (iree_status_is_ok(status)) {
    // Successful!
    IREE_TRACE_ZONE_END(z0);
    return 0;
  }

  // Dump nice status messages to stderr on failure.
  // An application can route these through its own logging infrastructure as
  // needed. Note that the status is a handle and must be freed!
  int ret = (int)iree_status_code(status);
  iree_status_fprint(stderr, status);
  iree_status_ignore(status);
  IREE_TRACE_ZONE_END(z0);
  return ret;
}

//===----------------------------------------------------------------------===//
// 2. Load modules and initialize state in iree_runtime_session_t
//===----------------------------------------------------------------------===//
// Each instantiation of a module will live in its own session. Module state
// like variables will be retained across calls within the same session.

// Loads the demo module and uses it to perform some math.
// In a real application you'd want to hang on to the iree_runtime_session_t
// and reuse it for future calls - especially if it holds state internally.
static iree_status_t iree_runtime_demo_run_session(
    iree_runtime_instance_t* instance) {
  // TODO(#5724): move device selection into the compiled modules.
  iree_hal_device_t* device = NULL;
  IREE_RETURN_IF_ERROR(iree_runtime_instance_try_create_default_device(
      instance, iree_make_cstring_view("vmla"), &device));

  // Set up the session to run the demo module.
  // Sessions are like OS processes and are used to isolate modules from each
  // other and hold runtime state such as the variables used within the module.
  // The same module loaded into two sessions will see their own private state.
  iree_runtime_session_options_t session_options;
  iree_runtime_session_options_initialize(&session_options);
  iree_runtime_session_t* session = NULL;
  iree_status_t status = iree_runtime_session_create_with_device(
      instance, &session_options, device,
      iree_runtime_instance_host_allocator(instance), &session);
  iree_hal_device_release(device);

  // Load the compiled user module in a demo-specific way.
  // Applications could specify files, embed the outputs directly in their
  // binaries, fetch them over the network, etc.
  if (iree_status_is_ok(status)) {
    status = iree_runtime_demo_load_module(session);
  }

  // Build and issue the call.
  if (iree_status_is_ok(status)) {
    status = iree_runtime_demo_perform_mul(session);
  }

  // Release the session and free all resources.
  iree_runtime_session_release(session);
  return status;
}

//===----------------------------------------------------------------------===//
// 3. Call a function within a module with buffer views
//===----------------------------------------------------------------------===//
// The inputs and outputs of a call are reusable across calls (and possibly
// across sessions depending on device compatibility) and can be setup by the
// application as needed. For example, an application could perform
// multi-threaded buffer view creation and then issue the call from a single
// thread when all inputs are ready. This simple demo just allocates them
// per-call and throws them away.

// Sets up and calls the simple_mul function and dumps the results:
// func @simple_mul(%arg0: tensor<4xf32>, %arg1: tensor<4xf32>) -> tensor<4xf32>
//
// NOTE: this is a demo and as such this performs no memoization; a real
// application could reuse a lot of these structures and cache lookups of
// iree_vm_function_t to reduce the amount of per-call overhead.
static iree_status_t iree_runtime_demo_perform_mul(
    iree_runtime_session_t* session) {
  // Initialize the call to the function.
  iree_runtime_call_t call;
  IREE_RETURN_IF_ERROR(iree_runtime_call_initialize_by_name(
      session, iree_make_cstring_view("module.simple_mul"), &call));

  // Append the function inputs with the HAL device allocator in use by the
  // session. The buffers will be usable within the session and _may_ be usable
  // in other sessions depending on whether they share a compatible device.
  // %arg0: tensor<4xf32>, %arg1: tensor<4xf32>
  iree_status_t status = iree_ok_status();
  {
    static const iree_hal_dim_t input_shape[1] = {4};
    static const float arg0_data[4] = {1.0f, 1.1f, 1.2f, 1.3f};
    static const float arg1_data[4] = {10.0f, 100.0f, 1000.0f, 10000.0f};
    // DO NOT SUBMIT
    if (iree_status_is_ok(status)) {
      // status = iree_runtime_append_buffer_view_copy(
      //     iree_runtime_call_inputs(&call),
      //     iree_runtime_session_device_allocator(session),
      //     IREE_HAL_ELEMENT_TYPE_FLOAT_32, input_shape,
      //     IREE_ARRAYSIZE(input_shape), arg0_data);
    }
    if (iree_status_is_ok(status)) {
      // status = iree_runtime_append_buffer_view_copy(
      //     iree_runtime_call_inputs(&call),
      //     iree_runtime_session_device_allocator(session),
      //     IREE_HAL_ELEMENT_TYPE_FLOAT_32, input_shape,
      //     IREE_ARRAYSIZE(input_shape), arg1_data);
    }
  }

  // Synchronously perform the call.
  if (iree_status_is_ok(status)) {
    status = iree_runtime_call_invoke(&call, /*flags=*/0);
  }

  // Dump the function outputs.
  if (iree_status_is_ok(status)) {
    // DO NOT SUBMIT
    // iree_runtime_call_outputs(&call);
  }

  iree_runtime_call_deinitialize(&call);
  return status;
}
