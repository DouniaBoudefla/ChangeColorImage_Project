Pour les fichiers a.test.c, pngio_mod.c, pngio_mod.h :


       gcc -Wall NOMFICHIER.c pngio_mod.c -lpng -lm


Pour les fichiers dans le dossier SDL :
Je précise que pour les programmes dans le fichier SDL vous devez avoir dans votre répertoire une image en format png sous le nom "image.png" pour faire fonctionner les programmes

           gcc -Wall NOMFICHIER.c -lSDL -lSDL_image -lm 
