/**
 * @file projet.c
 * @brief Ce fichier contient les définitions des fonctions pour formater, ouvrir, écrire dans des fichiers et afficher de l'aide.
 */

#include "projet.h"

/**
 * @brief Fonction pour formater une partition.
 * @param partitionName Le nom de la partition à formater.
 * @return 0 si la partition est formatée avec succès, -1 en cas d'erreur.
 * @author Lauriane
 */
int myFormat(char* partitionName) {
    int partition_fd = open(partitionName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (partition_fd == -1) {
        perror("Erreur: Impossible de créer la partition.\n");
        return -1;
    }

    // Initialisation des informations de la partition
    super_file_data.num_inodes = NUM_INODES;
    super_file_data.taille_partition = 32768;
    super_file_data.fileDescriptor = partition_fd;

    // Initialisation des blocs de données comme libres
    for (int i = 0; i < MAX_NUM_BLOCKS; ++i) {
        super_file_data.blocks[i].occ_block = BLOCK_FREE;
    }

    printf("Partition '%s' formatée avec succès.\n", partitionName);

    return 0;
}

/**
 * @brief Fonction pour ouvrir un fichier.
 * @param fileName Le nom du fichier à ouvrir.
 * @return Un pointeur vers la structure de fichier ouvert, NULL en cas d'erreur.
 * @author Lauriane
 */
file* myOpen(char* fileName) {
    // Recherche de l'inode associé au nom de fichier donné
    for (int i = 0; i < super_file_data.num_inodes; ++i) {
        if (super_file_data.inodes[i].name != NULL && strcmp(super_file_data.inodes[i].name, fileName) == 0) {
            // Vérifier si un descripteur de fichier est déjà ouvert pour ce fichier
            if (super_file_data.inodes[i].file_pointer != NULL) {
                return super_file_data.inodes[i].file_pointer;
            }
        }
    }

    // Si aucun inode associé au fichier n'est trouvé, rechercher un inode libre
    for (int i = 0; i < super_file_data.num_inodes; ++i) {
        if (super_file_data.inodes[i].name == NULL) {
            // Recherche d'un bloc de données libre
            int free_block_index = -1;
            for (int j = 0; j < MAX_NUM_BLOCKS; ++j) {
                if (super_file_data.blocks[j].occ_block == BLOCK_FREE) {
                    free_block_index = j;
                    super_file_data.blocks[j].occ_block = BLOCK_OCCUPIED; // Marquer le bloc comme occupé
                    break;
                }
            }
            if (free_block_index == -1) {
                printf("Erreur : Aucun bloc de données disponible pour créer un nouveau fichier.\n");
                return NULL;
            }

            // Créer un nouveau fichier et l'associer à l'inode libre
            file* newFile = (file*)malloc(sizeof(file));
            if (newFile == NULL) {
                perror("Erreur lors de l'allocation de mémoire pour le fichier.");
                return NULL;
            }
            newFile->name = strdup(fileName);
            if (newFile->name == NULL) {
                perror("Erreur lors de l'allocation de mémoire pour le nom de fichier.");
                free(newFile);
                return NULL;
            }
            newFile->fileSize = 0;  // Initialiser la taille du fichier à 0
            newFile->currentPosition = 0; // Initialiser la position actuelle à 0

            // Associer le fichier à l'inode libre
            super_file_data.inodes[i].name = strdup(fileName);
            if (super_file_data.inodes[i].name == NULL) {
                perror("Erreur lors de l'allocation de mémoire pour le nom de fichier.");
                free(newFile->name);
                free(newFile);
                return NULL;
            }
            super_file_data.inodes[i].file_pointer = newFile;
            super_file_data.inodes[i].firstDataBlock = &super_file_data.blocks[free_block_index]; // Mise à jour du premier bloc de données dans l'inode
            super_file_data.inodes[i].firstDataBlock->next = NULL;

            return newFile;
        }
    }

    printf("Erreur : Aucun inode disponible pour créer un nouveau fichier.\n");
    return NULL;
}

/**
 * @brief Fonction pour écrire dans un fichier.
 * @param f Le pointeur vers la structure de fichier.
 * @param buffer Le tampon contenant les données à écrire.
 * @param nBytes Le nombre d'octets à écrire.
 * @return Le nombre total d'octets écrits, -1 en cas d'erreur.
 * @author Lauriane
 */
int myWrite(file* f, void* buffer, int nBytes) {
    if (f == NULL || buffer == NULL || nBytes <= 0) {
        return -1; // Erreur de paramètres
    }

    int bytes_written = 0;

    // Écrire dans les blocs de données liés au fichier
    while (nBytes > 0) {
        // Calculer la position actuelle dans le bloc de données
        int position_in_block = f->currentPosition % BLOCK_SIZE;

        // Trouver le bloc de données correspondant à la position actuelle
        DataBlock* current_block = NULL;
        for (int i = 0; i < super_file_data.num_inodes; ++i) {
        	if (super_file_data.inodes[i].name != NULL && strcmp(super_file_data.inodes[i].name, f->name) == 0) {
        		DataBlock* current_block = super_file_data.inodes[i].firstDataBlock;
        		break;
        	}
        }
        while (position_in_block >= BLOCK_SIZE) {
            if (current_block->next == NULL) {
                // Allouer un nouveau bloc si nécessaire
                current_block->next = (DataBlock*)malloc(sizeof(DataBlock));
                if (current_block->next == NULL) {
                    return bytes_written; // Retourner le nombre d'octets écrits jusqu'à présent
                }
                memset(current_block->next, 0, sizeof(DataBlock));
            }
            current_block = current_block->next;
            position_in_block -= BLOCK_SIZE;
        }

        // Écrire les données depuis le tampon vers le bloc de données en utilisant write
        int bytes_to_write = BLOCK_SIZE - position_in_block;
        if (bytes_to_write > nBytes) {
            bytes_to_write = nBytes;
        }
        
        int bytes_written_this_time = write(super_file_data.fileDescriptor, buffer, bytes_to_write); 
        if (bytes_written_this_time < 0) {
            return bytes_written_this_time; // Erreur lors de l'écriture
        }
        
        // Mettre à jour la position actuelle et le nombre d'octets écrits
        f->currentPosition += bytes_written_this_time;
        bytes_written += bytes_written_this_time;
        buffer += bytes_written_this_time;
        nBytes -= bytes_written_this_time;
    }

    // Mettre à jour la taille du fichier si nécessaire
    if (f->currentPosition > f->fileSize) {
        f->fileSize = f->currentPosition;
    }

    return bytes_written;
}
/**
 * @brief Déplace la position de lecture/écriture dans un fichier.
 * @param f Le pointeur vers la structure de fichier.
 * @param offset Le décalage par rapport à la position de départ.
 * @param base La base à utiliser pour le décalage (SEEK_SET, SEEK_CUR ou SEEK_END).
 * @return 0 en cas de succès, -1 en cas d'erreur.
 * @author Boyan
 */
void mySeek(file* f, int offset, int base) {
    if (f == NULL) {
        printf("Erreur : fichier NULL.\n");
        return;
    }

    off_t newPosition;

    switch (base) {
        case SEEK_SET:
            newPosition = offset;
            break;
        case SEEK_CUR:
            newPosition = f->currentPosition + offset;
            break;
        case SEEK_END:
            newPosition = f->fileSize + offset;
            break;
        default:
            printf("Erreur : base de déplacement incorrecte.\n");
            return;
    }

    // Vérifier si la nouvelle position est dans les limites du fichier
    if (newPosition < 0 || newPosition > f->fileSize) {
        printf("Erreur : déplacement en dehors des limites du fichier.\n");
        return;
    }

    // Utiliser lseek pour déplacer le pointeur de fichier à la nouvelle position
    off_t result = lseek(super_file_data.fileDescriptor, newPosition, SEEK_SET);
    if (result == -1) {
        perror("Erreur lors du déplacement du pointeur de fichier");
        return;
    }

}

/**
 * @brief Fonction pour lire depuis un fichier.
 * @param f Le pointeur vers la structure de fichier.
 * @param buffer Le tampon pour stocker les données lues.
 * @param nBytes Le nombre d'octets à lire.
 * @return Le nombre total d'octets lus, -1 en cas d'erreur.
 * @author Boyan
 */
nt myRead(file* f, void* buffer, int nBytes) {
    if (f == NULL || buffer == NULL || nBytes <= 0) {
        return -1; // Erreur : Paramètres invalides
    }

    int bytes_read = 0;

    // Recherche de l'inode correspondant au fichier
    inode* inode_of_file = NULL;
    for (int i = 0; i < super_file_data.num_inodes; ++i) {
        if (super_file_data.inodes[i].name != NULL && strcmp(super_file_data.inodes[i].name, f->name) == 0) {
            inode_of_file = &super_file_data.inodes[i];
            break;
        }
    }

    if (inode_of_file == NULL || inode_of_file->firstDataBlock == NULL) {
        // Gérer l'erreur : fichier non trouvé ou aucun bloc de données associé
        return -1;
    }

    // Positionner la tête de lecture au bon endroit dans la partition
    mySeek(f, 0, SEEK_SET);

    // Lire à partir des blocs de données liés à l'inode
    DataBlock* current_block = inode_of_file->firstDataBlock;
    while (current_block != NULL && nBytes > 0) {
        // Lire les données à partir du bloc de données
        int bytes_to_read = (nBytes > BLOCK_SIZE) ? BLOCK_SIZE : nBytes;
        int bytes_read_current = read(super_file_data.fileDescriptor, buffer, bytes_to_read);
        if (bytes_read_current == -1) {
            // Gérer l'erreur de lecture
            return -1;
        }
        bytes_read += bytes_read_current;
        nBytes -= bytes_read_current;
        buffer += bytes_read_current;

        // Passer au bloc de données suivant
        current_block = current_block->next;
    }

    return bytes_read;
}

/**
 * @brief Fonction pour afficher l'aide.
 * @author Boyan
 */
void printHelp() {
    printf("Utilisation :\n");
    printf("Choix 1 : Ouvre un fichier texte existant. : <nom_fichier.txt>\n");
    printf("Choix 2 : Ecrit des données dans un fichier texte spécifié. : <nom_fichier.txt> <donnees>\n");
    printf("Choix 3 : Lit les données depuis un fichier texte existant. : <nom_fichier.txt>\n");
    printf("Choix 4 : Supprime le fichier voulu\n");
    printf("Choix 5 : Affiche les fichiers existants\n");
}


/**
 * @brief Liste tous les fichiers présents dans la partition.
 * @return Un tableau de chaînes de caractères contenant les noms de fichiers, NULL en cas d'erreur.
 *         Les fichiers retournés sont alloués dynamiquement, et il est de la responsabilité de l'appelant
 *         de libérer la mémoire une fois qu'ils ne sont plus nécessaires en appelant freeFiles().
 * @author Lauriane
 */
char** listFiles() {
    // Nombre maximum de fichiers dans la partition
    int max_files = 100;
    char** files = (char**)malloc(max_files * sizeof(char*));
    if (files == NULL) {
        perror("Erreur lors de l'allocation de mémoire pour la liste des fichiers.");
        return NULL;
    }

    int num_files = 0;

    // Parcourir tous les inodes pour trouver les noms de fichiers
    for (int i = 0; i < super_file_data.num_inodes; ++i) {
        if (super_file_data.inodes[i].name != NULL) {
            // Allouer de la mémoire pour le nom de fichier
            files[num_files] = strdup(super_file_data.inodes[i].name);
            if (files[num_files] == NULL) {
                perror("Erreur lors de l'allocation de mémoire pour le nom de fichier.");
                // Libérer la mémoire allouée pour les noms de fichiers précédents
                for (int j = 0; j < num_files; ++j) {
                    free(files[j]);
                }
                free(files);
                return NULL;
            }
            num_files++;
        }
    }

    // Terminer la liste des noms de fichiers avec NULL
    files[num_files] = NULL;

    return files;
}

/**
 * @brief Compte le nombre de fichiers dans un tableau de chaînes de caractères.
 * @param files Le tableau de chaînes de caractères contenant les noms de fichiers.
 * @return Le nombre de fichiers dans le tableau.
 * @author Boyan
 */
int numFiles(char** files) {
    int count = 0; /**< Compteur du nombre de fichiers */
    
    // Parcours du tableau de chaînes de caractères jusqu'à ce que NULL soit rencontré
    while (files[count] != NULL) {
        count++; /**< Incrémente le compteur */
    }
    
    return count; /**< Retourne le nombre total de fichiers */
}

/**
 * @brief Fonction pour supprimer un fichier de la partition.
 * @param fileName Le nom du fichier à supprimer.
 * @return 0 si le fichier est supprimé avec succès, -1 en cas d'erreur.
 * @author Boyan
 */
int deleteFileFromPartition(char* fileName) {
    // Recherche de l'inode associé au nom de fichier donné
    for (int i = 0; i < super_file_data.num_inodes; ++i) {
        if (super_file_data.inodes[i].name != NULL && strcmp(super_file_data.inodes[i].name, fileName) == 0) {
            // Libérer la mémoire des données réelles stockées dans les blocs de données
            DataBlock* current_block = super_file_data.inodes[i].firstDataBlock;
	    while (current_block != NULL) {
		DataBlock* temp = malloc(sizeof(DataBlock)); // Allouer de la mémoire pour une nouvelle structure DataBlock
		if (temp == NULL) {
		    // Gérer l'erreur d'allocation de mémoire
	       	    return -1;
	        }
	        memcpy(temp, current_block, sizeof(DataBlock)); // Copier les données de current_block dans temp
		current_block = current_block->next;
	        for(int j = 0; j < BLOCK_SIZE; j++) {
		    temp->data[j] = '\0';
	        }
	        free(temp);       // Libérer le bloc de données lui-même
	    }

            // Marquer les blocs de données comme libres après les avoir libérés
            super_file_data.inodes[i].firstDataBlock = NULL;

            // Libérer la mémoire du nom de fichier dans l'inode
            if (super_file_data.inodes[i].name != NULL) {
                free(super_file_data.inodes[i].name);
                super_file_data.inodes[i].name = NULL;
            }

            // Libérer la mémoire du pointeur de fichier
            if (super_file_data.inodes[i].file_pointer != NULL) {
                free(super_file_data.inodes[i].file_pointer);
                super_file_data.inodes[i].file_pointer = NULL;
            }

            printf("Le fichier '%s' a été supprimé avec succès.\n", fileName);
            return 0; // Succès
        }
    }

    printf("Erreur : Le fichier '%s' n'a pas été trouvé dans la partition.\n", fileName);
    return -1; // Fichier non trouvé
}

/**
 * @brief Fonction pour supprimer un fichier.
 * @author Boyan
 */
void deleteFile() {
    // Affichage de la liste des fichiers
    printf("Liste des fichiers :\n");
    char** files = listFiles();
    if (files == NULL) {
        printf("Erreur lors de la récupération des noms de fichiers.\n");
        return;
    }
    for (int i = 0; files[i] != NULL; ++i) {
        printf("%d. %s\n", i + 1, files[i]);
    }

    // Demande à l'utilisateur de choisir le numéro du fichier à supprimer
    int choix;
    printf("Entrez le numéro du fichier à supprimer : ");
    scanf("%d", &choix);

    // Vérification de la validité du choix
    if (choix < 1 || choix > numFiles(files)) {
        printf("Numéro de fichier invalide.\n");
        return;
    }

    // Suppression du fichier correspondant au choix de l'utilisateur
    char* fileName = files[choix - 1];
    deleteFileFromPartition(fileName);

    // Libération de la mémoire allouée pour la liste des fichiers
    for (int i = 0; files[i] != NULL; ++i) {
        free(files[i]);
    }
    free(files);
}

/**
 * @brief Fonction pour supprimer entièrement la partition.
 * @param partitionName Le nom de la partition à supprimer.
 * @return 0 si la partition est supprimée avec succès, -1 en cas d'erreur.
 * @author Lauriane
 */
void deletePartition(char* partitionName) {
    // Fermer le descripteur de fichier de la partition
    if (close(super_file_data.fileDescriptor) == -1) {
        perror("Erreur lors de la fermeture du descripteur de fichier de la partition");
        return;
    }

    // Libérer les ressources allouées pour chaque fichier
    for (int i = 0; i < super_file_data.num_inodes; ++i) {
        if (super_file_data.inodes[i].name != NULL) {
            // Libérer la mémoire du nom de fichier dans l'inode
            free(super_file_data.inodes[i].name);
            // Libérer la mémoire du pointeur de fichier
            if (super_file_data.inodes[i].file_pointer != NULL) {
                free(super_file_data.inodes[i].file_pointer);
            }
        }
    }

    // Libérer les ressources allouées pour chaque bloc de données
    for (int i = 0; i < MAX_NUM_BLOCKS; ++i) {
        // Libérer les données réelles stockées dans les blocs de données
        DataBlock* current_block = super_file_data.blocks[i].next;
        while (current_block != NULL) {
            DataBlock* temp = current_block;
            current_block = current_block->next;
            free(temp);
        }
    }

    // Réinitialiser les informations de la partition
    super_file_data.num_inodes = 0;
    super_file_data.taille_partition = 0;
    super_file_data.fileDescriptor = -1;

    // Supprimer le fichier de partition
    if (remove(partitionName) == -1) {
        perror("Erreur lors de la suppression du fichier de partition");
        return;
    }

    printf("Partition '%s' supprimée avec succès.\n", partitionName);
}

/**
 * @brief Fonction principale du programme.
 * @return 0 si le programme s'exécute avec succès, 1 en cas d'erreur.
 * @author Boyan & Lauriane
 */
int main() {

    char* nom_partition = "ma_partition";
    if (myFormat(nom_partition) == -1) {
        printf("Erreur lors du formatage de la partition.\n");
        return 1;
    }
    char choix;

    // Affichage du menu tant que l'utilisateur ne choisit pas de quitter
    do {
	printf("\nMenu :\n");
        printf("1. Ouvrir un fichier \n");
        printf("2. Ecrire dans un fichier \n");
        printf("3. Lire depuis un fichier \n");
        printf("4. Supprime le fichier choisi\n");
        printf("5. Afficher les fichiers existants \n");
        printf("6. Afficher l'aide\n");
        printf("7. Quitter\n");
        printf("Entrez votre choix : ");

        // Lecture du choix de l'utilisateur
        scanf(" %[^\n]", &choix);

        // Traitement du choix de l'utilisateur
        switch (choix) {
	    case '1':
                // Appel à la fonction myOpen avec le nom du fichier
                char nom_fichier[100];
                printf("Entrez le nom du fichier à ouvrir : ");
                scanf("%s", nom_fichier);
                file* monFichier = myOpen(nom_fichier);
                if (monFichier == NULL) {
                    printf("Erreur lors de l'ouverture du fichier.\n");
                } else {
                    printf("Fichier '%s' ouvert avec succès.\n", nom_fichier);
                }
                break;
                
            case '2':
                // Appel à la fonction myWrite avec le nom du fichier et les données
                char nom_fichier_ecriture[100];
                char donnees_ecriture[1000];
                
                printf("Entrez le nom du fichier : ");
		scanf(" %[^\n]", nom_fichier_ecriture); // Lire jusqu'au saut de ligne
		
		printf("Entrez les données à écrire : ");
		scanf(" %[^\n]", donnees_ecriture); // Lire jusqu'au saut de ligne

                file* fichier_ecriture = myOpen(nom_fichier_ecriture);
                if (fichier_ecriture == NULL) {
                    printf("Erreur lors de l'ouverture du fichier.\n");
                    return ERROR_FILE_OPEN;
                } else {
                    int bytes_ecrits = myWrite(fichier_ecriture, donnees_ecriture, strlen(donnees_ecriture));
                    if (bytes_ecrits == -1) {
                        printf("Erreur lors de l'écriture dans le fichier.\n");
                    } else {
                        printf("Nombre total d'octets écrits : %d\n", bytes_ecrits);
                    }
                }
                break;
                
            case '3':
                    // Appel à la fonction myRead avec le nom du fichier
    		char nom_fichier_lecture[100];
    		char donnees_lecture[1000];
    		printf("Entrez le nom du fichier : ");
    		scanf(" %[^\n]", nom_fichier_lecture);
    		file* fichier_lecture = myOpen(nom_fichier_lecture);
   		if (fichier_lecture == NULL) {
        		printf("Erreur lors de l'ouverture du fichier.\n");
   		} else {
      			int bytes_lues = myRead(fichier_lecture, donnees_lecture, sizeof(donnees_lecture));
        		if (bytes_lues == -1) {
            			printf("Erreur lors de la lecture dans le fichier.\n");
        		} else {
            			// Afficher les données lues
            			printf("Données lues depuis le fichier :\n%s\n", donnees_lecture);
            			printf("Nombre total d'octets lus : %d\n", bytes_lues);
        		}
    		}
    		break;

            case '4':
            	//Appel à la fonction de suppression de fichier deleteFiles
            	deleteFile();	
		break;

            case '5':
                // Appel à la fonction listeFiles
    	        char** files = listFiles();
	        if (files == NULL) {
		    printf("Erreur lors de la récupération des noms de fichiers.\n");
		}
		    printf("Liste des fichiers :\n");
		for (int i = 0; files[i] != NULL; ++i) {
        		printf("%s\n", files[i]);
		}
            	break;
            	
            case '6':
                // Affichage de l'aide
                printHelp();
                break;

            case '7':
                // Sortie du programme  
                printf("Au revoir !\n");
                break;

            default:
                printf("Choix invalide. Veuillez réessayer.\n");
                break;
        }
    } while (choix != '7');
    
    deletePartition("ma_partition");
    
    return 0;
}

