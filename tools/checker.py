"""
Checker de solutions pour le probleme de slotting.

Verifications :
1. Tous les produits assignes a un rack valide
2. Capacite des racks respectee
3. Aeration minimale par allee
4. Contiguite des circuits (intervalles non chevauchants sauf aux limites)
"""

import os
import math
from collections import defaultdict


def read_solution(file_path):
    """Lit rack_product_assignment.txt : ligne 0 = nb produits, puis un rack par ligne."""
    with open(file_path) as f:
        lines = [l.strip() for l in f if l.strip()]
    n = int(lines[0])
    if len(lines) - 1 != n:
        raise ValueError(f"Fichier annonce {n} produits mais contient {len(lines) - 1} lignes.")
    return [int(lines[i + 1]) for i in range(n)]


def check_all_products_assigned(instance, rack_of_product):
    """Verifie que chaque produit a un rack valide."""
    expected = instance.metadata["num_products"]
    num_racks = len(instance.rack_capacity)

    if len(rack_of_product) != expected:
        raise ValueError(f"{len(rack_of_product)} produits fournis, {expected} attendus.")

    for p, r in enumerate(rack_of_product):
        if not (0 <= r < num_racks):
            raise ValueError(f"Produit {p} : rack {r} invalide (0 a {num_racks - 1}).")


def check_rack_capacity(instance, rack_of_product):
    """Verifie que le nombre de produits par rack <= capacite."""
    count = defaultdict(int)
    for r in rack_of_product:
        count[r] += 1

    for r, n in count.items():
        cap = instance.rack_capacity[r]
        if n > cap:
            raise ValueError(f"Rack {r} : {n} produits pour capacite {cap}.")


def check_aeration(instance, rack_of_product):
    """Verifie l'aeration minimale par allee."""
    rate = instance.metadata["aeration_rate"] / 100.0
    count = defaultdict(int)
    for r in rack_of_product:
        count[r] += 1

    for i, aisle in enumerate(instance.aisles_racks):
        total_cap = sum(instance.rack_capacity[r] for r in aisle)
        min_aeration = math.ceil(total_cap * rate)
        actual = sum(instance.rack_capacity[r] - count[r] for r in aisle)
        if actual < min_aeration:
            raise ValueError(f"Allee {i} : aeration {actual}, minimum {min_aeration}.")


def check_circuit_contiguity(instance, rack_of_product):
    """
    Verifie la contiguite des circuits par intervalles.
    Les intervalles [min, max] ne peuvent se chevaucher qu'a une limite.
    """
    product_circuit = instance.product_circuit
    intervals = {}

    for p, r in enumerate(rack_of_product):
        c = product_circuit[p]
        if c not in intervals:
            intervals[c] = [r, r]
        else:
            intervals[c][0] = min(intervals[c][0], r)
            intervals[c][1] = max(intervals[c][1], r)

    # Tri par (borne inf, borne sup) puis comparaison des voisins (O(n log n))
    circuits = sorted(intervals.keys(), key=lambda c: (intervals[c][0], intervals[c][1]))
    for c1, c2 in zip(circuits, circuits[1:]):
        # Chevauchement interdit si max1 > min2 (egalite aux limites autorisee)
        if intervals[c1][1] > intervals[c2][0]:
            raise ValueError(
                f"Contiguite violee : circuit {c1} {intervals[c1]} "
                f"et circuit {c2} {intervals[c2]}."
            )


def calculate_cost(instance, rack_of_product):
    """Calcule le cout total : somme des distances de collecte."""
    adj = instance.adjacency
    start, end = 0, len(adj) - 1
    total = 0

    for order in instance.orders:
        racks = sorted({rack_of_product[p] for p in order})
        path = [start] + racks + [end]
        total += sum(adj[path[i]][path[i + 1]] for i in range(len(path) - 1))

    return total


def checker(instance, solution_dir):
    """Execute toutes les verifications et affiche le resultat."""
    try:
        path = os.path.join(solution_dir, "rack_product_assignment.txt")
        rack_of_product = read_solution(path)

        check_all_products_assigned(instance, rack_of_product)
        check_rack_capacity(instance, rack_of_product)
        check_aeration(instance, rack_of_product)
        check_circuit_contiguity(instance, rack_of_product)

        cost = calculate_cost(instance, rack_of_product)
        print(f"Solution valide. Cout = {cost}")
        return cost

    except (ValueError, FileNotFoundError) as e:
        print(f"ERREUR : {e}")
        return None
