"""Point d'entree : charge une instance et valide la solution."""

import os

from warehouse_loader import WarehouseLoader
from checker import checker
from analyse_donnees import Analyse_donnees

ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

ENABLE_WRITES = {
    "freq_prod": True,
    "freq_circuit": True,
    "concord_prod": True,
    "concord_circuit": True,
}

def get_warehouse_dir(instance_name: str) -> str:
    return os.path.join(ROOT_DIR, "warehouses", instance_name)


def get_analyse_data_dir() -> str:
    path = os.path.join(ROOT_DIR, "analyse_data")
    os.makedirs(path, exist_ok=True)
    return path


def build_out_path(out_dir: str, prefix: str, instance_name: str) -> str:
    return os.path.join(out_dir, f"{prefix}_{instance_name}.txt")

def run_analysis(instance, instance_name: str):
    print("\n=== ANALYSE ===")

    print("Exécution de : initialisation Analyse_donnees")
    analyse = Analyse_donnees(instance)

    analyse.pourcentage_prod = 0
    analyse.pourcentage_circuit = 0
    analyse.nb_prod_affiche = analyse.instance.metadata['num_products']

    print("Exécution de : calcul_nb_prod_in_family")
    analyse.calcul_nb_prod_in_family()

    print("Exécution de : stats_commandes")
    analyse.stats_commandes()

    print("Exécution de : get_frequence_produits_and_circuit")
    analyse.get_frequence_produits_and_circuit()

    out_dir = get_analyse_data_dir()

    # --- Écritures contrôlées ---
    if ENABLE_WRITES["freq_prod"]:
        freq_prod_file = build_out_path(out_dir, "freq_prod", instance_name)
        print(f"Écriture de : fréquences produits -> {freq_prod_file}")
        analyse.write_freq_prod(freq_prod_file)

    if ENABLE_WRITES["concord_prod"]:
        concord_prod_file = build_out_path(out_dir, "concord_prod_same_circuit", instance_name)
        print(f"Écriture de : concordance produits même circuit -> {concord_prod_file}")
        analyse.write_concordance_prod_in_circuit(concord_prod_file)

    if ENABLE_WRITES["freq_circuit"]:
        freq_circuit_file = build_out_path(out_dir, "freq_circuit", instance_name)
        print(f"Écriture de : fréquences circuits -> {freq_circuit_file}")
        analyse.write_freq_circuit(freq_circuit_file)

    if ENABLE_WRITES["concord_circuit"]:
        concord_circuit_file = build_out_path(out_dir, "concord_circuit", instance_name)
        print(f"Écriture de : concordance circuits -> {concord_circuit_file}")
        analyse.write_concordance_circuit_to_file(analyse.mat_concordance_circuit, concord_circuit_file)

    #print("Exécution de : print_stats")
    #analyse.print_stats()

def main(warehouse_dir, instance_name: str):
    loader = WarehouseLoader(warehouse_dir)
    instance = loader.load_all()

    print("=== INSTANCE ===")
    m = instance.metadata
    print(f"Racks: {m['num_racks']} | Produits: {m['num_products']} | "
          f"Circuits: {m['num_circuits']} | Commandes: {m['num_orders']}")

    print("\n=== VALIDATION ===")
    solution_dir = os.path.join(warehouse_dir, "solutions")
    checker(instance, solution_dir)

    run_analysis(instance, instance_name)

if __name__ == "__main__":
    #instance_name = "warehouse_toy"
    #instance_name = "warehouse_big_market"
    #instance_name = "warehouse_big_category"
    instance_name = "warehouse_big_family"

    print(f"Lancement instance : {instance_name}")
    main(get_warehouse_dir(instance_name), instance_name)
