"""Reglas ontológicas.

Una regla es una función que recibe:
- field: Field completo (para consultar vecinos)
- x, y, z, w: coordenadas a evaluar
- context: dict con parámetros adicionales

y devuelve:
- Tensor1x8 o None (None = no modifica el estado base)
"""

from __future__ import annotations
import math
from typing import Callable, Optional
from dataclasses import dataclass, field
from tensor import Tensor1x8, X_CH, Y_CH, Z_CH, W_CH, R_CH, G_CH, B_CH, A_CH
from field import Field

RuleFn = Callable[[Field, float, float, float, float, dict], Optional[Tensor1x8]]


@dataclass
class Rule:
    name: str
    description: str
    fn: RuleFn
    params: dict = field(default_factory=dict)
    priority: int = 0

    def apply(self, field: Field, x: float, y: float, z: float, w: float,
              extra: dict = None) -> Optional[Tensor1x8]:
        ctx = {**self.params, **(extra or {})}
        return self.fn(field, x, y, z, w, ctx)


# --- Reglas predefinidas ---

def punto_estatico(initial: Tensor1x8, x0: float = 0, y0: float = 0, z0: float = 0):
    """Una entidad que mantiene su estado constante en todo W en una posición fija."""
    def _rule(field, x, y, z, w, ctx):
        if x == x0 and y == y0 and z == z0:
            return initial
        return None
    return Rule(
        name="punto_estatico",
        description=f"Entidad estática en ({x0},{y0},{z0})",
        fn=_rule
    )


def propagacion_radial(velocidad: float = 1.0, decay: float = 1.0,
                       fuente_x: float = 0, fuente_y: float = 0, fuente_z: float = 0):
    """Propaga energía desde una fuente, expandiéndose radialmente con el tiempo W.

    En el universo de bloque, esto es una relación geométrica:
    - La energía llega a distancia r cuando W >= r/velocidad
    - Decae con 1/(1 + r^2)
    """
    def _rule(field, x, y, z, w, ctx):
        dx, dy, dz = x - fuente_x, y - fuente_y, z - fuente_z
        dist = (dx*dx + dy*dy + dz*dz) ** 0.5

        if w <= 0 or dist > w * velocidad:
            return None

        # Obtener la energía de la fuente en W=0
        src = field.get(fuente_x, fuente_y, fuente_z, 0)
        if src.is_zero():
            return None

        energia = 1.0 / (1.0 + dist * dist * decay)
        retardo = dist / velocidad
        factor_atenuacion = max(0, (w - retardo) / (w + 1))

        return Tensor1x8.from_list([
            src.get(R_CH) * energia * factor_atenuacion,
            src.get(G_CH) * energia * factor_atenuacion,
            src.get(B_CH) * energia * factor_atenuacion,
            src.get(A_CH) * energia * factor_atenuacion,
            x, y, z, w
        ])
    return Rule(
        name="propagacion_radial",
        description=f"Propagación radial desde ({fuente_x},{fuente_y},{fuente_z}) v={velocidad}",
        fn=_rule
    )


# --- Reglas de silla hiperrealista ---

def _dot(a, b):
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]


def _length(v):
    return (v[0] * v[0] + v[1] * v[1] + v[2] * v[2]) ** 0.5


def _normalize(v):
    l = _length(v)
    return (v[0] / l, v[1] / l, v[2] / l) if l != 0 else (0.0, 0.0, 0.0)


def _reflect(v, n):
    d = 2 * _dot(v, n)
    return (v[0] - d * n[0], v[1] - d * n[1], v[2] - d * n[2])


def _clamp_color(r, g, b):
    return (
        max(0.0, min(255.0, r)),
        max(0.0, min(255.0, g)),
        max(0.0, min(255.0, b)),
    )


def _sdf_box(x, y, z, hx, hy, hz):
    return max(abs(x) - hx, abs(y) - hy, abs(z) - hz)


def _sdf_round_box(x, y, z, hx, hy, hz, radius):
    qx = abs(x) - hx
    qy = abs(y) - hy
    qz = abs(z) - hz
    return _length((max(qx, 0.0), max(qy, 0.0), max(qz, 0.0))) + min(max(qx, max(qy, qz)), 0.0) - radius


def _sdf_cylinder_y(x, y, z, radius, height):
    qx = abs(x) - radius
    qy = abs(y) - height
    return _length((max(qx, 0.0), max(qy, 0.0), 0.0)) + min(max(qx, qy), 0.0)


