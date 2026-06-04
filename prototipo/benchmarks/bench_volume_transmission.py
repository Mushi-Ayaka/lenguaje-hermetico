"""Benchmark: numeric integration vs analytic for uniform slab transmission.
Genera CSV en out/.
"""
import math, os, csv

OUT_DIR = os.path.join(os.path.dirname(__file__), 'out')
os.makedirs(OUT_DIR, exist_ok=True)


def integrate_numeric(alpha_per_m, color, thickness, step_size):
    T = 1.0
    C = [0.0, 0.0, 0.0]
    z = 0.0
    while z < thickness:
        delta = min(step_size, thickness - z)
        # alpha per meter -> per segment
        a = alpha_per_m
        trans = math.exp(-a * delta)
        # emission approximated as c * (1 - exp(-a*delta))
        for i in range(3):
            C[i] += T * color[i] * (1 - trans)
        T *= trans
        z += delta
    return C, T


def analytic(alpha_per_m, color, thickness, solid_color):
    T = math.exp(-alpha_per_m * thickness)
    emission = [color[i] * (1 - T) for i in range(3)]
    final = [emission[i] + T * solid_color[i] for i in range(3)]
    return final, T


if __name__ == '__main__':
    color = [1.0, 0.0, 0.0]
    solid_color = [1.0, 1.0, 1.0]
    thickness = 1.0

    alpha_values = [0.2, 0.5, 1.0, 2.0, 5.0]
    step_sizes = [0.1, 0.05, 0.01, 0.005, 0.001]

    csv_path = os.path.join(OUT_DIR, 'volume_transmission.csv')
    with open(csv_path, 'w', newline='') as f:
        w = csv.writer(f)
        w.writerow(['alpha', 'step_size', 'numeric_r0', 'numeric_r1', 'numeric_r2', 'numeric_T', 'analytic_r0', 'analytic_r1', 'analytic_r2', 'analytic_T', 'err_L1'])
        for a in alpha_values:
            analytic_final, analytic_T = analytic(a, color, thickness, solid_color)
            for step in step_sizes:
                numeric_final, numeric_T = integrate_numeric(a, color, thickness, step)
                # final observed when solid behind slab
                final_numeric = [numeric_final[i] + numeric_T * solid_color[i] for i in range(3)]
                err = sum(abs(final_numeric[i]-analytic_final[i]) for i in range(3))
                w.writerow([a, step] + numeric_final + [numeric_T] + analytic_final + [analytic_T, err])
    print('Wrote', csv_path)
