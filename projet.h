/**
 * @file projet.h
 * @brief Ce fichier contient les déclarations des fonctions et structures utilisées dans le projet.
 */

#ifndef PROJET_H_
#define PROJET_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/**
 * @def ERROR_FILE_OPEN
 * @brief Code d'erreur en cas d'échec d'ouverture de fichier.
 */
#define ERROR_FILE_OPEN -4

/**
 * @def BLOCK_SIZE
 * @brief Taille d'un bloc de données en octets.
 */
#define BLOCK_SIZE 512

/**
 * @def MAX_NUM_BLOCKS
 * @brief Nombre maximal de blocs de données dans le système de fichiers.
 */
#define MAX_NUM_BLOCKS 100

/**
 * @def NUM_INODES
 * @brief Nombre d'inodes dans le système de fichiers.
 */
#define NUM_INODES 16

/**
 * @def BLOCK_FREE
 * @brief Constante représentant un bloc de données libre.
 */
#define BLOCK_FREE 0

/**
 * @def BLOCK_OCCUPIED
 * @brief Constante représentant un bloc de données occupé.
 */
#define BLOCK_OCCUPIED 1

/**
 * @struct DataBlock
 * @brief Structure représentant un bloc de données.
 */
typedef struct DataBlock {
    char data[BLOCK_SIZE]; /**< Données stockées dans le bloc. */
    int occ_block; /**< État du bloc (libre ou occupé). */
    struct DataBlock* next; /**< Pointeur vers le bloc de données suivant. */
} DataBlock;

/**
 * @struct file
 * @brief Structure représentant un fichier.
 */
typedef struct {
    char* name; /**< Nom du fichier. */
    int fileSize; /**< Taille du fichier en octets. */
    int currentPosition; /**< Position actuelle dans le fichier. */
} file;

/**
 * @struct inode
 * @brief Structure représentant un inode.
 */
typedef struct {
    char* name; /**< Nom du fichier associé à l'inode. */
    file* file_pointer; /**< Pointeur vers la structure de fichier associée. */
    DataBlock* firstDataBlock; /**< Pointeur vers le premier bloc de données du fichier. */
} inode;

/**
 * @struct SuperFileData
 * @brief Structure représentant les données du super fichier.
 */
typedef struct {
    int num_inodes; /**< Nombre d'inodes dans le système de fichiers. */
    int taille_partition; /**< Taille de la partition. */
    inode inodes[NUM_INODES]; /**< Tableau des inodes. */
    int fileDescriptor; /**< Descripteur de fichier de la partition. */
    int currentPosition; /**< Position actuelle dans la partition. */
    DataBlock blocks[MAX_NUM_BLOCKS]; /**< Tableau des blocs de données. */
} SuperFileData;

/**
 * @brief Structure contenant les données du super fichier.
 * 
 * Contient des informations sur le système de fichiers en cours d'utilisation.
 */
SuperFileData super_file_data;

/**
 * @brief Fonction pour formater une partition.
 * 
 * @param partitionName Nom de la partition à formater.
 * @return 0 en cas de succès, -1 en cas d'erreur.
 * @author Lauriane
 */
int myFormat(char* partitionName);

/**
 * @brief Fonction pour ouvrir un fichier.
 * 
 * @param fileName Nom du fichier à ouvrir.
 * @return Pointeur vers la structure de fichier ou NULL en cas d'erreur.
 * @author Lauriane
 */
file* myOpen(char* fileName);

/**
 * @brief Fonction pour écrire dans un fichier.
 * 
 * @param f Pointeur vers la structure de fichier.
 * @param buffer Tampon contenant les données à écrire.
 * @param nBytes Nombre d'octets à écrire.
 * @return Nombre d'octets écrits en cas de succès, -1 en cas d'erreur.
 * @author Lauriane
 */
int myWrite(file* f, void* buffer, int nBytes);

/**
 * @brief Fonction pour lire depuis un fichier.
 * 
 * @param f Pointeur vers la structure de fichier.
 * @param buffer Tampon pour stocker les données lues.
 * @param nBytes Nombre d'octets à lire.
 * @return Nombre d'octets lus en cas de succès, -1 en cas d'erreur.
 * @author Boyan
 */
int myRead(file* f, void* buffer, int nBytes);

/**
 * @brief Fonction pour déplacer la position de lecture/écriture dans un fichier.
 * 
 * Cette fonction déplace la position de lecture/écriture dans un fichier ouvert.
 * 
 * @param f Pointeur vers la structure de fichier.
 * @param offset Décalage par rapport à la position de départ.
 * @param base La base à utiliser pour le décalage (SEEK_SET, SEEK_CUR ou SEEK_END).
 * @author Boyan
 */
void mySeek(file* f, int offset, int base);

/**
 * @brief Fonction pour afficher l'aide.
 * @author Boyan
 */
void printHelp();

/**
 * @brief Fonction pour supprimer un fichier.
 * @author Boyan
 */
void deleteFile();

/**
 * @brief Fonction pour supprimer un fichier de la partition.
 * 
 * @param fileName Le nom du fichier à supprimer.
 * @return 0 si le fichier est supprimé avec succès, -1 en cas d'erreur.
 * @author Boyan
 */
int deleteFileFromPartition(char* fileName);

/**
 * @brief Fonction pour lister tous les fichiers présents dans la partition.
 * 
 * Cette fonction parcourt tous les inodes du système de fichiers pour extraire les noms
 * des fichiers présents.
 * 
 * @return Un tableau de chaînes de caractères contenant les noms de fichiers, NULL en cas d'erreur.
 *         Les fichiers retournés sont alloués dynamiquement, et il est de la responsabilité de l'appelant
 *         de libérer la mémoire une fois qu'ils ne sont plus nécessaires en appelant free() sur chaque
 *         élément du tableau, puis sur le tableau lui-même.
 * @author Lauriane
 */
char** listFiles();

/**
 * @brief Fonction pour obtenir le nombre de fichiers présents dans la partition.
 * 
 * @param files Le tableau de chaînes de caractères retourné par la fonction listFiles().
 * @return Le nombre de fichiers présents dans la partition.
 * @author Boyan
 */
int numFiles(char** files);

/**
 * @brief Fonction pour supprimer la partition lorsque l'utilisateur quitte le programme
 * 
 * @param partitionName le nom de la partition
 * @author Boyan
 */
void deletePartition(char* partitionName);

#endif /* PROJET_H_ */

