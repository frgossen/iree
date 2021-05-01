# Copyright 2021 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load("//build_tools/bazel:run_binary_test.bzl", "run_binary_test")

package(
    default_visibility = ["//visibility:public"],
    features = ["layering_check"],
    licenses = ["notice"],  # Apache 2.0
)

#===------------------------------------------------------------------------===#
# Public API
#===------------------------------------------------------------------------===#

cc_library(
    name = "runtime",
    hdrs = [
        "api.h",
    ],
    deps = [
        ":impl",
        "//iree/base",
    ],
)

#===------------------------------------------------------------------------===#
# Implementation
#===------------------------------------------------------------------------===#

cc_library(
    name = "impl",
    srcs = [
        "buffer_util.c",
        "call.c",
        "instance.c",
        "session.c",
        "status_util.c",
    ],
    hdrs = [
        "buffer_util.h",
        "call.h",
        "instance.h",
        "session.h",
        "status_util.h",
    ],
    deps = [
        "//iree/base",
        "//iree/base:core_headers",
        "//iree/base:tracing",
        "//iree/base/internal",
        "//iree/base/internal:file_io",
        "//iree/hal",
        "//iree/hal/drivers",
        "//iree/modules/hal",
        "//iree/vm",
        "//iree/vm:bytecode_module",
    ],
)

#===------------------------------------------------------------------------===#
# Demos
#===------------------------------------------------------------------------===#

cc_binary(
    name = "session_demo",
    srcs = [
        "session_demo.c",
        "session_demo_file.c",
    ],
    deps = [
        ":runtime",
    ],
)

cc_binary(
    name = "session_demo_embedded",
    srcs = [
        "session_demo.c",
        "session_demo_embedded.c",
    ],
    deps = [
        ":runtime",
        "//iree/runtime/testdata:mul_vmla_module_c",
    ],
)

run_binary_test(
    name = "session_demo_embedded_test",
    test_binary = ":session_demo_embedded",
)
