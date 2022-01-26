#include <stdio.h>
#include <syscall.h>
#include<unistd.h>
#define  NUMBER_OF_ITERATIONS     99999999
int main (){
        int i;
        int v;
        int t=2;
        int u=3;
        unsigned int w;

        // do nop
        for(i=0; i<NUMBER_OF_ITERATIONS; i++)
            v=(++t)*(u++);

        if(syscall(335, &w) != 0)
            printf("Error!\n");
        else
            printf("This process encounters %u times context switches.\n", w);
}
