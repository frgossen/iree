## Copyright 2022 The IREE Authors
#
# Licensed under the Apache License v2.0 with LLVM Exceptions.
# See https://llvm.org/LICENSE.txt for license information.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Defines IREE CUDA benchmarks."""

from typing import List, Tuple, Sequence
from benchmark_suites.iree import benchmark_tags, module_execution_configs
from e2e_test_framework import unique_ids
from e2e_test_framework.definitions import common_definitions, iree_definitions
from e2e_test_framework.device_specs import device_collections
from e2e_test_framework.models import model_groups
import benchmark_suites.iree.utils


class Linux_CUDA_Benchmarks(object):
  """Benchmarks on CUDA Linux devices."""

  SM_80_GPU_TARGET = iree_definitions.CompileTarget(
      target_architecture=common_definitions.DeviceArchitecture.CUDA_SM80,
      target_backend=iree_definitions.TargetBackend.CUDA,
      target_abi=iree_definitions.TargetABI.LINUX_GNU)
  SM_80_COMPILE_CONFIG = iree_definitions.CompileConfig.build(
      id=unique_ids.IREE_COMPILE_CONFIG_LINUX_CUDA_SM80_DEFAULTS,
      tags=["default-flags"],
      compile_targets=[SM_80_GPU_TARGET])
  SM_80_UBENCH_MATMUL_COMPILE_CONFIG = iree_definitions.CompileConfig.build(
      id=unique_ids.IREE_COMPILE_CONFIG_LINUX_CUDA_SM80_MATMUL_UBENCH,
      tags=["ukernel", "matmul"],
      compile_targets=[SM_80_GPU_TARGET],
      extra_flags=["--iree-hal-benchmark-dispatch-repeat-count=100"])
  SM_80_UBENCH_MATMUL_SPLITK_COMPILE_CONFIG = iree_definitions.CompileConfig.build(
      id=unique_ids.IREE_COMPILE_CONFIG_LINUX_CUDA_SM80_MATMUL_SPLITK_UBENCH,
      tags=["ukernel", "matmul", "splitk"],
      compile_targets=[SM_80_GPU_TARGET],
      extra_flags=[
          "--iree-hal-benchmark-dispatch-repeat-count=100",
          "--iree-flow-split-matmul-reduction=4",
          "--iree-codegen-llvmgpu-use-wmma"
      ])

  def _generate_configs(
      self,
      models: Sequence[common_definitions.Model],
      compile_config: iree_definitions.CompileConfig,
      execution_config: iree_definitions.
      ModuleExecutionConfig = module_execution_configs.CUDA_CONFIG,
      tags: Sequence[str] = (),
  ) -> Tuple[List[iree_definitions.ModuleGenerationConfig],
             List[iree_definitions.E2EModelRunConfig]]:
    gen_configs = [
        iree_definitions.ModuleGenerationConfig.build(
            compile_config=compile_config,
            imported_model=iree_definitions.ImportedModel.from_model(model),
            tags=tags) for model in models
    ]
    sm80_devices = device_collections.DEFAULT_DEVICE_COLLECTION.query_device_specs(
        architecture=common_definitions.DeviceArchitecture.NVIDIA_AMPERE,
        host_environment=common_definitions.HostEnvironment.LINUX_X86_64)
    run_module_configs = benchmark_suites.iree.utils.generate_e2e_model_run_configs(
        module_generation_configs=gen_configs,
        module_execution_configs=[execution_config],
        device_specs=sm80_devices,
        tags=tags)

    return (gen_configs, run_module_configs)

  def generate(
      self
  ) -> Tuple[List[iree_definitions.ModuleGenerationConfig],
             List[iree_definitions.E2EModelRunConfig]]:
    """Generates IREE compile and run configs."""
    # The CUDA tag is required to put them into the CUDA benchmark preset.
    gen_configs, run_configs = self._generate_configs(
        model_groups.CUDA_MODELS,
        self.SM_80_COMPILE_CONFIG,
        tags=[benchmark_tags.CUDA])
    ubench_gen_configs, ubench_run_configs = self._generate_configs(
        model_groups.MICRO_MATMUL,
        self.SM_80_UBENCH_MATMUL_COMPILE_CONFIG,
        execution_config=module_execution_configs.CUDA_BATCH_SIZE_100_CONFIG,
        tags=[benchmark_tags.CUDA])
    ubench_splitk_gen_configs, ubench_splitk_run_configs = self._generate_configs(
        model_groups.MICRO_MATMUL_SPLITK,
        self.SM_80_UBENCH_MATMUL_SPLITK_COMPILE_CONFIG,
        execution_config=module_execution_configs.CUDA_BATCH_SIZE_100_CONFIG,
        tags=[benchmark_tags.CUDA])
    large_gen_configs, large_module_configs = self._generate_configs(
        model_groups.CUDA_MODELS_LONG,
        self.SM_80_COMPILE_CONFIG,
        tags=[benchmark_tags.CUDA, benchmark_tags.LARGE])
    return (gen_configs + ubench_gen_configs + ubench_splitk_gen_configs +
            large_gen_configs, run_configs + ubench_run_configs +
            ubench_splitk_run_configs + large_module_configs)
