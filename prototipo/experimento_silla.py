import math
from PIL import Image

# Implementación autónoma de la silla bajo la representación hermética:
# tensor 1x8 con coordenadas [X,Y,Z,W,R,G,B,A]

WOOD_COLOR = (139, 69, 19)
METAL_COLOR = (200, 200, 200)


def smooth_min(a, b, k=0.3):
    h = max(k - abs(a - b), 0.0) / k
    return min(a, b) - h * h * k * 0.25


def length(v):
    return math.sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2])


def normalize(v):
    l = length(v)
    return (v[0] / l, v[1] / l, v[2] / l) if l != 0 else (0.0, 0.0, 0.0)


def dot(a, b):
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def reflect(v, n):
    d = 2.0 * dot(v, n)
    return (v[0] - d * n[0], v[1] - d * n[1], v[2] - d * n[2])


def sdf_rounded_box(x, y, z, hx, hy, hz, r):
    qx = abs(x) - hx
    qy = abs(y) - hy
    qz = abs(z) - hz
    return length((max(qx, 0.0), max(qy, 0.0), max(qz, 0.0))) + min(max(qx, max(qy, qz)), 0.0) - r


def sdf_cylinder_y(x, y, z, r, h):
    qx = length((x, z)) - r
    qy = abs(y) - h
    return length((max(qx, 0.0), max(qy, 0.0), 0.0)) + min(max(qx, qy), 0.0)


def chair_parts(x, y, z):
    seat = sdf_rounded_box(x, y - 0.45, z, 3.5, 0.35, 3.5, 0.12)
    back = sdf_rounded_box(x, y - 2.0, z + 3.2, 3.5, 2.0, 0.3, 0.12)
    leg1 = sdf_rounded_box(x - 3.0, y + 1.7, z - 3.0, 0.25, 1.7, 0.25, 0.08)
    leg2 = sdf_rounded_box(x + 3.0, y + 1.7, z - 3.0, 0.25, 1.7, 0.25, 0.08)
    leg3 = sdf_rounded_box(x - 3.0, y + 1.7, z + 3.0, 0.25, 1.7, 0.25, 0.08)
    leg4 = sdf_rounded_box(x + 3.0, y + 1.7, z + 3.0, 0.25, 1.7, 0.25, 0.08)

    parts = {
        "seat": seat,
        "back": back,
        "leg1": leg1,
        "leg2": leg2,
        "leg3": leg3,
        "leg4": leg4,
    }

    # Unión suave para mantener los sólidos integrados
    d = seat
    d = smooth_min(d, back, 0.4)
    d = smooth_min(d, leg1, 0.2)
    d = smooth_min(d, leg2, 0.2)
    d = smooth_min(d, leg3, 0.2)
    d = smooth_min(d, leg4, 0.2)

    return d, parts


def chair_distance(x, y, z):
    dist, _ = chair_parts(x, y, z)
    return dist


def chair_material(x, y, z):
    _, parts = chair_parts(x, y, z)
    name, _ = min(parts.items(), key=lambda item: item[1])
    return name


def estimate_normal(x, y, z):
    eps = 0.0008
    nx = chair_distance(x + eps, y, z) - chair_distance(x - eps, y, z)
    ny = chair_distance(x, y + eps, z) - chair_distance(x, y - eps, z)
    nz = chair_distance(x, y, z + eps) - chair_distance(x, y, z - eps)
    return normalize((nx, ny, nz))


def wood_grain(x, y, z):
    return 0.15 * math.sin(18.0 * x + 0.6 * z) + 0.08 * math.cos(25.0 * z - 0.4 * x) + 0.04 * math.sin(12.0 * y)


def shade_point(x, y, z, normal, part):
    light_dir = normalize((0.6, 0.8, -1.0))
    view_dir = normalize((0.0, 0.0, -1.0))
    diffuse = max(0.0, dot(normal, light_dir))
    specular = pow(max(0.0, dot(reflect((-light_dir[0], -light_dir[1], -light_dir[2]), normal), view_dir)), 40)
    ambient = 0.1

    if part in ("seat", "back"):
        base_r, base_g, base_b = WOOD_COLOR
        variation = wood_grain(x * 0.4, y * 0.4, z * 0.4)
        base_r = base_r * (1.0 + variation)
        base_g = base_g * (1.0 + variation * 0.8)
        base_b = base_b * (1.0 + variation * 0.6)
        r = base_r * (ambient + 0.8 * diffuse) + 90.0 * specular
        g = base_g * (ambient + 0.8 * diffuse) + 70.0 * specular
        b = base_b * (ambient + 0.7 * diffuse) + 40.0 * specular
    else:
        base_r, base_g, base_b = METAL_COLOR
        r = base_r * (ambient + 0.75 * diffuse) + 170.0 * specular
        g = base_g * (ambient + 0.75 * diffuse) + 170.0 * specular
        b = base_b * (ambient + 0.75 * diffuse) + 190.0 * specular

    return (max(0, min(255, int(r))), max(0, min(255, int(g))), max(0, min(255, int(b))))


def ray_march(orig, dir_vec, max_steps=90, max_dist=80.0, eps=0.001):
    dist = 0.0
    for _ in range(max_steps):
        point = (orig[0] + dir_vec[0] * dist, orig[1] + dir_vec[1] * dist, orig[2] + dir_vec[2] * dist)
        d = chair_distance(point[0], point[1], point[2])
        if d < eps:
            normal = estimate_normal(point[0], point[1], point[2])
            part = chair_material(point[0], point[1], point[2])
            color = shade_point(point[0], point[1], point[2], normal, part)
            return True, color, normal, point
        dist += d
        if dist >= max_dist:
            break
    return False, None, None, None


def render_image(width=512, height=512, filename="silla.png"):
    camera_pos = (0.0, 1.6, 12.0)
    target = (0.0, 1.2, 0.0)
    up = (0.0, 1.0, 0.0)
    forward = normalize((target[0] - camera_pos[0], target[1] - camera_pos[1], target[2] - camera_pos[2]))
    right = normalize((forward[2] * up[1] - forward[1] * up[2], forward[0] * up[2] - forward[2] * up[0], forward[1] * up[0] - forward[0] * up[1]))
    screen_height = 5.5
    screen_width = screen_height * (width / height)
    image = Image.new("RGB", (width, height), (8, 8, 12))
    pixels = image.load()

    for j in range(height):
        v = (height - 1 - j + 0.5) / height * 2.0 - 1.0
        for i in range(width):
            u = (i + 0.5) / width * 2.0 - 1.0
            ray_dir = normalize((forward[0] + u * screen_width * right[0] + v * screen_height * up[0],
                                 forward[1] + u * screen_width * right[1] + v * screen_height * up[1],
                                 forward[2] + u * screen_width * right[2] + v * screen_height * up[2]))
            hit, color, _, _ = ray_march(camera_pos, ray_dir)
            pixels[i, j] = color if hit else (8, 8, 12)

    image.save(filename)
    return filename


if __name__ == "__main__":
    output_file = render_image(width=512, height=512, filename="silla.png")
    print(f"Imagen generada: {output_file}")
