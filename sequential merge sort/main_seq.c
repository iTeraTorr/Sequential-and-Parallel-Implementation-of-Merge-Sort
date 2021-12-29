#include <stdio.h>   
#include <stdlib.h>  
#include <time.h>    
#include <string.h>  
#include <limits.h>  

//Prints the contents of a given list of a process
void printList(char arrayName[], int array[], int arraySize) {
    printf("%s: ", arrayName);
    for (int i = 0; i < arraySize; i++) {
        printf(" %d", array[i]);
    }
    printf("\n");
}

int comp (const void * a, const void * b) {
   return ( (*(int*)a) - (*(int*)b) );
}

//implements merge sort: merges sorted arrays from processes until we have a single array containing all integers in sorted order
int* mergeSort(int array[], int arraySize){

    qsort(array, arraySize, sizeof(int), comp); 
	return array;
}

int main(int argc, char** argv) {
    int arraySize;
    int *array;
    clock_t startTime, endTime;

    // get size of array
    if (argc!= 2){
		fprintf(stderr, "usage:  %s <size_of_array> \n", argv[0]);
        arraySize = -1;
    } else {
		arraySize = atoi(argv[1]);
	}

	if (arraySize <= 0) {
        exit(-1);
    }

    // allocate memory for global array and fill with values
    array = (int*) malloc (arraySize * sizeof(int));
	srand(time(0));
	for (int i = 0; i < arraySize; i++) {
		array[i] = rand() % 100;
	}
    printList("UNSORTED ARRAY", array, arraySize);  
    
    //Start timing
    startTime = clock();
    
    //Merge sort
	array = mergeSort(array, arraySize);

    //End timing
    endTime = clock();

    printList("FINAL SORTED ARRAY", array, arraySize); 
    printf("Sorting %d integers took %f seconds \n", arraySize, (double)(endTime-startTime)/CLOCKS_PER_SEC);
	free(array);
	
    return 0;
}   