"""Point d'entree : charge une instance et valide la solution."""

import os
from warehouse_loader import WarehouseLoader, WarehouseInstance
from checker import checker
import matplotlib.pyplot as plt
from statistics import mean, median
from collections import Counter
import numpy as np


# Repertoire racine du projet (parent de tools/)
ROOT_DIR = os.path.dirname(os.path.abspath(__file__))

class Analyse_donnees :
    """
    Classe contenant une analyse des données.

    Données analysées : 
    - nb_max_prod : le nombre maximum de produits d'une commande.
    - nb_min_prod : le nombre minimum de produits d'une commande.
    - nb_moy_prod : le nombre moyen de produits d'une commande.
    - nb_med_prod : le nombre médian de produits d'une commande.
    - freq_prod : liste des fréquences d'apparition des produits dans les commandes. Chaque élément de la liste est un 
                        triplet de la forme (id_produit, nombre d'apparition, pourcentage d'apparition).
    - freq_circuit : liste des fréquences d'apparition des circuits dans les commandes. Chaque élément de la liste est un 
                        triplet de la forme (id_circuit, nombre d'apparition, pourcentage d'apparition).
    - pourcentage_prod : choix du pourcentage d'apparition d'un produit pour être affiché dans les statistiques.
    - pourcentage_circuit : choix du pourcentage d'apparition d'un circuit pour être affiché dans les statistiques.
    - nb_prod_affiche : nombre de produits à afficher dans les statistiques.
    - mat_concordance : matrice de concordance des circuits.
    """
    def __init__(self, WarehouseInstance ,nb_max_prod, nb_min_prod, nb_moy_prod, nb_med_prod, freq_prod, freq_circuit, pourcentage_prod, pourcentage_circuit, nb_prod_affiche, mat_concordance,nb_prod_in_family):
        self.instance = WarehouseInstance
        self.commandes = WarehouseInstance.orders
        self.nb_max_prod = nb_max_prod
        self.nb_min_prod = nb_min_prod
        self.nb_moy_prod = nb_moy_prod
        self.nb_med_prod = nb_med_prod
        self.freq_prod = freq_prod
        self.freq_circuit = freq_circuit
        self.pourcentage_prod = pourcentage_prod      
        self.pourcentage_circuit = pourcentage_circuit 
        self.nb_prod_affiche = nb_prod_affiche         
        self.mat_concordance = mat_concordance     
        self.nb_prod_in_family = nb_prod_in_family

    def stats_commandes(self):
        """
        Renvoie min, max, moyenne et médiane des tailles de commandes.
        """
        tailles = [len(c) for c in self.commandes]
        if not tailles:
            return None
        self.nb_min_prod = min(tailles)
        self.nb_max_prod = max(tailles)
        self.nb_moy_prod = mean(tailles)
        self.nb_med_prod = median(tailles)

    def calcul_nb_prod_in_family(self):
        """
        Calcule le nombre de produits par circuit.
        """
        self.nb_prod_in_family = Counter()
        for prod in self.instance.product_circuit:
            self.nb_prod_in_family[prod] += 1


    def get_frequence_produits_and_circuit(self):
        """
        Compte :
        - Dans combien de commandes apparait chaque produit.
        - Dans combien de commandes apparait chaque circuit.

        Calcule aussi la matrice de concordance des circuits pour chaque paire de circuits (c1, c2)
        """
        total_commandes = len(self.commandes)
        compteur_prod = Counter()
        compteur_family = Counter()
        self.mat_concordance = np.zeros((self.instance.metadata['num_circuits'], self.instance.metadata['num_circuits']))
        
        for cmd in self.commandes:
            produits_uniques = set(cmd)
            compteur_prod.update(produits_uniques)
            family_products = []
            for id in produits_uniques :
                family_products.append(self.instance.product_circuit[id])
            family_products = set(family_products)
            compteur_family.update(family_products)
            for c1 in family_products :
                for c2 in family_products :
                    self.mat_concordance[c1][c2] += 1
            
        self.freq_prod = []
        for id_produit, count in compteur_prod.most_common():
            pourcentage = (count / total_commandes) * 100
            self.freq_prod.append((id_produit, count, pourcentage))

        self.freq_circuit = []
        for id_circuit, count in compteur_family.most_common():
            pourcentage = (count / total_commandes) * 100
            self.freq_circuit.append((id_circuit, count, pourcentage))

    def print_stats(self):
        print(f"Nombre max de produits : {self.nb_max_prod}")
        print(f"Nombre min de produits : {self.nb_min_prod}")
        print(f"Nombre moyen de produits : {self.nb_moy_prod}")
        print(f"Nombre médian de produits : {self.nb_med_prod}")
        print("\nFréquence d'apparition des produits :\n")
        compteur_prod = 0
        for id_produit, count, pourcentage in self.freq_prod[:self.nb_prod_affiche]:
            if pourcentage >= self.pourcentage_prod:
                print(f"Produit {id_produit}: {count} apparitions : {round(pourcentage,3)}%")
                compteur_prod += 1
        if compteur_prod == 0:
            print("Aucun produit n'est présent dans ", round(self.pourcentage_prod,3), "% des commandes")

        print("\nFréquence d'apparition des circuits :\n")
        compteur_family = 0
        for id_circuit, count, pourcentage in self.freq_circuit:
            if pourcentage >= self.pourcentage_circuit :
                print(f"Circuit {id_circuit} ({self.nb_prod_in_family[id_circuit]}): {count} apparitions : {round(pourcentage,3)}%")
                compteur_family += 1
        if compteur_family == 0:
            print("Aucun circuit n'est présent dans ", round(self.pourcentage_circuit,3), "% des commandes")

    def get_aisle_capacity(self):
        """
        Calcule la capacité totale par allée.
        """
        aisle_capacity = []
        for aisle in self.instance.aisles_racks:
            total_capacity = sum(self.instance.rack_capacity[rack_id] for rack_id in aisle)
            aisle_capacity.append(total_capacity)
        return aisle_capacity
