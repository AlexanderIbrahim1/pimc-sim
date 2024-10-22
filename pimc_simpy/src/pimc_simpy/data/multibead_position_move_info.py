"""
This module contains the POD type MultibeadPositionMoveInfo.
"""

import dataclasses


@dataclasses.dataclass
class MultibeadPositionMoveInfo:
    upper_level_fraction: float
    lower_level: int
