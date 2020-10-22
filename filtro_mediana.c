#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// --------------------------------------------------------------------------------------------------------

#define kQTD_PARAMS 4
#define kARQ_SAIDA "saida.bmp"
#define kQTD_BITS_IMG 24
#define kQTD_ELEMENTOS_MASCARA 49
//#define kDEBUG
#define kDEBUG_MASCARA_ORDENADA
#define kDEBUG_MEDIANA

// Sem alinhamento
#pragma pack (1)

// --------------------------------------------------------------------------------------------------------

void quicksort(int *v, int start, int end);
int partition(int *v, int start, int end);
void bubble_sort(int *v, int size);
int median(int *v, int tamanhoMascara);
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

void bubble_sort(int *v, int size) {
    int k, j, aux;

    for(k=(size - 1) ; k>0 ; k--) {
        //printf("\n[%d] ", k);

        for(j=0 ; j<k ; j++) {
            //printf("%d, ", j);

            if(v[j] > v[j + 1]) {
                aux = v[j];
                v[j] = v[j+1];
                v[j+1] = aux;
            }
        }
    }
}

int median(int *v, int tamanhoMascara) {

    int posElemento = 0, val1 = 0, val2 = 0;

    if((tamanhoMascara*tamanhoMascara)%2 == 0) {

        val1 = v[(tamanhoMascara*tamanhoMascara/2) - 1];

        val2 = v[tamanhoMascara*tamanhoMascara/2];

        #ifdef kDEBUG
            printf("Posicao elemento 01 para mediana: %d\n", (tamanhoMascara*tamanhoMascara/2)-1);
            printf("Posicao elemento 02 para mediana: %d\n", (tamanhoMascara*tamanhoMascara/2));
        #endif

        val1 += val2;

        return (val1/2);
    }
    else {

        val1 = v[(tamanhoMascara*tamanhoMascara-1)/2];

    #ifdef kDEBUG
        printf("Posicao elemento para mediana: %d\n", (tamanhoMascara*tamanhoMascara-1)/2);
    #endif

        return val1;
    }
}

