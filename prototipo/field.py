"""Campo tensorial disperso infinito.

Solo almacena puntos con valor no-cero. El espacio es conceptualmente
infinito: cualquier coordenada no presente se considera vacío (tensor zero).
"""

from __future__ import annotations
from collections import defaultdict
from tensor import Tensor1x8, CHANNEL_NAMES, W_CH, X_CH, Y_CH, Z_CH


class Field:
    def __init__(self, tolerance: float = 0.0):
        # key: (x, y, z, w) -> Tensor1x8
        self._data: dict[tuple[float, float, float, float], Tensor1x8] = {}
        self._tolerance = tolerance

    def _key(self, x: float, y: float, z: float, w: float) -> tuple[float, float, float, float]:
        return (x, y, z, w)

    def set(self, x: float, y: float, z: float, w: float, t: Tensor1x8):
        if t.is_zero(self._tolerance):
            self._data.pop(self._key(x, y, z, w), None)
        else:
            self._data[self._key(x, y, z, w)] = t

    def get(self, x: float, y: float, z: float, w: float) -> Tensor1x8:
        return self._data.get(self._key(x, y, z, w), Tensor1x8.zero())

    def perturb(self, x: float, y: float, z: float, w: float, **kwargs):
        old = self.get(x, y, z, w)
        self.set(x, y, z, w, old.perturb(**kwargs))

    def slice_w(self, w: float):
        """Itera sobre todos los puntos en un instante W dado."""
        for (x, y, z, wt), t in self._data.items():
            if wt == w:
                yield (x, y, z, t)

    def slice_z(self, z: float, w: float):
        """Itera sobre puntos en una capa Z y tiempo W."""
        for (x, y, zt, wt), t in self._data.items():
            if zt == z and wt == w:
                yield (x, y, t)

    def bounds(self, w: float):
        """Devuelve (min_x, max_x, min_y, max_y) para un instante W."""
        xs, ys = [], []
        for (x, y, _, wt), _ in self._data.items():
            if wt == w:
                xs.append(x)
                ys.append(y)
        if not xs:
            return (0, 0, 0, 0)
        return (min(xs), max(xs), min(ys), max(ys))

    def num_points(self) -> int:
        return len(self._data)

    def num_points_at(self, w: float) -> int:
        return sum(1 for (_, _, _, wt) in self._data if wt == w)

    def all_points(self):
        return self._data.items()
