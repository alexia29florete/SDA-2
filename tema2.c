/* FLORETE Alexia-Maria - 315CC */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Huffnode
{
    int prioritate;
    char *nume; //daca declaram char nume[16] puteam ajunge in situatii de buffer overflow la anumite teste
    struct Huffnode *left, *right;
} Huffnode;

typedef struct Heap
{
    int size;
    Huffnode **node; //salvez nodurile initiale
} Heap;

typedef struct Stiva
{
    Huffnode *satelit; //salvez nodul curent
    int nivel; //adancimea nodului curent
    char cod_binar[1001]; //reprezentarea pana la nodul curent
} Stiva;

Huffnode *create_node(char *name, int valoare)
{
    Huffnode *new_node = (Huffnode *)malloc(sizeof(Huffnode));
    new_node->prioritate = valoare;
    new_node->nume = (char *)malloc(strlen(name) + 1);
    strcpy(new_node->nume, name);
    new_node->left = new_node->right = NULL;
    return new_node;
}

void swap(Huffnode **a, Huffnode **b)
{
    Huffnode *temp = *a;
    *a = *b;
    *b = temp;
}

void heapify_up(Heap *heap, int index)
{
    while(index > 0)
    {
        int index_parinte = (index - 1) / 2;
        if((heap->node[index_parinte]->prioritate > heap->node[index]->prioritate )||(heap->node[index_parinte]->prioritate == heap->node[index]->prioritate && strcmp(heap->node[index_parinte]->nume, heap->node[index]->nume) > 0))
        {
            swap(&heap->node[index_parinte], &heap->node[index]);
            index = index_parinte;
        }
        else 
        {
            break;
        }
    }
}

void insert(Heap *heap, Huffnode *node)
{
    heap->node[heap->size] = node; //adaug noul element ultimul
    heapify_up(heap, heap->size); //il adaug pe pozitia corecta
    heap->size++; //contorizez dimensiunea
}

void heapify_down(Heap *heap, int index)
{
    int parinte = index;
    int index_st = 2 * index + 1;
    int index_dr = 2 * index + 2;

    if(index_st < heap->size && (heap->node[index_st]->prioritate < heap->node[parinte]->prioritate || (heap->node[index_st]->prioritate == heap->node[parinte]->prioritate && strcmp(heap->node[index_st]->nume, heap->node[parinte]->nume) < 0)))
    {
        parinte = index_st;
    }
    if(index_dr < heap->size && (heap->node[index_dr]->prioritate < heap->node[parinte]->prioritate || (heap->node[index_dr]->prioritate == heap->node[parinte]->prioritate && strcmp(heap->node[index_dr]->nume, heap->node[parinte]->nume) < 0)))
    {
        parinte = index_dr;
    }
    if(parinte != index)
    {
        swap(&heap->node[parinte], &heap->node[index]);
        heapify_down(heap, parinte);
    }
}

Huffnode *heap_extract_min(Heap *heap)
{
    if(heap->size == 0)
    {
        return NULL;
    }
    Huffnode *minim = heap->node[0]; //retin minimul care este radacina
    heap->node[0] = heap->node[heap->size - 1]; //salvez ultimul element in loc de radacina
    heap->size--;
    heapify_down(heap, 0);
    return minim;
}

Huffnode *construire_heap(Heap *heap)
{
    while(heap->size > 1)
    {
        //extrag cele mai mici doua noduri
        Huffnode *st = heap_extract_min(heap);
        Huffnode *dr = heap_extract_min(heap);
        //formez numele pentru satelitul parinte prin concatenarea celui din stanga cu celui din dreapta
        int lungime_nume = strlen(st->nume) + strlen(dr->nume) + 1;
        //daca declaram char nume_satelit_parinte[32], daca se depasea limita precizata, atunci apareau caractere neprintabile
        char *nume_satelit_parinte = (char *)malloc(lungime_nume);
        strcpy(nume_satelit_parinte, st->nume);
        strcat(nume_satelit_parinte, dr->nume);
        //creez nodul parinte si ii realizez legaturile
        Huffnode *parinte = create_node(nume_satelit_parinte, st->prioritate + dr->prioritate);
        free(nume_satelit_parinte);
        parinte->left = st;
        parinte->right = dr;
        //il inserez in min_heap
        insert(heap, parinte);
    }
    return heap_extract_min(heap);
}

//calculez radacina arborelui
int height(Huffnode *nod)
{
    if(nod == NULL)
    {
        return 0;
    }
    int inaltime_stanga = height(nod->left);
    int inaltime_dreapta = height(nod->right);
    if(inaltime_stanga > inaltime_dreapta)
    {
        return 1 + inaltime_stanga;
    }
    else
    {
        return 1 + inaltime_dreapta;
    }
}

