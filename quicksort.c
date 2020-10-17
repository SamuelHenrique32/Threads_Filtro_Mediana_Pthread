#include <stdio.h>

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