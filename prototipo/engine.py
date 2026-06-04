"""Motor de resolución ontológica.

Toma un conjunto de reglas y perturbaciones, y evalúa el campo
en cualquier punto (x,y,z,w) de forma determinista.

El motor no simula paso a paso. Evalúa bajo demanda:
dado (x,y,z,w), computa el tensor resultante aplicando
todas las reglas aplicables en orden de prioridad.
"""

from __future__ import annotations
from collections import defaultdict
from dataclasses import dataclass, field
from typing import List, Optional
from tensor import Tensor1x8
from field import Field
from rule import Rule


class Engine:
    def __init__(self):
        self.field = Field()
        self.rules: List[Rule] = []
        self._cache: dict[tuple[float, float, float, float], Tensor1x8] = {}
        self._cache_hits = 0
        self._cache_misses = 0

    def add_rule(self, rule: Rule):
        self.rules.append(rule)
        self.rules.sort(key=lambda r: r.priority)
        self._cache.clear()

    def add_rules(self, rules: List[Rule]):
        for r in rules:
            self.add_rule(r)

    def perturb(self, x: float, y: float, z: float, w: float, **kwargs):
        self.field.perturb(x, y, z, w, **kwargs)
        self._cache.clear()

    def evaluate(self, x: float, y: float, z: float, w: float,
                 use_cache: bool = True) -> Tensor1x8:
        key = (x, y, z, w)
        if use_cache and key in self._cache:
            self._cache_hits += 1
            return self._cache[key]

        self._cache_misses += 1

        # 1. Estado base: lo que hay en el campo
        result = self.field.get(x, y, z, w)

        # 2. Aplicar reglas en orden de prioridad
        acumulado = result
        for rule in self.rules:
            modificacion = rule.apply(self.field, x, y, z, w)
            if modificacion is not None:
                # Fusión: suma de influencias (superposición lineal)
                # En un modelo completo, esto dependería del tipo de interacción
                acumulado = acumulado.combine(
                    modificacion,
                    lambda a, b: a + b
                )

        if use_cache:
            self._cache[key] = acumulado
        return acumulado

    def evaluate_grid_2d(self, w: float, z: float,
                         x_min: int, x_max: int,
                         y_min: int, y_max: int) -> dict:
        """Evalúa una rebanada 2D del campo en un instante W y capa Z."""
        results = {}
        for y in range(y_min, y_max + 1):
            for x in range(x_min, x_max + 1):
                t = self.evaluate(float(x), float(y), float(z), float(w))
                if not t.is_zero():
                    results[(x, y)] = t
        return results

    def check_consistency(self, x: float, y: float, z: float, w: float) -> bool:
        """Verifica que no hay contradicciones en un punto.

        Una contradicción ocurre cuando dos reglas intentan establecer
        valores diferentes en el mismo canal para el mismo punto.
        Esto es una comprobación simplificada; la real requiere evaluar
        reglas por separado y detectar conflictos.
        """
        results = []
        base = self.field.get(x, y, z, w)
        results.append(("base", base))

        for rule in self.rules:
            mod = rule.apply(self.field, x, y, z, w)
            if mod is not None:
                results.append((rule.name, mod))

        # El primer resultado no-base que difiera significativamente del
        # acumulado podría indicar conflicto (simplificado)
        for name, t in results[1:]:
            for ch in range(8):
                v = t.values[ch]
                if v != 0:
                    for name2, t2 in results[1:]:
                        v2 = t2.values[ch]
                        if name != name2 and v2 != 0 and abs(v - v2) > 0.001:
                            return False
        return True

    def clear_cache(self):
        self._cache.clear()
        self._cache_hits = 0
        self._cache_misses = 0

    def stats(self) -> dict:
        return {
            "puntos_en_campo": self.field.num_points(),
            "reglas": len(self.rules),
            "cache_hits": self._cache_hits,
            "cache_misses": self._cache_misses,
        }
