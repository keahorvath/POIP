# Tools - Outils pour le challenge POIP

## Structure

```
tools/
├── main.py              # Point d'entree
├── warehouse_loader.py  # Chargement des donnees
└── checker.py           # Validation + calcul cout
```

## Utilisation

```bash
python main.py
```

Par defaut, valide l'instance `warehouses/warehouse_toy`. Modifier le chemin dans `main.py` pour une autre instance.

Le checker prend en entree :
- `instance` : objet charge par `WarehouseLoader.load_all()` contenant les donnees de l'instance
- `solution_dir` : chemin vers le dossier contenant `rack_product_assignment.txt`

## Fichiers d'instance (entree)

| Fichier | Contenu |
|---------|---------|
| `metadata.txt` | Parametres globaux |
| `rack_capacity.txt` | Capacite de chaque rack |
| `rack_adjacency_matrix.txt` | Matrice des distances |
| `product_circuit.txt` | Circuit de chaque produit |
| `aisle_racks.txt` | Racks par allee |
| `orders.txt` | Commandes clients |

## Fichier de solution (sortie)

`solutions/rack_product_assignment.txt` : rack assigne a chaque produit.

```
<nb_produits>
<rack_produit_0>
<rack_produit_1>
...
```

## Contraintes verifiees par le checker

1. **Capacite** : produits/rack <= capacite
2. **Aeration** : emplacements vides >= ceil(capacite_allee * taux)
3. **Contiguite** : intervalles [min_rack, max_rack] par circuit non chevauchants (sauf aux limites)


