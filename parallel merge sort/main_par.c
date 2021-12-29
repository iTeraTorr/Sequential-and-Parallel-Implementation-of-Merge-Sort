#include <mpi.h>     
#include <stdio.h>   
#include <stdlib.h>  
#include <time.h>    
#include <math.h>    
#include <string.h>  
#include <limits.h>  
int comp (const void * a, const void * b) {
   return ( (*(int*)a) - (*(int*)b) );
}
int* merge(int half1[], int half2[], int mergeResult[], int size){
    int ai, bi, ci;
    ai = bi = ci = 0;
    // integers remain in both arrays to compare
    while ((ai < size) && (bi < size)){
        if (half1[ai] <= half2[bi]){
			mergeResult[ci] = half1[ai];
			ai++;
		} else {
			mergeResult[ci] = half2[bi];
			bi++;
		}
		ci++;
	}
	// integers only remain in rightArray
	if (ai >= size){
        while (bi < size) {
			mergeResult[ci] = half2[bi];
			bi++; ci++;
		}
	}
	// integers only remain in localArray
	if (bi >= size){
		while (ai < size) {
			mergeResult[ci] = half1[ai];
			ai++; ci++;
		}
	}
	return mergeResult;
}
int *mergeSort(int height, int id, int localArray[], int size,MPI_Comm comm,int globalArray[]){
int parent, rightChild,myHeight=0;
int *lefthalf, *righthalf, *mergeResult;
qsort(localArray,size,sizeof(int),comp);
lefthalf=localArray;

while(myHeight<height){
    parent=(id & (~(1<<myHeight)));
    if(parent==id){
        rightChild=(id | ((1<<myHeight)));
        righthalf=(int*)malloc(size*sizeof(int));
        MPI_Recv(righthalf,size,MPI_INT,rightChild,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        mergeResult=(int*)malloc(size*2*sizeof(int));
        mergeResult=merge(lefthalf,righthalf,mergeResult,size);
        lefthalf=mergeResult;
        size*=2;
        free(righthalf);
        mergeResult=NULL;
        myHeight++;
    }else{
        MPI_Send(lefthalf,size,MPI_INT,parent,0,MPI_COMM_WORLD);
        if(myHeight!=0)
            free(lefthalf);
        myHeight=height;
    }
}
if(id==0){
    return lefthalf;
}
return NULL;;
}

int main(int argc, char **argv){
    
    MPI_Init(&argc,&argv);
    int nProcess, id, arraySize,height,localArraySize;
    int *array, *localArray;
    MPI_Comm_size(MPI_COMM_WORLD, &nProcess);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    //Checking if number of processes is a power of 2 or not
    if(id==0){
        if(nProcess==0 || (nProcess & (nProcess-1))!=0){
            printf("Number of processes must be a power of 2\n");
            MPI_Finalize();
            exit(1);
        }

        //Checking if the input is in correct format or not
        if(argc!=2){
            fprintf(stderr, "Usage: mpirun -n <p> %s <size_of_array>\n",argv[0]);
            arraySize=-1;
        }else if((atoi(argv[1]))%nProcess!=0){
            fprintf(stderr,"Size of array must be equally divisible among the processes\n");
            arraySize=-1;
        }else{
            arraySize=atoi(argv[1]);
        }
    }

    MPI_Bcast(&arraySize,1,MPI_INT,0,MPI_COMM_WORLD);
    if(arraySize<=0){
        MPI_Finalize();
        exit(1);
    }

    height=log2(nProcess);

    localArraySize=arraySize/nProcess;
    localArray=(int*)malloc(localArraySize*sizeof(int));
    srand(id+time(0));
    for(int i=0;i<localArraySize;i++){
        localArray[i]=rand()%100;
    }

    array=(int*)malloc(arraySize*sizeof(int));
    MPI_Gather(localArray,localArraySize,MPI_INT,array,localArraySize,MPI_INT,0,MPI_COMM_WORLD);

    if(id==0){
        printf("UNSORTED ARRAY: ");
        for(int i=0;i<arraySize;i++){
            printf("%d, ",array[i]);
        }
        printf("\n");
    }

    double startTime, localTime, totalTime,processTime;
    startTime=MPI_Wtime();
    processTime=MPI_Wtime();
    array=mergeSort(height,id,localArray,localArraySize,MPI_COMM_WORLD,array);    
    processTime=MPI_Wtime()-processTime;
    printf("Process #%d of %d took %f seconds\n",id,nProcess,processTime);
    localTime=MPI_Wtime()-startTime;
    MPI_Reduce(&localTime,&totalTime,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

    if(id==0){
        printf("FINAL SORTED ARRAY: ");
        for(int i=0;i<arraySize;i++){
            printf("%d, ",array[i]);
        }
        printf("\n");
        printf("Sorting %d integers took %f seconds\n",arraySize,totalTime);
        free(array);
    }

    free(localArray);
    MPI_Finalize();
    return 0;
}