# Projet de Simulation de Système de Fichiers UNIX

## Présentation

Ce projet a pour objectif d’implémenter une bibliothèque de fonctions simulant la gestion de fichiers dans un système UNIX. L’idée est de travailler dans un fichier unique, représentant une partition, sans utiliser les fonctions standards de gestion de fichiers d’UNIX, sauf pour la création initiale de cette partition.

Le projet inclut également un programme de test interactif, en ligne de commande, permettant de tester les fonctions principales de la bibliothèque.

Le projet est fonctionnel mais avec quelques erreurs de gestion de la partition

## Fonctionnalités implémentées

La bibliothèque permet :

- Le formatage d’une partition (fichier de base)
- La création ou ouverture de fichiers internes à la partition
- L’écriture et la lecture dans ces fichiers
- Le déplacement du pointeur de lecture/écriture
- L'effacement d'un fichier 