int main(int argc, char **argv) {

    int i, j, tamanhoMascara, nroThreads, deslPosMascara, posVetMascaraRed = 0, posVetMascaraGreen = 0, posVetMascaraBlue = 0, posX, posY, startX, startY, medianRed, medianGreen, medianBlue;

    int mascaraVet[kQTD_ELEMENTOS_MASCARA];

    unsigned char media;

    HEADER c;

    // Vetor de ponteiros
    RGB **img = NULL, **imgCopy = NULL, *vetMascaraRGB = NULL, pixel;

    int *vetmascaraRedInt = NULL, *vetmascaraGreenInt = NULL, *vetmascaraBlueInt = NULL;

    // Descritor
    FILE *in, *out;

    if(argc!=kQTD_PARAMS) {

        printf("%s <tamanho_mascara> <numero_threads> <arquivo_entrada>\n", argv[0]);

        exit(0);
    }

    tamanhoMascara = atoi(argv[1]);

    nroThreads = atoi(argv[2]);

    vetMascaraRGB = malloc(tamanhoMascara*tamanhoMascara*sizeof(RGB));

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

    //vetmascaraRedInt = malloc(sizeof(int)*tamanhoMascara*tamanhoMascara);
    //vetmascaraGreenInt = malloc(sizeof(int)*tamanhoMascara*tamanhoMascara);
    //vetmascaraBlueInt = malloc(sizeof(int)*tamanhoMascara*tamanhoMascara);

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

    if(c.nbits != kQTD_BITS_IMG) {
        printf("\nA imagem lida nao possui %d bits", kQTD_BITS_IMG);

        exit(0);
    }

    deslPosMascara = tamanhoMascara/2;

    // printf("deslPosMascara = %d\n", deslPosMascara);

    // Escreve cabecalho de saida
    fwrite(&c, sizeof(HEADER), 1, out);

    // Altura * tam para ponteiro de RGB
    img = (RGB**) malloc(c.altura*sizeof(RGB *));
    imgCopy = (RGB**) malloc(c.altura*sizeof(RGB *));

    // Cada pos aponta para vetor de RGB
    for(i=0 ; i<c.altura ; i++) {
        img[i] = (RGB*) malloc(c.largura*sizeof(RGB));
    }

    for(i=0 ; i<c.altura ; i++) {
        imgCopy[i] = (RGB*) malloc(c.largura*sizeof(RGB));
    }

    // Le 1 pixel por vez
    for(i=0 ; i<c.altura ; i++) {
        for(j=0 ; j<c.largura ; j++) {
            fread(&img[i][j], sizeof(RGB), 1, in);
            imgCopy[i][j] = img[i][j];
        }
    }

    posVetMascaraRed = 0;
    posVetMascaraGreen = 0;
    posVetMascaraBlue = 0;

    // Posicao atual na matriz
    posX = deslPosMascara;
    posY = deslPosMascara;

    // Posicao inicial de deslocamento em X para mascara
    startX = 0;

    // Posicao corrente de deslocamento em X para mascara
    j = startX;

    // Posicao inicial de deslocamento em Y para mascara
    startY = 0;

    // Posicao corrente de deslocamento em Y para mascara
    i = startY;

    // Para cada pixel da imagem
    while((posX<c.largura) && (posY<c.altura)) {

        if((startX>=0) && (startY>=0)) {

            vetmascaraRedInt = malloc(sizeof(int)*tamanhoMascara*tamanhoMascara);
            vetmascaraGreenInt = malloc(sizeof(int)*tamanhoMascara*tamanhoMascara);
            vetmascaraBlueInt = malloc(sizeof(int)*tamanhoMascara*tamanhoMascara);

            // Processamento para pixel atual
            while(1) {

                vetmascaraRedInt[posVetMascaraRed] = img[i][j].red;
                vetmascaraGreenInt[posVetMascaraGreen] = img[i][j].green;
                vetmascaraBlueInt[posVetMascaraBlue] = img[i][j].blue;

                medianRed = 0;
                medianGreen = 0;
                medianBlue = 0;

                //printf("\nRed img[%d][%d].red = %d vetmascaraRedInt[%d] = %d\n", i, j, img[i][j].red, posVetMascaraRed, vetmascaraRedInt[posVetMascaraRed]);
                //printf("Green img[%d][%d].green = %d vetmascaraGreenInt[%d] = %d\n", i, j, img[i][j].green, posVetMascaraGreen, vetmascaraGreenInt[posVetMascaraGreen]);
                //printf("Blue img[%d][%d].blue = %d vetmascaraBlueInt[%d] = %d\n", i, j, img[i][j].blue, posVetMascaraBlue, vetmascaraBlueInt[posVetMascaraBlue]);

                posVetMascaraRed++;
                posVetMascaraGreen++;
                posVetMascaraBlue++;

                // Linha coluna
                //printf("[%d][%d]", i, j);
                //printf("\n");

                // Incrementa coluna
                if((j+1-startX) <= (deslPosMascara*2)) {
                    j++;
                }
                else {

                    // Troca linha
                    i++;
                    j = startX;
                }

                // Proximo pixel
                if(posVetMascaraRed >= (tamanhoMascara*tamanhoMascara)) {

                    memset(mascaraVet, 0, sizeof(mascaraVet));

                    for(int x=0 ; x<tamanhoMascara*tamanhoMascara ; x++) {
                        mascaraVet[x] = vetmascaraRedInt[x];
                    }

                #ifdef kQUICK_SORT
                    quicksort(mascaraVet, 0, tamanhoMascara*tamanhoMascara);
                #else
                    bubble_sort(mascaraVet, tamanhoMascara*tamanhoMascara);
                #endif

                    medianRed = median(mascaraVet, tamanhoMascara);
                #ifdef kDEBUG_MEDIANA
                    printf("\nMediana Red: %d\n", medianRed);
                #endif

                #ifdef kDEBUG_MASCARA_ORDENADA
                    int pos = 0;
                    
                    printf("\nMascara Red apos ordenacao:\n");                    

                    for(int pos=0 ; pos<tamanhoMascara*tamanhoMascara ; pos++) {
                        printf("%d ", mascaraVet[pos]);
                    }

                    printf("\n\n");
                #endif

                    free(vetmascaraRedInt);

                    memset(mascaraVet, 0, sizeof(mascaraVet));

                    for(int x=0 ; x<tamanhoMascara*tamanhoMascara ; x++) {
                        mascaraVet[x] = vetmascaraGreenInt[x];
                    }

                #ifdef kQUICK_SORT
                    quicksort(mascaraVet, 0, tamanhoMascara*tamanhoMascara);
                #else
                    bubble_sort(mascaraVet, tamanhoMascara*tamanhoMascara);
                #endif

                    medianGreen = median(mascaraVet, tamanhoMascara);
                #ifdef kDEBUG_MEDIANA
                    printf("\nMediana Green: %d\n", medianGreen);
                #endif

                #ifdef kDEBUG_MASCARA_ORDENADA
                    pos = 0;
                    
                    printf("\nMascara Green apos ordenacao:\n");                    

                    for(int pos=0 ; pos<tamanhoMascara*tamanhoMascara ; pos++) {
                        printf("%d ", mascaraVet[pos]);
                    }

                    printf("\n\n");
                #endif

                    free(vetmascaraGreenInt);

                    memset(mascaraVet, 0, sizeof(mascaraVet));

                    for(int x=0 ; x<tamanhoMascara*tamanhoMascara ; x++) {
                        mascaraVet[x] = vetmascaraBlueInt[x];
                    }

                #ifdef kQUICK_SORT
                    quicksort(mascaraVet, 0, tamanhoMascara*tamanhoMascara);
                #else
                    bubble_sort(mascaraVet, tamanhoMascara*tamanhoMascara);
                #endif

                    medianBlue = median(mascaraVet, tamanhoMascara);
                #ifdef kDEBUG_MEDIANA
                    printf("\nMediana Blue: %d\n", medianBlue);
                #endif

                #ifdef kDEBUG_MASCARA_ORDENADA
                    pos = 0;
                    
                    printf("\nMascara Blue apos ordenacao:\n");                    

                    for(int pos=0 ; pos<tamanhoMascara*tamanhoMascara ; pos++) {
                        printf("%d ", mascaraVet[pos]);
                    }

                    printf("\n\n");
                #endif

                    free(vetmascaraBlueInt);

                    posVetMascaraRed = 0;
                    posVetMascaraGreen = 0;
                    posVetMascaraBlue = 0;

                    break;
                }
            }
        }

        // Pixel a direita
        if((posX+1) <= ((c.largura-1)-deslPosMascara)) {

            posX++;

            // Posicao inicial de deslocamento em X para mascara
            startX = posX-deslPosMascara;

            // Posicao corrente de deslocamento em X para mascara
            j = startX;

            // Posicao inicial de deslocamento em Y para mascara
            startY = posY-deslPosMascara;

            // Posicao corrente de deslocamento em Y para mascara
            i = startY;
        }
        // Pixel abaixo
        else if((posY+1) <= ((c.altura-1)-deslPosMascara)) {

            posX = deslPosMascara;

            posY++;

            // Posicao inicial de deslocamento em X para mascara
            startX = posX-deslPosMascara;

            // Posicao corrente de deslocamento em X para mascara
            j = startX;

            // Posicao inicial de deslocamento em Y para mascara
            startY = posY-deslPosMascara;

            // Posicao corrente de deslocamento em Y para mascara
            i = startY;
        }
        else {
            break;
        }
    }


    // median(vetMascara, tamanhoMascara);

    // Percorre matriz ja carregada
    for(i=0 ; i<c.altura ; i++) {
        for(j=0 ; j<c.largura ; j++) {

            // Grava pixel
            fwrite(&imgCopy[i][j], sizeof(RGB), 1, out);
        }
    }

    for(i=0 ; i<c.altura ; i++) {
        free(img[i]);
        free(imgCopy[i]);
    }

    free(img);
    free(imgCopy);

    fclose(in);
    fclose(out);
}

// --------------------------------------------------------------------------------------------------------