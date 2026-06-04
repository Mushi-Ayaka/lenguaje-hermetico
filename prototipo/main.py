#!/usr/bin/env python3
"""
Prototipo del Cosmos Tensorial Axiomático.

Caso de prueba: Onda expansiva desde un punto fuente.
Se perturba el vacío en el origen con energía lumínica,
y una regla de propagación expande la energía radialmente
a través del espacio-tiempo.

Sin render, sin PIL. Solo lógica de campo tensorial.
"""

from tensor import Tensor1x8, R_CH, G_CH, B_CH, A_CH
from field import Field
from rule import punto_estatico, propagacion_radial, silla_hiperrealista
from engine import Engine
from export import (field_to_json, engine_slice_to_json,
                    slice_to_text_grid, export_gte_slot)


def test_onda_expansiva():
    """Prueba: fuente puntual emite energía que se propaga."""
    print("=" * 60)
    print("PRUEBA: ONDA EXPANSIVA DESDE EL ORIGEN")
    print("=" * 60)

    engine = Engine()

    # 1. Perturbar el vacío en (0,0,0,0) con energía lumínica
    fuente_energia = Tensor1x8.from_list([255.0, 200.0, 100.0, 255.0, 0, 0, 0, 0])
    engine.perturb(0, 0, 0, w=0, R=255, G=200, B=100, A=255)

    # 2. Añadir regla de propagación
    regla_prop = propagacion_radial(velocidad=1.0, decay=1.0)
    engine.add_rule(regla_prop)

    # 3. Evaluar en varios puntos a diferentes tiempos W
    print("\n--- Evaluación punto a punto ---")
    for w in [0, 1, 2, 3, 4, 5]:
        for dist in [0, 1, 2, 3, 4, 5, 10]:
            t = engine.evaluate(float(dist), 0.0, 0.0, float(w))
            if not t.is_zero():
                print(f"  W={w}, dist={dist}: A={t.get(A_CH):.1f}, "
                      f"R={t.get(R_CH):.1f}")

    # 4. Verificar que el frente de onda está donde debe
    print("\n--- Frente de onda ---")
    for w in range(1, 6):
        # A dist < w debe haber energía, a dist > w no
        dentro = engine.evaluate(float(w - 0.5), 0.0, 0.0, float(w))
        fuera = engine.evaluate(float(w + 0.5), 0.0, 0.0, float(w))
        ok = dentro.get(A_CH) > 0 and fuera.is_zero()
        print(f"  W={w}: en r={w-0.5} A={dentro.get(A_CH):.2f}, "
              f"en r={w+0.5} A={fuera.get(A_CH):.2f} " +
              ("(OK)" if ok else "(EXTRAÑO)"))

    # 5. Grid de texto para una rebanada en Z=0, W=fijo
    print("\n--- Grid de opacidad (W=3, Z=0) ---")
    grid = slice_to_text_grid(engine, w=3.0, z=0.0,
                              x_min=-5, x_max=5,
                              y_min=-5, y_max=5,
                              channel="A")
    print(grid)

    # 6. Estadísticas
    stats = engine.stats()
    print(f"\n--- Estadísticas ---")
    print(f"  Puntos en campo: {stats['puntos_en_campo']}")
    print(f"  Reglas: {stats['reglas']}")
    print(f"  Cache hits: {stats['cache_hits']}")
    print(f"  Cache misses: {stats['cache_misses']}")

    # 7. Exportar a JSON un slice
    print("\n--- Export JSON slice W=3, Z=0 ---")
    json_out = engine_slice_to_json(engine, w=3.0, z=0.0,
                                    x_range=(-3, 3), y_range=(-3, 3))
    print(json_out[:500] + "...")

    # 8. Generar código GTE para un frame
    print("\n--- Export GTE slot (W=3, cuadrante 0..4) ---")
    gte = export_gte_slot(engine, w=3.0, z=0.0,
                          x_min=0, x_max=4,
                          y_min=0, y_max=4)
    print(gte)

    print("\n[OK] Prueba completada.")
    return engine


def test_multiples_fuentes():
    """Prueba: dos fuentes cuyas ondas se superponen."""
    print("\n" + "=" * 60)
    print("PRUEBA: DOS FUENTES CON SUPERPOSICIÓN")
    print("=" * 60)

    engine = Engine()

    # Fuente 1 en (-3, 0)
    engine.perturb(-3, 0, 0, w=0, R=255, G=0, B=0, A=255)

    # Fuente 2 en (3, 0)
    engine.perturb(3, 0, 0, w=0, R=0, G=0, B=255, A=255)

    # Misma regla de propagación para ambas
    engine.add_rule(propagacion_radial(
        velocidad=1.0, decay=1.0,
        fuente_x=-3, fuente_y=0, fuente_z=0))

    engine.add_rule(propagacion_radial(
        velocidad=1.0, decay=1.0,
        fuente_x=3, fuente_y=0, fuente_z=0))

    # Grid en W=3, Z=0
    print("\n--- Grid de opacidad (W=4, Z=0) ---")
    grid = slice_to_text_grid(engine, w=4.0, z=0.0,
                              x_min=-8, x_max=8,
                              y_min=-4, y_max=4,
                              channel="A")
    print(grid)

    # Verificar punto medio entre fuentes (energía de ambas)
    t_medio = engine.evaluate(0.0, 0.0, 0.0, 4.0)
    print(f"\n  Punto medio (0,0) en W=4: "
          f"R={t_medio.get(0):.2f} G={t_medio.get(1):.2f} B={t_medio.get(2):.2f}")

    print("\n[OK] Prueba completada.")
    return engine


def test_silla_hiperrealista():
    """Prueba: renderizado de una silla definida solo por reglas y luz interna."""
    print("\n" + "=" * 60)
    print("PRUEBA: SILLA HIPERREALISTA")
    print("=" * 60)

    engine = Engine()
    engine.add_rule(silla_hiperrealista())

    print("\n--- Grid de intensidad alfa (W=0, Z=0) ---")
    grid = slice_to_text_grid(engine, w=0.0, z=0.0,
                              x_min=-10, x_max=10,
                              y_min=-10, y_max=10,
                              channel="A")
    print(grid)

    print("\n--- Estadísticas de la silla ---")
    stats = engine.stats()
    print(f"  Reglas: {stats['reglas']}")
    print(f"  Puntos en campo: {stats['puntos_en_campo']}")
    print(f"  Cache hits: {stats['cache_hits']}, misses: {stats['cache_misses']}")

    print("\n[OK] Prueba de silla completada.")
    return engine


if __name__ == "__main__":
    engine1 = test_onda_expansiva()
    engine2 = test_multiples_fuentes()
    engine3 = test_silla_hiperrealista()