Huffnode *decode_satelit(Huffnode *nod, char *codificare, int *index)
{
    //frunza => returnez codul gasit
    if(nod->left == NULL && nod->right == NULL)
    {
        return nod;
    }

    int i = *index;
    if(codificare[i] == '\0')
    {
        return NULL;
    }

    //daca bitul este zero, ma duc in subarborele stang
    if(codificare[i] == '0')
    {
        i++;
        *index = i;
        return decode_satelit(nod->left, codificare, index);
    }

    //daca bitul este zero, ma duc in subarborele stang
    if(codificare[i] == '1')
    {
        i++;
        *index = i;
        return decode_satelit(nod->right, codificare, index);
    }

    return NULL;
}

//decodeaza toti satelitii din binar
void identificare_sateliti(Huffnode *root, char *codificare, FILE *fout)
{ 
    int index = 0;
    int lungime = strlen(codificare);

    while(index < lungime)
    {
        Huffnode *node = decode_satelit(root, codificare, &index);
        if(node)
        {
            fprintf(fout, "%s ", node->nume);
        }
        else
        {
            break;
        }
    }
    fprintf(fout, "\n");
}

int codificare_sateliti(Huffnode *root, char *satelit_cautat, char *codificare, int nivel)
{
    Stiva stiva[1000];
    int top = -1;

    //pornesc de la radacina si initializez stiva
    top++;
    stiva[top].satelit = root;
    stiva[top].nivel = 0;
    stiva[top].cod_binar[0] = '\0';

    while(top >= 0)
    {
        Stiva satelit_curent = stiva[top]; //extrag elementul din varf
        top--;

        if(satelit_curent.satelit == NULL)
        {
            continue; //il ignor
        }

        //daca este frunza si am gasit satelitul cautat
        if(satelit_curent.satelit->left == NULL && satelit_curent.satelit->right == NULL)
        {
            if(strcmp(satelit_curent.satelit->nume, satelit_cautat) == 0)
            {
                strcpy(codificare, satelit_curent.cod_binar);
                return 1;
            }
        }
        else 
        {
            //caut copilul drept
            if(satelit_curent.satelit->right != NULL)
            {
                //adaug pe stiva partea din stanga
                top++;
                stiva[top].satelit = satelit_curent.satelit->right;
                stiva[top].nivel = satelit_curent.nivel + 1;
                strcpy(stiva[top].cod_binar, satelit_curent.cod_binar);
                stiva[top].cod_binar[satelit_curent.nivel] = '1';
                stiva[top].cod_binar[satelit_curent.nivel + 1] = '\0';
            }

            //copilul stang
            if(satelit_curent.satelit->left != NULL)
            {
                //adaug pe stiva partea din dreapta
                top++;
                stiva[top].satelit = satelit_curent.satelit->left;
                stiva[top].nivel = satelit_curent.nivel + 1;
                strcpy(stiva[top].cod_binar, satelit_curent.cod_binar);
                stiva[top].cod_binar[satelit_curent.nivel] = '0';
                stiva[top].cod_binar[satelit_curent.nivel + 1] = '\0';
            }
        }
    }
    return 0;
}

//am aplicat algortimul LCA pentru n noduri, nu doar 2 asa cum era in algortimul clasic
Huffnode *LCA_mai_multi_sateliti(Huffnode *root, char nume_sateliti[][16], int numar_sateliti)
{
    if(root == NULL)
    {
        return NULL;
    }

    //daca e frunza, verific daca am vreun satelit al carui nume ma intereseaza
    if(root->left == NULL && root->right == NULL)
    {
        for(int i = 0; i < numar_sateliti; i++)
        {
            if(strcmp(root->nume, nume_sateliti[i]) == 0)
            {
                return root;
            }
        }
    }

    Huffnode *st_lca = LCA_mai_multi_sateliti(root->left, nume_sateliti, numar_sateliti);
    Huffnode *dr_lca = LCA_mai_multi_sateliti(root->right, nume_sateliti, numar_sateliti);

    //daca am elemente si in stanga si in dreapta => lowest common ancestor e nodul curent
    if(st_lca != NULL && dr_lca != NULL)
    {
        return root;
    }
    
    //daca am elemnete doar in stanga 
    if(st_lca != NULL)
    {
        return st_lca;
    }

    //daca am elemente doar in dreapta
    if(dr_lca != NULL)
    {
        return dr_lca;
    }
}

//printez pentru un anumit nivel specificat
void print_nivel(Huffnode *root, int nivel, FILE *fout)
{
    if(root == NULL)
    {
        return;
    }
    if(nivel == 1)
    {
        fprintf(fout, "%d-%s ", root->prioritate, root->nume);
    }
    else if(nivel > 1)
    {
        print_nivel(root->left, nivel - 1, fout);
        print_nivel(root->right, nivel - 1, fout);
    }
}

//printez pentru fiecare nivel in ordine, de la root (inaltimea 1 pana la inaltimea arborelui, adica pana ajung la frunze)
void print_nivel_in_ordine(Huffnode *root, FILE *fout)
{
    int inaltime = height(root);
    for(int i = 1; i <= inaltime; i++)
    {
        print_nivel(root, i, fout);
        fprintf(fout, "\n");
    }
}

