#include <stdio.h>

#define FILL        0xFFFFFFFF
#define NONE        0X00000000
#define DIAMETER    15
#define BALL_SIZE   20

struct Ball_s{
    	int x; /*horizontal anchor*/
    	int y; /*vertical anchor*/
    	int d; /*direction*/
    	int s; /*speed*/
    	int c; /*color*/
} Ball_default = {0, 0, 0, 50, 0};
typedef struct Ball_s Ball;

const unsigned int BALL_MASK[][DIAMETER] = {
                                {NONE,NONE,NONE,NONE,NONE,FILL,FILL,FILL,FILL,FILL,NONE,NONE,NONE,NONE,NONE},
                                {NONE,NONE,NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE,NONE,NONE},
                                {NONE,NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE,NONE},
                                {NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE},
                                {NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE},
                                {FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL},
                                {FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL},
                                {FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL},
                                {FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL},
                                {FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL},
                                {NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE},
                                {NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE},
                                {NONE,NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE,NONE},
                                {NONE,NONE,NONE,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,FILL,NONE,NONE,NONE},
                                {NONE,NONE,NONE,NONE,NONE,FILL,FILL,FILL,FILL,FILL,NONE,NONE,NONE,NONE,NONE}
                            };

int main(void){
    int i, j;
    for(i = 0; i < DIAMETER; i++){
        for(j = 0; j < DIAMETER; j++){
            if(BALL_MASK[i][j]){
                printf("O");
            }
            else{
                printf(" ");
            }
        }
        printf("\r\n");
    }
    return 0;
}