"""
        print("\nConcordance des circuits :")
        for i in range(len(self.mat_concordance)-1):
            if sum(self.mat_concordance[i]) > 0: # Vérifie qu'il y a des concordances pour ce circuit
                print(f"\nCircuit {i} : ", end="")
            for j in range(len(self.mat_concordance)):
                if j > i and self.mat_concordance[i][j] > 0 :
                    print(f"({j}):", round(self.mat_concordance[i][j]/self.instance.metadata['num_orders'] * 100,3), "%  ", end="")
"""
            
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

    print(instance.adjacency)
    print(instance.adjacency[1][5])
    print(instance.adjacency[1][2] + instance.adjacency[2][3] + instance.adjacency[3][5])
    

"""
    print("\n=== Stats Commandes ===")
    analyse = Analyse_donnees(instance,0,0,0,0,[],[],0,0,10,[],0)
    analyse.pourcentage_prod = 5
    analyse.pourcentage_circuit = 0
    analyse.nb_prod_affiche = analyse.instance.metadata['num_products']
    analyse.calcul_nb_prod_in_family()
    print(analyse.nb_prod_in_family)
    analyse.stats_commandes()
    analyse.get_frequence_produits_and_circuit()
    analyse.print_stats()
    capacities = analyse.get_aisle_capacity()
    for i, cap in enumerate(capacities):
        print(f"Capa de l'allée {i} : {cap}", end="  " if (i+1) % 3 != 0 else "\n")
    if len(capacities) % 3 != 0:
        print()
    print("capacité max allée : ", max(capacities))
    print("capacité min allée : ", min(capacities))
"""

"""
    print("\nHistogramme de répartition des produits dans les commandes\n")
    T=[]
    for i in range (len(analyse.commandes)):
        T += analyse.commandes[i]
    # print("T =", T)
    plt.hist(T, bins=max(T)+1)
    plt.title("Repartition des produits dans les commandes")
    plt.xlabel("ID produit")
    plt.ylabel("Nombre d'apparitions")
    plt.show()
"""

if __name__ == "__main__":
    print(ROOT_DIR)
    main(os.path.join(ROOT_DIR, "warehouse_toy", "warehouse_toy"))
    #main(os.path.join(ROOT_DIR, "warehouse_big_", "warehouse_big_market"))