//functie de eliberare a memoriei
void free_function(Huffnode *root)
{
    if(root)
    {
        free_function(root->left);
        free_function(root->right);
        free(root->nume);
        free(root);
    }
}

void rezolvare_task1(FILE *fin, FILE *fout)
{
    int n;
    fscanf(fin, "%d", &n);

    Heap heap;
    heap.size = 0;
    heap.node = (Huffnode **)malloc(n * sizeof(Huffnode *));
    
    for(int i = 0; i < n; i++)
    {
        char nume[16];
        int prioritate;
        fscanf(fin, "%d %s", &prioritate, nume);
        Huffnode *new_node = create_node(nume, prioritate);
        insert(&heap, new_node);
    }

    Huffnode *root = construire_heap(&heap);
    print_nivel_in_ordine(root, fout);

    free_function(root);
    free(heap.node);
}

void rezolvare_task2(FILE *fin, FILE *fout)
{
    int n;
    fscanf(fin, "%d", &n);

    Heap heap;
    heap.size = 0;
    heap.node = (Huffnode **)malloc(n * sizeof(Huffnode *));
    //citirea satelitilor pentru a crea heap-ul
    for(int i = 0; i < n; i++)
    {
        char nume[16];
        int prioritate;
        fscanf(fin, "%d %s", &prioritate, nume);
        Huffnode *new_node = create_node(nume, prioritate);
        insert(&heap, new_node);
    }
    Huffnode *root = construire_heap(&heap);

    //citirea codificarilor satelitilor
    int numar_codificari;
    char codificare[1001];
    fscanf(fin, "%d", &numar_codificari);
    for(int i = 0; i < numar_codificari; i++)
    {
        fscanf(fin, "%s", codificare);
        identificare_sateliti(root, codificare, fout);
        fprintf(fout, "\n");
    }

    free_function(root);
    free(heap.node);
}

void rezolvare_task3(FILE *fin, FILE *fout)
{
    int n;
    fscanf(fin, "%d", &n);

    Heap heap;
    heap.size = 0;
    heap.node = (Huffnode **)malloc(n * sizeof(Huffnode *));
    //citirea satelitilor pentru a crea heap-ul
    for(int i = 0; i < n; i++)
    {
        char nume[16];
        int prioritate;
        fscanf(fin, "%d %s", &prioritate, nume);
        Huffnode *new_node = create_node(nume, prioritate);
        insert(&heap, new_node);
    }

    //citirea satelitilor carora trebuie sa le stiu codificarea
    Huffnode *root = construire_heap(&heap);
    int numar_sateliti;
    fscanf(fin, "%d", &numar_sateliti);

    for(int i = 0; i < numar_sateliti; i++)
    {
        char nume_sateliti[16];
        fscanf(fin, "%s", nume_sateliti);

        char codificare[1001] = {0};
        if(codificare_sateliti(root, nume_sateliti, codificare, 0))
        {
            fprintf(fout, "%s", codificare);
        }
    }
    fprintf(fout, "\n");

    free_function(root);
    free(heap.node);
}

void rezolvare_task4(FILE *fin, FILE *fout)
{
    int n;
    fscanf(fin, "%d", &n);

    Heap heap;
    heap.size = 0;
    heap.node = (Huffnode **)malloc(n * sizeof(Huffnode *));
    //citirea satelitilor pentru a crea heap-ul
    for(int i = 0; i < n; i++)
    {
        char nume[16];
        int prioritate;
        fscanf(fin, "%d %s", &prioritate, nume);
        Huffnode *new_node = create_node(nume, prioritate);
        insert(&heap, new_node);
    }

    //citirea satelitilor carora trebuie sa le aflu satelitul comun
    Huffnode *root = construire_heap(&heap);
    int numar_sateliti;
    fscanf(fin, "%d", &numar_sateliti);

    char nume_sateliti[100][16]; //vector de stringuri
    for(int i = 0; i < numar_sateliti; i++)
    {
        fscanf(fin, "%s", nume_sateliti[i]);
    }
    Huffnode *lowest_common_ancestor = LCA_mai_multi_sateliti(root, nume_sateliti, numar_sateliti);
    fprintf(fout, "%s\n", lowest_common_ancestor->nume);

    free_function(root);
    free(heap.node);    
}

int main(int argc, char *argv[])
{
    FILE *fin = fopen(argv[2], "r");
    FILE *fout = fopen(argv[3], "w");

    if(strcmp(argv[1], "-c1") == 0)
    {
        rezolvare_task1(fin, fout);
    }
    else if(strcmp(argv[1], "-c2") == 0)
    {
        rezolvare_task2(fin, fout);
    }
    else if(strcmp(argv[1], "-c3") == 0)
    {
        rezolvare_task3(fin, fout);
    }
    else if(strcmp(argv[1], "-c4") == 0)
    {
        rezolvare_task4(fin, fout);
    }
    
    fclose(fin);
    fclose(fout);
    return 0;
}