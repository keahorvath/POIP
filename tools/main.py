"""Point d'entree : charge une instance et valide la solution."""

import os
from warehouse_loader import WarehouseLoader
from checker import checker

# Repertoire racine du projet (parent de tools/)
ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def main(warehouse_dir):
    loader = WarehouseLoader(warehouse_dir)
    instance = loader.load_all()

    print("=== INSTANCE ===")
    m = instance.metadata
    print(f"Racks: {m['num_racks']} | Produits: {m['num_products']} | "
          f"Circuits: {m['num_circuits']} | Commandes: {m['num_orders']}")

    print("\n=== VALIDATION ===")
    solution_dir = os.path.join(warehouse_dir, "solutions")
    checker(instance, solution_dir)


if __name__ == "__main__":
    main(os.path.join(ROOT_DIR, "warehouses", "warehouse_big_market"))
