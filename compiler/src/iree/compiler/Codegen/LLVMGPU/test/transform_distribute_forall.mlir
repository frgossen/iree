// RUN: iree-opt %s --pass-pipeline="builtin.module(hal.executable(iree-transform-dialect-interpreter,transform-dialect-drop-schedule))" | FileCheck %s

#executable_target_cuda_nvptx_fb = #hal.executable.target<"cuda", "cuda-nvptx-fb", {target_arch = "sm_60"}>
#map = affine_map<()[s0] -> (s0 * 8)>
#map1 = affine_map<(d0) -> (d0)>
#pipeline_layout = #hal.pipeline.layout<push_constants = 0, sets = [<0, bindings = [<0, storage_buffer, ReadOnly>, <1, storage_buffer>]>]>
#translation = #iree_codegen.translation_info<TransformDialectCodegen>
hal.executable private @distribute {
  hal.executable.variant public @cuda_nvptx_fb, target = #executable_target_cuda_nvptx_fb {
    hal.executable.export public @distribute ordinal(0) layout(#pipeline_layout) attributes {translation_info = #translation} {
    ^bb0(%arg0: !hal.device, %arg1: index, %arg2: index):
      %c1 = arith.constant 1 : index
      hal.return %arg1, %c1, %c1 : index, index, index
    }
    builtin.module {

//       CHECK: #[[$DIV32MOD8:.*]] = affine_map<()[s0] -> ((s0 floordiv 32) mod 8)>
// CHECK-LABEL: func.func @distribute
      func.func @distribute() {
        %cst_0 = arith.constant dense<0.000000e+00> : vector<1xf16>
        %c250 = arith.constant 250 : index
        %c8 = arith.constant 8 : index
        %c0 = arith.constant 0 : index
        %1 = hal.interface.binding.subspan set(0) binding(1) type(storage_buffer) alignment(64) offset(%c0) : memref<2xf16>
        memref.assume_alignment %1, 64 : memref<2xf16>
        %workgroup_id_x = hal.interface.workgroup.id[0] : index
        %subview = memref.subview %1[%workgroup_id_x] [1] [1] : memref<2xf16> to memref<1xf16, strided<[1], offset: ?>>

// CHECK: %[[TX:.+]] = gpu.thread_id  x
// CHECK: %[[COND:.*]] = arith.cmpi ult
// CHECK: scf.if %[[COND]] {
// CHECK:   vector.transfer_write %{{.*}}, %{{.*}}[%[[TX]]] {in_bounds = [true]} : vector<1xf16>, memref<1xf16, strided<[1], offset: ?>>
        scf.forall (%arg0) in (%c250) {
          vector.transfer_write %cst_0, %subview[%arg0]
          {in_bounds = [true]} : vector<1xf16>, memref<1xf16, strided<[1], offset: ?>>
        } {mapping = [#gpu.thread<x>]}

// CHECK: %[[WX:.+]] = affine.apply #[[$DIV32MOD8]]()[%[[TX]]]
// CHECK: vector.transfer_write %{{.*}}, %{{.*}}[%[[WX]]] {in_bounds = [true]} : vector<1xf16>, memref<1xf16, strided<[1], offset: ?>>
        scf.forall (%arg0) in (%c8) {
          vector.transfer_write %cst_0, %subview[%arg0]
          {in_bounds = [true]} : vector<1xf16>, memref<1xf16, strided<[1], offset: ?>>
        } {mapping = [#gpu.warp<x>]}        
        return
      }
      module {
        transform.sequence failures(propagate) {
        ^bb0(%variant_op: !transform.any_op):
        %17 = transform.structured.match ops{["func.func"]} in %variant_op 
          : (!transform.any_op) -> !transform.any_op
        transform.iree.map_nested_forall_to_gpu_threads %17 
          workgroup_dims = [256, 1, 1] warp_dims = [8, 1, 1] : (!transform.any_op) -> ()

        // Late canonicalizations to cleanup and pass the checks.
        // Needs to occur on the whole variant to perform cse on the workgroup_count region
        transform.apply_patterns to %variant_op {
          transform.apply_patterns.canonicalization
        } : !transform.any_op
        transform.iree.apply_licm %variant_op : !transform.any_op
        transform.iree.apply_cse %variant_op : !transform.any_op
      }
    }
  }
}
}
