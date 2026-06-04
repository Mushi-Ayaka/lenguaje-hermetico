from __future__ import annotations
from dataclasses import dataclass
from typing import List

R_CH, G_CH, B_CH, A_CH, X_CH, Y_CH, Z_CH, W_CH = range(8)
NUM_CHANNELS = 8
CHANNEL_NAMES = ["R", "G", "B", "A", "X", "Y", "Z", "W"]


@dataclass(frozen=True)
class Tensor1x8:
    values: tuple[float, float, float, float, float, float, float, float]

    def __post_init__(self):
        assert len(self.values) == NUM_CHANNELS

    @staticmethod
    def zero() -> Tensor1x8:
        return Tensor1x8((0.0,) * NUM_CHANNELS)

    @staticmethod
    def from_list(v: List[float]) -> Tensor1x8:
        assert len(v) == NUM_CHANNELS
        return Tensor1x8(tuple(v))

    def get(self, ch: int) -> float:
        return self.values[ch]

    def with_channel(self, ch: int, value: float) -> Tensor1x8:
        lst = list(self.values)
        lst[ch] = value
        return Tensor1x8(tuple(lst))

    def perturb(self, **kwargs: float) -> Tensor1x8:
        lst = list(self.values)
        for name, val in kwargs.items():
            lst[CHANNEL_NAMES.index(name.upper())] = val
        return Tensor1x8(tuple(lst))

    def map(self, fn) -> Tensor1x8:
        return Tensor1x8(tuple(fn(v) for v in self.values))

    def combine(self, other: Tensor1x8, op) -> Tensor1x8:
        return Tensor1x8(tuple(op(a, b) for a, b in zip(self.values, other.values)))

    def is_zero(self, tolerance: float = 0.0) -> bool:
        return all(abs(v) <= tolerance for v in self.values)

    def __repr__(self) -> str:
        parts = ", ".join(f"{n}={v:.2f}" for n, v in zip(CHANNEL_NAMES, self.values))
        return f"T1x8({parts})"
