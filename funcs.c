#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "funcs.h"

char *str_to_lower (char *s) {
    for (int i=0; i<strlen(s); ++i) s[i] = tolower(s[i]);

    return s;
}

void ShowLStr (LStr *l) {
    while ((*l) != NULL && (*l)->prox != NULL) {
        printf ("%s -> ", (*l)->nome);
        l = &((*l)->prox);
    }
    if ((*l) != NULL) printf ("%s\n", (*l)->nome);
}

void acrescentaLStr (LStr *l, char *s) {
    while ((*l) != NULL) l = &((*l)->prox);

    (*l) = malloc (sizeof (struct slist));
    (*l)->nome = strdup (s);
    (*l)->prox = NULL;
}

void ShowProj (LProj *p) {
    while ((*p) != NULL) {
        printf ("Chave: %s\n", (*p)->chave);
        printf ("Título: %s\n", (*p)->titulo);
        printf ("Autores: ");
        ShowLStr (&((*p)->autores));
        printf ("\n");
        p = &((*p)->prox);
    }
}

void copiaLStr (LStr *source, LStr *dest) {
    *dest = NULL;
    while (*source != NULL) {
        (*dest) = malloc (sizeof (struct slist));
        (*dest)->nome = (*source)->nome;
        (*dest)->prox = NULL;
        source = &((*source)->prox);
        dest = &((*dest)->prox);
    }
    *dest = NULL;
}

void acrescentaProj (LProj *p, char *chave, char *titulo, LStr autores) {
    while (*p != NULL) p = &((*p)->prox);

    (*p) = malloc (sizeof (struct projeto));
    (*p)->chave = strdup (chave);
    (*p)->titulo = strdup (titulo);
    (*p)->autores = autores;
    (*p)->prox = NULL;
}

void copiaLProj (LProj *source, LProj *dest) {
    *dest = NULL;
    while (*source != NULL) {
        *dest = malloc (sizeof (struct projeto));
        (*dest)->chave = (*source)->chave;
        (*dest)->titulo = (*source)->titulo;
        copiaLStr (&((*source)->autores), &((*dest)->autores));
        (*dest)->prox = NULL;
        source = &((*source)->prox);
        dest = &((*dest)->prox);
    }
    *dest = NULL;
}

void acrescentaCat (LCat *l, char *s, LProj p) {
    while (*l != NULL && strcmp((*l)->nome, s) < 0)
        l = &((*l)->prox);
    
    if (*l == NULL) {
        *l = malloc (sizeof (struct categoria));
        (*l)->nome = strdup (s);
        (*l)->num_ocorr = 1;
        copiaLProj (&p, &((*l)->projeto));
        (*l)->prox = NULL;
    }
    else if (strcmp ((*l)->nome, s) == 0) {
        (*l)->num_ocorr++;
        copiaLProj (&p, &((*l)->projeto));
    }
    else {
        LCat new = malloc (sizeof (struct categoria));
        new->nome = strdup (s);
        new->num_ocorr = 1;
        copiaLProj (&p, &(new->projeto));
        new->prox = *l;
        *l = new;
    }
}

/*
int main () {
    LCat categoria = NULL;
    LProj proj = NULL;
    LStr nomes = NULL;

    acrescentaLStr (&nomes, "hello");
    acrescentaLStr (&nomes, "bye");
    acrescentaProj (&proj, "Zé", "Abílio420", nomes);
    nomes = NULL;
    acrescentaCat (&categoria, "inproceeding", proj);
    proj = NULL;
    ShowProj (&(categoria->projeto));
    ShowProj (&proj); 

    return 0;
}*/