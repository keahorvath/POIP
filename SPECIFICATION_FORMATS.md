# Spécification des formats de données

Ce document décrit les formats des fichiers d'entrée et de sortie pour le problème de slotting en entrepôt.

## Convention des points de départ et d'arrivée

Dans toutes les instances, les racks spéciaux sont définis comme suit :
- **Rack 0** : point de départ (start) — capacité 0
- **Rack n-1** (dernier rack) : point d'arrivée (end) — capacité 0

Ces deux racks sont inclus dans la matrice d'adjacence et permettent de calculer les distances de parcours des commandes.

---

# Structure d'une instance

```
warehouse_XXX/
├── metadata.txt              # Grandeurs globales
├── rack_capacity.txt         # Capacite de chaque rack
├── rack_adjacency_matrix.txt      # Matrice des distances
├── product_circuit.txt       # Circuit de chaque produit
├── aisle_racks.txt          # Racks par allee
├── orders.txt                # Commandes clients
│
├── help/                     # Fichiers d'aide (optionnels)
│   ├── rack_coordinates.txt
│   └── aisle_directions.txt
│
└── solutions/                # Solutions des etudiants
    └── rack_product_assignment.txt
```

---

# 1. Fichiers d'entree

## 1.1 metadata.txt

Grandeurs globales du probleme.

```
<nombre_de_racks>
<nombre_total_d_emplacements>
<taux_d_aeration_en_pourcentage>
<nombre_de_produits>
<nombre_de_circuits>
<nombre_d_allees>
<nombre_de_commandes>
```

---

## 1.2 rack_capacity.txt

Capacite de chaque rack.

```
<nombre_de_racks>
<capacite_rack_0>
<capacite_rack_1>
...
```

---

## 1.3 rack_adjacency_matrix.txt

Matrice des distances entre noeuds (racks + start/end).

```
<nombre_de_noeuds>
<distance[0][0]> <distance[0][1]> ...
<distance[1][0]> <distance[1][1]> ...
...
```

- Matrice asymetrique : d[i][j] != d[j][i]
- node_id = 0 : depart
- node_id = n-1 : arrivee

---

## 1.4 product_circuit.txt

Circuit logistique de chaque produit.

```
<nombre_de_produits>
<circuit_produit_0>
<circuit_produit_1>
...
```

---

## 1.5 aisle_racks.txt

Racks appartenant a chaque allee.

```
<nombre_d_allees>
<nb_racks> <rack_id_1> <rack_id_2> ...
...
```

---

## 1.6 orders.txt

Commandes clients.

```
<nombre_de_commandes>
<nb_produits> <product_id_1> <product_id_2> ...
...
```

---

# 2. Fichier de solution

## 2.1 rack_product_assignment.txt

Rack d'affectation de chaque produit.

```
<nombre_de_produits>
<rack_du_produit_0>
<rack_du_produit_1>
...
```

---

## 2.2 Contraintes de validation

### Capacite des racks
- Nombre de produits <= capacite
- Aucun produit dans les racks de capacite 0

### Aeration par allee
- Minimum requis : `ceil(capacite_totale x taux_aeration)` emplacements vides
- Aeration = capacite - nombre de produits

### Contiguite des circuits
Pour chaque circuit, intervalle [rack_min, rack_max].
Les intervalles ne peuvent se chevaucher qu'aux limites.

- Valide : circuit 1:[1,45], circuit 2:[46,60], circuit 3:[60,77]
- Invalide : circuit 1:[1,45], circuit 3:[44,77]

---

# 3. Fichiers d'aide

## 3.1 rack_coordinates.txt

Coordonnees XY des racks pour visualisation.

```
<nombre_de_racks>
<x_rack_0> <y_rack_0>
<x_rack_1> <y_rack_1>
...
```

---

## 3.2 aisle_directions.txt

Direction de parcours de chaque allee.

```
<nombre_d_allees>
<direction_allee_0>
<direction_allee_1>
...
```

| Code | Direction |
|------|-----------|
| 0 | DOWN (Y decroissant) |
| 1 | UP (Y croissant) |
| 2 | LEFT (X decroissant) |
| 3 | RIGHT (X croissant) |
| 4 | NO_DIRS (double sens) |
