"""Benchmark: BVH refit vs rebuild time for dynamic objects.
Simula una carga de trabajo de N cajas que se mueven y mide el tiempo relativo.
"""
import time
import random
import os
import csv

OUT_DIR = os.path.join(os.path.dirname(__file__), 'out')
os.makedirs(OUT_DIR, exist_ok=True)

class BBox:
    def __init__(self, min_p, max_p):
        self.min = list(min_p)
        self.max = list(max_p)

    def union(self, other):
        return BBox(
            [min(self.min[i], other.min[i]) for i in range(3)],
            [max(self.max[i], other.max[i]) for i in range(3)]
        )

class BVHNode:
    def __init__(self, bbox, left=None, right=None, leaf_idx=None):
        self.bbox = bbox
        self.left = left
        self.right = right
        self.leaf_idx = leaf_idx

def build_recursive(bboxes, indices):
    if len(indices) == 1:
        return BVHNode(bboxes[indices[0]], leaf_idx=indices[0])
    
    # Simple split: alternate axes or use longest axis
    # Here we just split in half for benchmarking purposes
    mid = len(indices) // 2
    left = build_recursive(bboxes, indices[:mid])
    right = build_recursive(bboxes, indices[mid:])
    return BVHNode(left.bbox.union(right.bbox), left, right)

def refit_recursive(node, bboxes):
    if node.leaf_idx is not None:
        node.bbox = bboxes[node.leaf_idx]
        return node.bbox
    
    left_bbox = refit_recursive(node.left, bboxes)
    right_bbox = refit_recursive(node.right, bboxes)
    node.bbox = left_bbox.union(right_bbox)
    return node.bbox

def benchmark_bvh():
    counts = [10, 100, 1000, 5000]
    csv_path = os.path.join(OUT_DIR, 'bvh_bench_results.csv')
    
    with open(csv_path, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['node_count', 'rebuild_ms', 'refit_ms', 'speedup_x'])
        
        for n in counts:
            # Setup
            bboxes = [BBox([i, i, i], [i+1, i+1, i+1]) for i in range(n)]
            indices = list(range(n))
            
            # Rebuild timing
            start = time.perf_counter()
            root = build_recursive(bboxes, indices)
            rebuild_time = (time.perf_counter() - start) * 1000
            
            # Simular movimiento
            for b in bboxes:
                b.min[0] += 0.1
                b.max[0] += 0.1
                
            # Refit timing
            start = time.perf_counter()
            refit_recursive(root, bboxes)
            refit_time = (time.perf_counter() - start) * 1000
            
            speedup = rebuild_time / refit_time if refit_time > 0 else 0
            w.writerow([n, rebuild_time, refit_time, speedup])
            print(f"N={n}: Rebuild={rebuild_time:.2f}ms, Refit={refit_time:.2f}ms, Speedup={speedup:.1f}x")

if __name__ == '__main__':
    benchmark_bvh()
