/**
 * \file WinMover.c
 *
 * Programme donnant les coordonnées voulues aux fenêtres d'une classe
 * donnée, sous un système de type Windows NT.
 */

#define _WIN32_WINNT  0x0400
		/* programme conçu pour Windows NT 4 ou ultérieur */


#include <assert.h>
#include <stdio.h>

#include <windows.h>


/*========================================================================*/
/*                               CONSTANTES                               */
/*========================================================================*/

/**
 * \def VERSION_PRG
 *
 * Version courante du programme WinMouse.
 */
#define VERSION_PRG  "0.7.7 du 18/09/2023"

/**
 * \def TAILLE_MAX_MSG
 *
 * Taille maximale d'un message affiché par ce programme.
 */
#define TAILLE_MAX_MSG  4096U


/* == Messages affichés == */

static const char* const TITRE_PRG = "WinMover" ;
static const char* const FMT_MSG_A_PROPOS =
		"%s version %s.\n" ;
static const char* const MSG_FENETRE_DEPLACEE =
		" - fenetre [0x%08x] deplacee avec succes.\n" ;
static const char* const MSG_NB_FEN_REDIM =
		"%u fenetre(s) deplacee(s).\n" ;
static const char* const MSG_ERR_FMT_ECHEC_FNCT =
		"Echec de la fonction %s !" ;
static const char* const MSG_ERR_SYS =
		"\n Message d'erreur systeme : " ;


/* == Valeurs numériques == */

/* coordonnées à donner à la fenêtre indiquée */
static const RECT COORDS_FEN_DEFAUT = {
		.left = 41L,
		.top = 0L,
		.bottom = 687L,
		.right = 1287L
};


/* == Constantes chaînes de caractères à ne pas traduire ! == */

/* classe de fenêtre à rechercher */
static const WCHAR* const CLASSE_FEN_EXPLOR_DEFAUT = L"CabinetWClass" ;

/* nom du fichier INI de configuration de ce programme */
static const WCHAR* const NOM_FICHIER_CONFIG = L"WinMover.ini" ;
/* nom de section contenant la classe de fenêtre à rechercher */
static const WCHAR* const NOM_SECTION_CLASSE = L"Win_Class" ;
/* nom de la valeur pour le nom de classe */
static const WCHAR* const NOM_VAL_CLASSE = L"Name" ;
/* nom de section contenant les coordonnées à donner aux fenêtres */
static const WCHAR* const NOM_SECTION_COORDS = L"Win_Rect" ;
/* nom de la valeur pour la gauche de la fenêtre */
static const WCHAR* const NOM_VAL_GAUCHE = L"Left" ;
/* nom de la valeur pour le haut de la fenêtre */
static const WCHAR* const NOM_VAL_HAUT = L"Top" ;
/* nom de la valeur pour le bas de la fenêtre */
static const WCHAR* const NOM_VAL_BAS = L"Bottom" ;
/* nom de la valeur pour le haut de la fenêtre */
static const WCHAR* const NOM_VAL_DROITE = L"Right" ;


/*========================================================================*/
/*                           VARIABLES GLOBALES                           */
/*========================================================================*/

/* classe de fenêtre à rechercher */
static WCHAR classe_fenetre[TAILLE_MAX_MSG] ;
/* coordonnées à donner aux fenêtres trouvées */
static RECT coords_fen ;

/* nombre de fenêtre redimensionnées */
static unsigned int nbFenRedim ;


/*========================================================================*/
/*                               FONCTIONS                                */
/*========================================================================*/

/* === FONCTIONS UTILITAIRES "PRIVEES" === */

/* met en forme et affiche un message d'erreur du système NT
   sur la sortie console d'erreur standard */
static DWORD
MsgErreurSys (const char* nomFnct)
{
	DWORD codeErr = GetLastError () ;
	char msgErr[TAILLE_MAX_MSG] ;
	snprintf (msgErr, TAILLE_MAX_MSG,
	          MSG_ERR_FMT_ECHEC_FNCT,
	          nomFnct, codeErr) ;
	/* essaie d'obtenir une description de l'erreur en question */
	LPSTR ptrMsgSys;
	DWORD res = FormatMessageA (FORMAT_MESSAGE_ALLOCATE_BUFFER
	                             | FORMAT_MESSAGE_FROM_SYSTEM,
	                            NULL,
	                            codeErr,
	                            MAKELANGID(LANG_SYSTEM_DEFAULT,
	                                       SUBLANG_SYS_DEFAULT),
	                            (LPSTR)&ptrMsgSys,
	                            0,
	                            NULL) ;
	if (res != 0) {
		strcat (msgErr, MSG_ERR_SYS) ;
		strncat (msgErr, ptrMsgSys,
		         TAILLE_MAX_MSG - strlen(msgErr)) ;
		LocalFree (ptrMsgSys) ;
	}
	/* afficher le message d'erreur ainsi construit */
	fprintf (stderr, msgErr) ;
	fflush (stdout) ;
	/* renvoie le code d'erreur système utilisé */
	return codeErr ;
}


