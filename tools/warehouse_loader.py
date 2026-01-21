"""Chargeur de donnees d'instance d'entrepot."""

import os
from dataclasses import dataclass
from typing import Dict, List


@dataclass
class WarehouseInstance:
    """Instance complete d'un probleme de slotting."""
    adjacency: List[List[int]]
    rack_capacity: List[int]
    product_circuit: List[int]
    aisles_racks: List[List[int]]
    orders: List[List[int]]
    metadata: Dict[str, float]


class WarehouseLoader:
    """Charge les donnees d'un entrepot depuis un repertoire."""

    def __init__(self, warehouse_dir: str):
        self.dir = warehouse_dir

    def _path(self, filename: str) -> str:
        return os.path.join(self.dir, filename)

    def _read_lines(self, filename: str) -> List[str]:
        with open(self._path(filename)) as f:
            return [l.strip() for l in f if l.strip() and not l.startswith("...")]

    def load_adjacency_matrix(self) -> List[List[int]]:
        lines = self._read_lines("rack_adjacency_matrix.txt")
        n = int(lines[0])
        return [list(map(int, lines[i + 1].split())) for i in range(n)]

    def load_rack_capacity(self) -> List[int]:
        lines = self._read_lines("rack_capacity.txt")
        return list(map(int, lines[1:]))

    def load_product_circuits(self) -> List[int]:
        lines = self._read_lines("product_circuit.txt")
        return list(map(int, lines[1:]))

    def load_aisles_racks(self) -> List[List[int]]:
        lines = self._read_lines("aisle_racks.txt")
        # Nouveau format: <nb_racks> <rack_id_1> <rack_id_2> ..., on skip le premier element
        return [list(map(int, l.split()))[1:] for l in lines[1:]]

    def load_orders(self) -> List[List[int]]:
        lines = self._read_lines("orders.txt")
        return [list(map(int, l.split()))[1:] for l in lines[1:]]

    def load_metadata(self) -> Dict[str, float]:
        lines = self._read_lines("metadata.txt")
        keys = ["num_racks", "total_slots", "aeration_rate",
                "num_products", "num_circuits", "num_aisles", "num_orders"]
        return {k: float(lines[i]) if k == "aeration_rate" else int(lines[i])
                for i, k in enumerate(keys)}

    def load_all(self) -> WarehouseInstance:
        return WarehouseInstance(
            adjacency=self.load_adjacency_matrix(),
            rack_capacity=self.load_rack_capacity(),
            product_circuit=self.load_product_circuits(),
            aisles_racks=self.load_aisles_racks(),
            orders=self.load_orders(),
            metadata=self.load_metadata(),
        )
