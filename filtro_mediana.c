#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// --------------------------------------------------------------------------------------------------------

#define kQTD_PARAMS 4
#define kARQ_SAIDA "saida.bmp"
#define kDEBUG

// Sem alinhamento
#pragma pack (1)

// --------------------------------------------------------------------------------------------------------

void quicksort(int *v, int start, int end);
int partition(int *v, int start, int end);
int main(int argc, char **argv);

// --------------------------------------------------------------------------------------------------------

typedef struct header {
    // Cabecalho arquivo
    unsigned short int tipo; // 2 bytes
    unsigned int tamanhoArquivo; // 4 bytes
    unsigned short int reservado1;
    unsigned short int reservado2;
    unsigned int offset;

    // Cabecalho imagem
    unsigned int tamanhoCabecalho;
    unsigned int largura;
    unsigned int altura;
    unsigned short int planos; // Deve ser 1
    unsigned short int nbits;
    unsigned int compressao;
    unsigned int tamanhoImagem;
    unsigned int xres; // Resolucao x
    unsigned int yres; // Resolucao y
    unsigned int ucores;
    unsigned int sigcores;
    
} HEADER;

// 8 bits por cor = 24 bits (0 a 255)
// Armazenado ao contrario
typedef struct rgb {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
} RGB;

// --------------------------------------------------------------------------------------------------------

void quicksort(int *v, int start, int end) {
    int pivot;

    if(end>start) {

        // Separate in two partitions
        pivot = partition(v, start, end);

        // Call function for two partitions
        quicksort(v, start, pivot-1);
        quicksort(v, pivot+1, end);
    }
}

int partition(int *v, int start, int end) {

    int left, right, pivot, aux;

    left = start;

    right = end;

    pivot = v[start];

    while(left<right) {

        while(v[left] <= pivot)
        {
            left++;
        }

        while(v[right] > pivot)
        {
            right--;
        }

        // Change left and right
        if(left<right) {
            aux = v[left];
            v[left] = v[right];
            v[right] = aux;
        }        
    }

    v[start] = v[right];

    v[right] = pivot;

    return right;
}

int main(int argc, char **argv) {

    int i, j, tamanhoMascara, nroThreads;

    unsigned char media;

    HEADER c;

    // Pixel
    RGB pixel;

    // Vetor de ponteiros
    RGB **img = NULL;

    // Descritor
    FILE *in, *out;

    if(argc!=kQTD_PARAMS) {

        printf("%s <tamanho_mascara> <numero_threads> <arquivo_entrada>\n", argv[0]);

        exit(0);
    }

    tamanhoMascara = atoi(argv[1]);

    nroThreads = atoi(argv[2]);

    in = fopen(argv[3], "rb");

    if(in == NULL) {
        printf("Erro ao abrir arquivo \"%s\" de entrada\n", argv[3]);

        exit(0);
    }

    // Abre arquivo binario para escrita
    out = fopen(kARQ_SAIDA, "wb");

    if(out == NULL) {
        printf("Erro ao abrir arquivo de saida\n");

        exit(0);
    }

    printf("Tamanho mascara: %d\nQuantidade de threads: %d\n", tamanhoMascara, nroThreads);

    // Le cabecalho de entrada
    fread(&c, sizeof(HEADER), 1, in);

#ifdef kDEBUG
    printf("Tamanho do arquivo: %d\n", c.tamanhoArquivo);

    printf("Offset: %d\n", c.offset);

    printf("Largura: %d\n", c.largura);

    printf("Altura: %d\n", c.altura);

    printf("Nro bits: %d\n", c.nbits);

    // Somente bytes da imagem
    printf("Tamanho da imagem: %d\n", c.tamanhoImagem);
#endif

    // Escreve cabecalho de saida
    fwrite(&c, sizeof(HEADER), 1, out);

    // Altura * tam para ponteiro de RGB
    img = (RGB**) malloc(c.altura*sizeof(RGB *));

    // Cada pos aponta para vetor de RGB
    for(i=0 ; i<c.altura ; i++) {
        img[i] = (RGB*) malloc(c.largura*sizeof(RGB));
    }
}

// --------------------------------------------------------------------------------------------------------