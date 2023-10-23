#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/syscall.h> 
#include <assert.h>
#include <string.h>
#include <math.h>

#define total_times 500000000

static int numCPU = -1;

struct thread_info {    /* Used as argument to thread() */
	int id;
	double upper;
	double lower;
};


void thread(void *arg) {
    struct thread_info **ti = (struct thread_info **)&arg;

    double n=total_times/numCPU; // divide quarter-circle to n chunks
    double chunk=(double)1/total_times; // the length of every chunks
    int start=n*(*ti)->id;
    int finish=n*((*ti)->id+1);
    double u;
    double l;
    //printf("%d %d\n",start,finish);
    for(int i=start;i<finish;i++){
        u+=(sqrt(1-pow(i*chunk,2))); // 0~n-1
    }
    // lower = upper left-shift one chunk and add the finish chunk.
    l=u-(sqrt(1-pow(start*chunk,2)))+(sqrt(1-pow(finish*chunk,2))); 

    u*=chunk;
    l*=chunk;

    (*ti)->upper = u;
    (*ti)->lower = l;
    //printf("th2: %lf,%lf\n",(*ti)->upper,(*ti)->lower);
}

int main(int argc, char **argv) {
    char* exename = argv[0];
    numCPU=atoi(argv[1]);
    //sys_numCPU = sysconf( _SC_NPROCESSORS_ONLN );
    //printf("cores num: %d\n", sys_numCPU);
    struct thread_info tinfo[numCPU]; 
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t) * numCPU);
    
    for (int i=0; i<numCPU; i++) {
    	tinfo[i].id=i;
        pthread_create(&tid[i], NULL, (void *) thread, (void *)&tinfo[i]);
    }

    for (int i=0; i< numCPU; i++) {
        pthread_join(tid[i], NULL);
    }

    double upper_sum=0;
    double lower_sum=0;
    for(int i=0;i<numCPU;i++){
        upper_sum+=tinfo[i].upper;
        lower_sum+=tinfo[i].lower;
    }

    upper_sum*=4;
    lower_sum*=4;
    char upper_string[256];
    char lower_string[256];
    sprintf(upper_string,"%.10lf",upper_sum);
    sprintf(lower_string,"%.10lf",lower_sum);
    printf("upper_sum = %s\n",upper_string);
    printf("lower_sum = %s\n",lower_string);
    int precise=0;
    char PI[256];
    while(upper_string[precise]!='\0' && lower_string[precise]!='\0' && upper_string[precise]==lower_string[precise]){
        PI[precise]=upper_string[precise];
        precise++;
    }
    printf("%d\n",precise-2);
    PI[precise]='\0';

    printf("PI = %s\n",PI);
}