/* détermine les paramètres de fonctionnement du programme */
static void
DetermineParams (void)
{
	/* retrouve le chemin vers le fichier INI à lire */
	WCHAR chemin_ini[MAX_PATH] ;
	memset (chemin_ini, 0, MAX_PATH) ;
	DWORD len = GetCurrentDirectoryW (MAX_PATH, chemin_ini) ;
	wcsncat (chemin_ini, L"\\", MAX_PATH - len - 1) ;
	wcsncat (chemin_ini, NOM_FICHIER_CONFIG, MAX_PATH - len - 2) ;
	/* retrouve le nom de la classe à rechercher */
	GetPrivateProfileStringW (NOM_SECTION_CLASSE,
	                          NOM_VAL_CLASSE,
	                          CLASSE_FEN_EXPLOR_DEFAUT,
	                          classe_fenetre,
	                          TAILLE_MAX_MSG,
	                          chemin_ini) ;
	/* détermine les coordonnées à donner aux fenêtres trouvées :
	   lit les valeurs voulues dans le fichier INI une à une,
	   en prenant les valeurs par défaut le cas échéant */
	coords_fen.left = (LONG) GetPrivateProfileIntW (NOM_SECTION_COORDS,
	                                                NOM_VAL_GAUCHE,
	                                                COORDS_FEN_DEFAUT.left,
	                                                chemin_ini);
	coords_fen.top = (LONG) GetPrivateProfileIntW (NOM_SECTION_COORDS,
	                                               NOM_VAL_HAUT,
	                                               COORDS_FEN_DEFAUT.top,
	                                               chemin_ini);
	coords_fen.bottom = (LONG) GetPrivateProfileIntW (NOM_SECTION_COORDS,
	                                                  NOM_VAL_BAS,
	                                                  COORDS_FEN_DEFAUT.bottom,
	                                                  chemin_ini);
	coords_fen.right = (LONG) GetPrivateProfileIntW (NOM_SECTION_COORDS,
	                                                 NOM_VAL_DROITE,
	                                                 COORDS_FEN_DEFAUT.right,
	                                                 chemin_ini);
}


/* redimensionne la fenêtre indiquée */
static BOOL
RedimFenetre (HWND hWndCible)
{
	BOOL ok = MoveWindow (hWndCible,
	                      coords_fen.left,
	                      coords_fen.top,
	                      coords_fen.right - coords_fen.left,
	                      coords_fen.bottom - coords_fen.top,
	                      TRUE) ;
	if (!ok) {
		MsgErreurSys ("MoveWindow()") ;
		return FALSE ;
	}
	fprintf (stdout, MSG_FENETRE_DEPLACEE, hWndCible) ;
	fflush (stdout) ;
	ok = SetForegroundWindow (hWndCible) ;
	if (!ok) {
		MsgErreurSys ("SetForegroundWindow()") ;
		/* erreur non fatale (avertissement) */
	}
	return TRUE ;
}


/* === FONCTIONS "CALLBACK" === */

/* fonction appelée par EnumWindows() pour chaque fenêtre trouvée ;
   vérifie la classe de la fenêtre, et agit de façon adéquate */
BOOL CALLBACK
MainWindowEnumerated (HWND hWnd,
                      LPARAM lParam)
{
	/* paramètre inutilisé */
	(void)lParam ;

	/* retrouve la classe de la fenêtre */
	WCHAR strCls[TAILLE_MAX_MSG] ;
	int len = GetClassNameW (hWnd,
	                         strCls,
	                         (int)TAILLE_MAX_MSG) ;

	/* s'agit-il de la classe recherchée ? */
	if (wcsncmp (strCls, classe_fenetre, (size_t)len) == 0) {
		RedimFenetre (hWnd) ;
		nbFenRedim++ ;
	}

	/* fenêtre suivante */
	return TRUE ;
}


/* === POINT D'ENTREE === */

int WINAPI
main (int argc,
      char* argv[])
{
	/* la ligne de commande n'est pas prise en compte */
	(void)argc ;   (void)argv ;

	/* affiche "l'entête" du programme */
	fprintf (stdout, FMT_MSG_A_PROPOS, TITRE_PRG, VERSION_PRG) ;
	fflush (stdout) ;

	/* retrouve les valeurs à donner aux paramètres */
	DetermineParams () ;

	/* énumère les fenêtres principales affichées,
	   pour trouver celles à traiter (selon leur classe) */
	BOOL ok = EnumWindows (MainWindowEnumerated, 0L) ;
	if (!ok) {
		MsgErreurSys ("EnumWindows()") ;
		return 1 ;
	}

	/* OK, programme terminé */
	fprintf (stdout, MSG_NB_FEN_REDIM, nbFenRedim ) ;
	fflush (stdout) ;
	return 0 ;
}

