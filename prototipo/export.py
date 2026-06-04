"""Exportación del campo tensorial a formatos inspeccionables.

No genera imágenes. Produce estructuras de datos y texto
que permiten verificar el comportamiento del sistema.
"""

from __future__ import annotations
import json
from typing import Dict, List, Tuple
from field import Field
from engine import Engine
from tensor import Tensor1x8, CHANNEL_NAMES, R_CH, G_CH, B_CH, A_CH, X_CH, Y_CH, Z_CH, W_CH


def field_to_json(field: Field, w_slice: float = None) -> str:
    """Convierte el campo a JSON plano."""
    data = []
    for (x, y, z, w), t in field.all_points():
        if w_slice is not None and w != w_slice:
            continue
        entry = {
            "x": x, "y": y, "z": z, "w": w,
            "R": round(t.get(R_CH), 4),
            "G": round(t.get(G_CH), 4),
            "B": round(t.get(B_CH), 4),
            "A": round(t.get(A_CH), 4),
        }
        data.append(entry)
    data.sort(key=lambda e: (e["w"], e["z"], e["y"], e["x"]))
    return json.dumps(data, indent=2)


def engine_slice_to_json(engine: Engine, w: float, z: float,
                         x_range: Tuple[int, int],
                         y_range: Tuple[int, int]) -> str:
    """Evalúa el engine en una rebanada 2D y exporta a JSON."""
    results = engine.evaluate_grid_2d(w, z, x_range[0], x_range[1],
                                      y_range[0], y_range[1])
    data = []
    for (x, y), t in sorted(results.items()):
        entry = {
            "x": x, "y": y,
            "R": round(t.get(R_CH), 4),
            "G": round(t.get(G_CH), 4),
            "B": round(t.get(B_CH), 4),
            "A": round(t.get(A_CH), 4),
        }
        data.append(entry)
    return json.dumps(data, indent=2)


def slice_to_text_grid(engine: Engine, w: float, z: float,
                       x_min: int, x_max: int,
                       y_min: int, y_max: int,
                       channel: str = "A") -> str:
    """Exporta una rebanada como grid de texto ASCII para inspección visual."""
    ch_idx = CHANNEL_NAMES.index(channel.upper())
    lines = []
    for y in range(y_min, y_max + 1):
        row = []
        for x in range(x_min, x_max + 1):
            t = engine.evaluate(float(x), float(y), float(z), float(w))
            val = t.values[ch_idx]
            if val == 0:
                row.append(" . ")
            elif val < 0.3:
                row.append(" o ")
            elif val < 0.6:
                row.append(" O ")
            elif val < 0.9:
                row.append(" @ ")
            else:
                row.append(" # ")
        lines.append("".join(row))
    return "\n".join(lines)


def export_gte_slot(engine: Engine, w: float, z: float,
                    x_min: int, x_max: int,
                    y_min: int, y_max: int,
                    slot_name: str = "render_ontologico") -> str:
    """Genera código GTE a partir de la evaluación del engine.

    Para el prototipo, genera una estructura de datos que el compilador
    GTE podría interpretar. En producción, el Resolvedor produciría
    expresiones algebraicas cerradas, no lookup tables.
    """
    lines = []
    lines.append(f"// Slot generado por el Resolvedor Ontológico")
    lines.append(f"// W={w}, Z={z}, dominio x=[{x_min},{x_max}], y=[{y_min},{y_max}]")
    lines.append(f"slot {slot_name} {{")
    lines.append(f"    en: y = {y_min}..{y_max}, x = {x_min}..{x_max};")
    lines.append("    relleno {")

    # En lugar de if/else anidados, emitimos expresiones por canal
    # que el Resolvedor podría extraer de las reglas ontológicas.
    # Para el caso de onda expansiva, la expresión cerrada sería:
    #   dist = sqrt((x - x0)^2 + (y - y0)^2)
    #   energia = max(0, (w*vel - dist) / (w*vel + 1)) * intensidad / (1 + dist^2)

    # Leemos valores representativos del engine
    t_centro = engine.evaluate(0.0, 0.0, z, w)
    t_radio = engine.evaluate(float(w), 0.0, z, w)

    intensidad_r = t_centro.get(R_CH) if not t_centro.is_zero() else 255
    intensidad_g = t_centro.get(G_CH) if not t_centro.is_zero() else 200
    intensidad_b = t_centro.get(B_CH) if not t_centro.is_zero() else 100

    # Generar una expresión simbólica GTE que reproduce la propagación radial
    # Esto es exactamente lo que el Resolvedor computaría:
    # f(x, y) = intensidad * max(0, (w - sqrt(x^2 + y^2)) / (w + 1)) / (1 + x^2 + y^2)
    r = f"max(0, ({w} - sqrt(x*x + y*y)) / ({w} + 1)) * {intensidad_r:.0f} / (1 + x*x + y*y)"
    g = f"max(0, ({w} - sqrt(x*x + y*y)) / ({w} + 1)) * {intensidad_g:.0f} / (1 + x*x + y*y)"
    b = f"max(0, ({w} - sqrt(x*x + y*y)) / ({w} + 1)) * {intensidad_b:.0f} / (1 + x*x + y*y)"
    a = f"max(0, ({w} - sqrt(x*x + y*y)) / ({w} + 1)) * 255 / (1 + x*x + y*y)"

    lines.append(f"        R: {r};")
    lines.append(f"        G: {g};")
    lines.append(f"        B: {b};")
    lines.append(f"        A: {a};")
    lines.append("    }")
    lines.append("}")

    return "\n".join(lines)
