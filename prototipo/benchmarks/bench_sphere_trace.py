"""Benchmark: sphere-trace steps vs eps and L_est for a unit sphere SDF.
Genera CSV de resultados en out/.
"""
import math, os, csv

OUT_DIR = os.path.join(os.path.dirname(__file__), 'out')
os.makedirs(OUT_DIR, exist_ok=True)


def norm(p):
    return math.sqrt(sum(x*x for x in p))


def sphere_sdf(center, r, p):
    return norm([p[i]-center[i] for i in range(3)]) - r


def sphere_trace(ray_origin, ray_dir, sdf_fn, max_steps=1024, eps=1e-4, L_est=1.0, z_max=100.0):
    z = 0.0
    steps = 0
    p = list(ray_origin)
    while steps < max_steps and z < z_max:
        p = [ray_origin[i] + z*ray_dir[i] for i in range(3)]
        d = sdf_fn(p)
        if math.isnan(d):
            return None, steps
        if abs(d) <= eps:
            return z, steps
        step = max(eps, abs(d)/L_est)
        z += step
        steps += 1
    return None, steps


if __name__ == '__main__':
    center = (0.0, 0.0, 5.0)
    r = 1.0
    ray_origin = (0.0, 0.0, 0.0)
    ray_dir = (0.0, 0.0, 1.0)

    eps_values = [1e-2, 5e-3, 1e-3, 5e-4, 1e-4, 5e-5, 1e-5]
    L_est_values = [0.5, 1.0, 2.0]

    csv_path = os.path.join(OUT_DIR, 'sphere_trace_results.csv')
    with open(csv_path, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['eps', 'L_est', 'hit_z', 'steps'])
        for eps in eps_values:
            for L_est in L_est_values:
                z_hit, steps = sphere_trace(ray_origin, ray_dir, lambda p: sphere_sdf(center, r, p), max_steps=10000, eps=eps, L_est=L_est, z_max=20.0)
                w.writerow([eps, L_est, '' if z_hit is None else float(z_hit), steps])
    print('Wrote', csv_path)
