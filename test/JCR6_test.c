#include <stdio.h>
#include "../JCR6.h"


int main(void){
	jcr6_chat=true; // This is a debugging session after all!
	printf(" node = %ld\n",sizeof( jcr6_TCompressionDriveNode));
	printf("*node = %ld\n",sizeof(struct tjcr6_TCompressionDriveNode));
	jcr6_init();
	jcr6_dispose();
}
