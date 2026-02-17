import numpy as np
from statistics import mean, median
from collections import Counter

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
    - mat_concordance_circuit : matrice de concordance des circuits.
    - mat_concordance_produit_in_circuit : matrice de concordance des produits.
    """
    def __init__(self, WarehouseInstance):
        self.instance = WarehouseInstance
        self.commandes = WarehouseInstance.orders
        self.nb_max_prod = 0
        self.nb_min_prod = 0
        self.nb_moy_prod = 0
        self.nb_med_prod = 0
        self.freq_prod = []
        self.freq_circuit = []
        self.pourcentage_prod = 0
        self.pourcentage_circuit = 0
        self.nb_prod_affiche = 0
        self.mat_concordance_circuit = None
        self.mat_concordance_produit = None
        self.nb_prod_in_family = Counter()
        self.mat_concordance_prod_in_circuit = None

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
        Calcule aussi la matrice de concordance des produits pour chaque paire de produits (p1, p2)
        """
        total_commandes = len(self.commandes)
        compteur_prod = Counter()
        compteur_family = Counter()
        self.mat_concordance_circuit = np.zeros((self.instance.metadata['num_circuits'], self.instance.metadata['num_circuits']))
        self.mat_concordance_produit = np.zeros((self.instance.metadata['num_products'], self.instance.metadata['num_products']))
        self.mat_concordance_prod_in_circuit = np.zeros((self.instance.metadata['num_products'], self.instance.metadata['num_products']))
        
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
                    self.mat_concordance_circuit[c1][c2] += 1
            for p1 in produits_uniques:
                for p2 in produits_uniques:
                    self.mat_concordance_produit[p1][p2] += 1
            for pc1 in produits_uniques :
                for pc2 in produits_uniques :
                    if pc1 != pc2 :
                        if self.instance.product_circuit[pc1] == self.instance.product_circuit[pc2] :
                            self.mat_concordance_prod_in_circuit[pc1][pc2] += 1
            
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
       
        print("\nConcordance des circuits :")
        for i in range(len(self.mat_concordance_circuit)-1):
            if sum(self.mat_concordance_circuit[i]) > 0: # Vérifie qu'il y a des concordances pour ce circuit
                print(f"\nCircuit {i} : ", end="")
            for j in range(len(self.mat_concordance_circuit)):
                if j > i and self.mat_concordance_circuit[i][j] > 0 :
                    print(f"({j}):", round(self.mat_concordance_circuit[i][j]/self.instance.metadata['num_orders'] * 100,3), "%  ", end="")

    def get_aisle_capacity(self):
        """
        Calcule la capacité totale par allée.
        """
        aisle_capacity = []
        for aisle in self.instance.aisles_racks:
            total_capacity = sum(self.instance.rack_capacity[rack_id] for rack_id in aisle)
            aisle_capacity.append(total_capacity)
        return aisle_capacity

    def write_freq_circuit(self, filename):
        with open(filename, 'w') as f:
            f.write("id_circuit nb_prod_in_family count pourcentage\n")
            for id_circuit, count, pourcentage in self.freq_circuit:
                if pourcentage >= self.pourcentage_circuit :
                    f.write(f"{id_circuit} {self.nb_prod_in_family[id_circuit]} {count} {round(pourcentage,3)}\n")

    def write_freq_prod(self, filename):
        with open(filename, 'w') as f:
            f.write("id_prod count pourcentage\n")
            for id_prod, count, pourcentage in self.freq_prod:
                if pourcentage >= self.pourcentage_prod:
                    f.write(f"{id_prod} {count} {round(pourcentage,3)}\n")

    def write_concordance_produits(self, filename, min_freq=1):
        """
        Écrit la matrice de concordance des produits dans un fichier.

        Chaque ligne correspond à un produit p.
        Les produits co-présents avec p sont triés par fréquence décroissante.

        Format :
        p : q1(count, %) q2(count, %) ...
        """
        nb_commandes = self.instance.metadata['num_orders']
        with open(filename, 'w') as f:
            for p in range(self.instance.metadata['num_products']):
                ligne = []
                for q in range(self.instance.metadata['num_products']):
                    if p == q:
                        continue  # on ignore la diagonale
                    count = self.mat_concordance_produit[p][q]
                    if count >= min_freq:
                        pourcentage = count / nb_commandes * 100
                        ligne.append((q, count, pourcentage))
                # tri par fréquence décroissante
                ligne.sort(key=lambda x: x[1], reverse=True)
                # écriture
                f.write(str(p) + " ")
                for q, count, pct in ligne:
                    f.write(str(f"{q}({int(count)},{round(pct,2)}%) "))
                f.write("\n")

    def write_concordance_prod_in_circuit(self, filename) :
        """
        Write in filename.txt product which are most time with another one for each product
        
        Format :
        each ligne correspond to a product.
        product ligne : product_most_associated second_product_most_associated third_product_most_associated etc
        -1 on a ligne means product never appear in any order or it appears alone everytime
        """
        nb_product = self.instance.metadata['num_products']
        with open(filename, 'w') as f:
            for p in range(nb_product):
                sort_value = dict()
                ligne = []
                if self.mat_concordance_prod_in_circuit[p].any() == False :
                    ligne.append(-1)
                for q in range(nb_product):
                    if q != p and self.mat_concordance_prod_in_circuit[p][q] > 0 :
                        sort_value[q] = int(self.mat_concordance_prod_in_circuit[p][q])
                sort_value = dict(sorted(sort_value.items(), key=lambda item: item[1], reverse = True))
                for keys in sort_value :
                    ligne.append(keys)
                f.write(str(p) + " ")
                for q in ligne:
                    f.write(str(f"{q} "))
                f.write("\n")

    def write_concordance_circuit_to_file(self, mat_concordance_circuit, filename):
        """
        Ecrit la matrice de concordance des circuits dans un fichier texte.
        Format :
        - première ligne : N
        - puis N lignes avec N entiers séparés par des espaces
        """
        mat = np.array(mat_concordance_circuit, dtype=int)
        n = mat.shape[0]

        with open(filename, "w") as f:
            f.write(str(n) + "\n")
            for i in range(n):
                line = " ".join(str(int(mat[i][j])) for j in range(n))
                f.write(line + "\n")