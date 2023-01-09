#include "cm4_utils.h"

void delayMs(uint32_t milliseconds){
	struct timespec ms;
	ms.tv_sec = 0;
	ms.tv_nsec = milliseconds*1000*1000;
	nanosleep(&ms, NULL);
}


void printBuffer(const uint8_t * buffer, uint8_t size){

    for(uint8_t i=0;i<size;i++){
		printf("%u",buffer[i]);
		printf(" ");
    }
    printf("\n");
}

void printBufferHex(const uint8_t * buffer, uint8_t size){
	printf("0x ");
    for(uint8_t i=0;i<size;i++){
		printf("%2x",buffer[i]);
		printf(" ");
    }
    printf("\n");
}
