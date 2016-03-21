#include <stdio.h>

#define FILL 0xFFFFFFFF
#define NONE 0X00000000
#define DIAMETER 15

const unsigned int BALL[][DIAMETER] = {
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
								{NONE,NONE,NONE,NONE,NONE,FILL,FILL,FILL,FILL,FILL,NONE,NONE,NONE,NONE,NONE},
							};
							

							
int main(void){
	int i, j;
	for(i = 0; i < DIAMETER; i++){
		for(j = 0; j < DIAMETER; j++){
			if(BALL[i][j]){
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
