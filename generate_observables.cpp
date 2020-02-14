#include <cstdio>
using namespace std;

int system_size = 20;

int main(){
    freopen("generated_observables.txt", "w", stdout);
    printf("%d\n", system_size);

    for(int i = 0; i < system_size - 1; i++){
        for(int j = 0; j < system_size - 1; j++){
            if(j == i || j == i + 1 || j+1 == i) continue;
            printf("4 Y %d Y %d X %d X %d\n", i, i+1, j, j+1);
        }
    }
    for(int i = 0; i < system_size - 1; i++){
        for(int j = 0; j < system_size; j++){
            if(j == i || j == i + 1) continue;
            for(int j2 = 0; j2 < system_size; j2++){
                if(j2 == i || j2 == i + 1 || j2 == j) continue;
                printf("4 X %d X %d Z %d Z %d\n", i, i+1, j, j2);
            }
        }
    }
    for(int i = 0; i < system_size - 1; i++){
        for(int j = 0; j < system_size; j++){
            if(j == i || j == i + 1) continue;
            printf("3 X %d X %d Z %d\n", i, i+1, j);
        }
    }
}
