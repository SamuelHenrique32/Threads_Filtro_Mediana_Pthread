#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// --------------------------------------------------------------------------------------------------------

#define kVECTOR_LENGTH 10

// --------------------------------------------------------------------------------------------------------

void quicksort(int *v, int start, int end);
int partition(int *v, int start, int end);
int main(void);

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

int main() {

    int i, v[kVECTOR_LENGTH];

    srand(time(NULL));
  
    for(int i=0; i<kVECTOR_LENGTH; i++) {
        v[i] = rand() % 100;
    }

    for(int i=0; i<kVECTOR_LENGTH; i++) {
        printf("%d ", v[i]);
    }

    printf("\n\n");

    quicksort(v, 0, (kVECTOR_LENGTH-1));

    for(int i=0; i<kVECTOR_LENGTH; i++) {
        printf("%d ", v[i]);
    }

    printf("\n\n");
}

// --------------------------------------------------------------------------------------------------------