#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define LONG 12
#define LARG 12
#define TAILLE 12

const char MUR = '#';
const char CAISSE = '$';
const char CIBLE = '.';
const char SOKOBAN = '@';
const char CAISSE_CIBLE = '*';
const char SOKOBAN_CIBLE = '+';
const char CASE_VIDE = ' ';

typedef char t_Plateau[LONG][LARG];

int kbhit();
void chargerPartie(t_Plateau plateau, char fichier[50]);
void enregistrerPartie(t_Plateau plateau, char fichier[50]);
void afficherEntete(char fichier[50], int deplacements);
void afficherPlateau(t_Plateau plateau);
void trouverSokoban(t_Plateau plateau, int *ligsok, int *colsok);
bool verifTouche(t_Plateau plateau, int ligsok, int colsok, char fichier[50], int *deplacements);
char verifCaseSuivante(t_Plateau plateau, int ligsok, int colsok, int ligsuiv, int colsuiv);
bool deplacer(t_Plateau plateau, int ligsok, int colsok, int ligsuiv, int colsuiv, char fichier[50], int deplacements);
bool gagne(t_Plateau plateau);


int kbhit(){
	// la fonction retourne :
	// 1 si un caractere est present
	// 0 si pas de caractere présent
	int unCaractere=0;
	struct termios oldt, newt;
	int ch;
	int oldf;

	// mettre le terminal en mode non bloquant
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
	ch = getchar();

	// restaurer le mode du terminal
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
 
	if(ch != EOF){
		ungetc(ch, stdin);
		unCaractere=1;
	} 
	return unCaractere;
}

void chargerPartie(t_Plateau plateau, char fichier[50]){
    FILE * f;
    char finDeLigne;

    f = fopen(fichier, "r");
    if (f==NULL){
        printf("ERREUR SUR FICHIER");
        exit(EXIT_FAILURE);
    } else {
        for (int ligne=0 ; ligne<TAILLE ; ligne++){
            for (int colonne=0 ; colonne<TAILLE ; colonne++){
                fread(&plateau[ligne][colonne], sizeof(char), 1, f);
            }
            fread(&finDeLigne, sizeof(char), 1, f);
        }
        fclose(f);
    }
}

void enregistrerPartie(t_Plateau plateau, char fichier[50]){
    FILE * f;
    char finDeLigne = '\n';

    f = fopen(fichier, "w");

    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            fwrite(&plateau[i][j], sizeof(char), 1, f);
        }
        fwrite(&finDeLigne, sizeof(char), 1, f);
    }

    fclose(f);
}

//Fonction qui affiche le plateau et qui remplace les états intermédiaire '+' et '*' pour ne pas les afficher
void afficherPlateau(t_Plateau plateau){
    for (int i = 0; i < LARG; i++) {
        printf("\n");
        for (int j = 0; j < LONG; j++) {

            char caseTab = plateau[i][j];

            if (caseTab == SOKOBAN_CIBLE){
                printf("%c", SOKOBAN);
            }
            else if(caseTab == CAISSE_CIBLE){
                printf("%c", CAISSE);
            }
            else{
                printf("%c", caseTab);
            }
        }
    }
}

//Fonction qui affiche l'entete et le nombre de déplacement effectués
void afficherEntete(char fichier[50], int deplacements){
    printf("%s\n", fichier);
    printf("Q = gauche | Z = haut | S= bas | D = droite\n");
    printf("X = quitter | R = recommencer\n");
    printf("Déplacements : %d\n", deplacements);
}

//Fonction permettant d'avoir les coordonnées du joueur
void trouverSokoban(t_Plateau plateau, int *ligsok, int *colsok){
    for (int i = 0; i < LARG; i++){
        for (int j = 0; j < LONG; j++){
            if (plateau[i][j] == SOKOBAN || plateau[i][j] == SOKOBAN_CIBLE){
                *ligsok = i;
                *colsok = j;
            }
        }
    }
}

//Fonction qui vérifie la case suivante qui renvoie les caractères de la case suivante ou celle d'après si il y à une caisse
char verifCaseSuivante(t_Plateau plateau, int ligsok, int colsok, int ligsuiv, int colsuiv){

    char caseSuiv = plateau[ligsuiv][colsuiv];

    if (caseSuiv == CASE_VIDE || caseSuiv == CIBLE ){
        return CASE_VIDE; 
    }

    if (caseSuiv == CAISSE || caseSuiv == CAISSE_CIBLE){
        int ligSuiv2 = ligsuiv + (ligsuiv - ligsok);
        int colSuiv2 = colsuiv + (colsuiv - colsok);

        char caseSuiv2 = plateau[ligSuiv2][colSuiv2];

        if (caseSuiv2 == CASE_VIDE || caseSuiv2 == CIBLE)
            return CAISSE;
    }

    return MUR;
}