def _smooth_min(a, b, k=0.25):
    h = max(k - abs(a - b), 0.0) / k
    return min(a, b) - h * h * k * 0.25


def _wood_variation(x, y, z):
    grain = 0.08 * math.sin(12.0 * x + 0.5 * z)
    ring = 0.06 * math.cos(18.0 * z + 0.3 * x)
    age = 0.04 * math.sin(40.0 * y)
    return grain + ring + age


def _material_shade(part_id, normal, light_dir, view_dir, x, y, z):
    diffuse = max(0.0, _dot(normal, light_dir))
    reflect_dir = _reflect(tuple(-v for v in light_dir), normal)
    specular = pow(max(0.0, _dot(reflect_dir, view_dir)), 32)

    if part_id in ("seat", "back"):
        base_r, base_g, base_b = 139.0, 69.0, 19.0
        variation = _wood_variation(x * 0.5, y * 0.5, z * 0.5)
        base_r *= 1.0 + variation
        base_g *= 1.0 + variation * 0.8
        base_b *= 1.0 + variation * 0.6
        color = (
            base_r * (0.25 + 0.7 * diffuse) + 180.0 * specular,
            base_g * (0.25 + 0.7 * diffuse) + 140.0 * specular,
            base_b * (0.25 + 0.7 * diffuse) +  80.0 * specular,
        )
    else:
        base_r, base_g, base_b = 200.0, 200.0, 200.0
        metallic = 0.7
        color = (
            base_r * (0.20 + 0.65 * diffuse) + 220.0 * specular * metallic,
            base_g * (0.20 + 0.65 * diffuse) + 220.0 * specular * metallic,
            base_b * (0.20 + 0.65 * diffuse) + 240.0 * specular * metallic,
        )

    return _clamp_color(*color)


def _chair_distance(x, y, z):
    seat = _sdf_round_box(x, y, z, 4.0, 0.5, 4.0, 0.15)
    back = _sdf_round_box(x, y - 4.0, z + 4.0, 4.0, 4.0, 0.5, 0.15)
    legs = [
        _sdf_round_box(x - 3.5, y + 3.0, z - 3.5, 0.5, 3.0, 0.5, 0.08),
        _sdf_round_box(x + 3.5, y + 3.0, z - 3.5, 0.5, 3.0, 0.5, 0.08),
        _sdf_round_box(x - 3.5, y + 3.0, z + 3.5, 0.5, 3.0, 0.5, 0.08),
        _sdf_round_box(x + 3.5, y + 3.0, z + 3.5, 0.5, 3.0, 0.5, 0.08),
    ]

    # Suavizar transiciones entre partes para un sólido más integrado
    d = seat
    d = _smooth_min(d, back, 0.35)
    for leg in legs:
        d = _smooth_min(d, leg, 0.25)

    return d, {
        "seat": seat,
        "back": back,
        "leg1": legs[0],
        "leg2": legs[1],
        "leg3": legs[2],
        "leg4": legs[3],
    }


def _estimate_normal(x, y, z):
    eps = 0.001
    dx = _chair_distance(x + eps, y, z)[0] - _chair_distance(x - eps, y, z)[0]
    dy = _chair_distance(x, y + eps, z)[0] - _chair_distance(x, y - eps, z)[0]
    dz = _chair_distance(x, y, z + eps)[0] - _chair_distance(x, y, z - eps)[0]
    return _normalize((dx, dy, dz))


def silla_hiperrealista():
    """Regla que describe una silla sólida con luz y materiales nativos 1x8."""
    light_dir = _normalize((1.0, 0.8, -1.2))
    view_dir = _normalize((0.0, 0.0, -1.0))

    def _rule(field, x, y, z, w, ctx):
        dist, parts = _chair_distance(x, y, z)
        if dist > 0.0:
            return None

        part_id = min(parts.items(), key=lambda item: item[1])[0]
        normal = _estimate_normal(x, y, z)
        r, g, b = _material_shade(part_id, normal, light_dir, view_dir, x, y, z)
        alpha = 255.0

        return Tensor1x8.from_list([r, g, b, alpha, x, y, z, w])

    return Rule(
        name="silla_hiperrealista",
        description="Regla hiperrealista de silla construida como sólidos y luz interna",
        fn=_rule,
        priority=10
    )
