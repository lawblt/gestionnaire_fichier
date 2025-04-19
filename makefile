# Nom de l'exécutable
TARGET = projet

# Compilateur
CC = gcc

# Options de compilation
CFLAGS = -Wall -Wextra -Werror

# Liste des fichiers source
SRCS = projet.c

# Liste des fichiers objet générés à partir des fichiers source
OBJS = $(SRCS:.c=.o)

# Commande pour générer l'exécutable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

# Commande pour générer les fichiers objet
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Commande pour générer la documentation avec Doxygen
doc:
	doxygen config_file

# Commande pour nettoyer les fichiers générés
clean:
	rm -f $(OBJS) $(TARGET) -r html latex