//Booléen qui va déplacer le Sokoban et qui renvoie true si déplacement est possible pour l'ajouter au nombre de déplacement
bool deplacer(t_Plateau plateau, int ligsok, int colsok, int ligsuiv, int colsuiv, char fichier[50], int deplacements){

    char resultat = verifCaseSuivante(plateau, ligsok, colsok, ligsuiv, colsuiv);
    if (resultat == MUR) return false;

    char caseActuelle = plateau[ligsok][colsok];
    char caseSuiv = plateau[ligsuiv][colsuiv];

    if (caseActuelle == SOKOBAN_CIBLE)
        plateau[ligsok][colsok] = CIBLE;
    else
        plateau[ligsok][colsok] = CASE_VIDE;

    if (resultat == CAISSE){
        int ligsuiv2 = ligsuiv + (ligsuiv - ligsok);
        int colsuiv2 = colsuiv + (colsuiv - colsok);
        char caseSuiv2 = plateau[ligsuiv2][colsuiv2];

        if (caseSuiv2 == CIBLE)
            plateau[ligsuiv2][colsuiv2] = CAISSE_CIBLE;
        else
            plateau[ligsuiv2][colsuiv2] = CAISSE;

        if (caseSuiv == CIBLE || caseSuiv == CAISSE_CIBLE)
            plateau[ligsuiv][colsuiv] = SOKOBAN_CIBLE;
        else
            plateau[ligsuiv][colsuiv] = SOKOBAN;
    }

    else if (resultat == CASE_VIDE){
        if (caseSuiv == CIBLE)
            plateau[ligsuiv][colsuiv] = SOKOBAN_CIBLE;
        else
            plateau[ligsuiv][colsuiv] = SOKOBAN;
    }

    system("clear");
    afficherEntete(fichier, deplacements);
    afficherPlateau(plateau);

    return true;
}

//Booléen qui renvoie true si il n'y à plus aucune caisse sur le plateau (= il n'y à que des caisses sur cibles)
bool gagne(t_Plateau plateau){
    for (int i = 0; i < TAILLE; i++){
        for (int j = 0; j < TAILLE; j++){
            if (plateau[i][j] == CAISSE)
                return false;
        }
    }
    return true;
}

//Booléen qui vérifie les touches appuyées pour les mettres dans la fonction deplacer qui va faire le dééplacement, et renvoie true pour mettre fin à la partie dans le cas 'x'
bool verifTouche(t_Plateau plateau, int ligsok, int colsok, char fichier[50], int *deplacements){ 

    int ligsuiv = 0, colsuiv = 0;
    char touche = '\0';

    if (kbhit()){
        touche = getchar();
    } 

    switch(touche){

        case 'x': {
            char reponse;
            printf("\nVoulez-vous sauvegarder la partie avant de quitter ? (o/n) : ");
            scanf("%c", &reponse);

            if (reponse == 'o') {
                char fichierSauvegarde[50];
                printf("Nom du fichier de sauvegarde (.sok) : ");
                scanf("%s", fichierSauvegarde);
                enregistrerPartie(plateau, fichierSauvegarde);
                printf("Partie sauvegardée dans '%s'.\n", fichierSauvegarde);
            }
            return true;
        }

        case 'r': {
            char confirmation;
            printf("\nRecommencer la partie depuis le début ? (o/n) : ");
            scanf("%c", &confirmation);

            if (confirmation == 'o') {
                chargerPartie(plateau, fichier);
                system("clear");
                afficherEntete(fichier, *deplacements);
                afficherPlateau(plateau);
            }
            return false;
        }

        case 'z':
            ligsuiv = ligsok-1;
            colsuiv = colsok;
            if (deplacer(plateau, ligsok, colsok, ligsuiv, colsuiv, fichier, *deplacements)){
                (*deplacements)++;
            }
            return false;

        case 's':
            ligsuiv = ligsok+1;
            colsuiv = colsok;
            if (deplacer(plateau, ligsok, colsok, ligsuiv, colsuiv, fichier, *deplacements)){
                (*deplacements)++;
            }
            return false;

        case 'q':
            ligsuiv = ligsok;
            colsuiv = colsok-1;
            if (deplacer(plateau, ligsok, colsok, ligsuiv, colsuiv, fichier, *deplacements)){
                (*deplacements)++;
            }
            return false;

        case 'd':
            ligsuiv = ligsok;
            colsuiv = colsok+1;
            if (deplacer(plateau, ligsok, colsok, ligsuiv, colsuiv, fichier, *deplacements)){
                (*deplacements)++;
            }
            return false;

        default:
            return false;
    }
}

int main(){

    t_Plateau plateau;
    char fichier[50];
    bool abandon = false;
    int deplacements = 0; 

    printf("Nom du fichier .sok : ");
    scanf("%s", fichier);

    chargerPartie(plateau, fichier);

    system("clear");
    afficherEntete(fichier, deplacements);
    afficherPlateau(plateau);

    while (!abandon && !gagne(plateau)) {

        int ligsok, colsok;
        trouverSokoban(plateau, &ligsok, &colsok);
        abandon = verifTouche(plateau, ligsok, colsok, fichier, &deplacements);
    }

    if (gagne(plateau)) {
        printf("\nPartie gagnée\n");
    } else {
        printf("\nPartie abandonnée\n");
    }

    return 0;
}