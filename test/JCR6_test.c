// Lic:
// ***********************************************************
// JCR6_test.c
// This particular file has been released in the public domain
// and is therefore free of any restriction. You are allowed
// to credit me as the original author, but this is not 
// required.
// This file was setup/modified in: 
// 2018
// If the law of your country does not support the concept
// of a product being released in the public domain, while
// the original author is still alive, or if his death was
// not longer than 70 years ago, you can deem this file
// "(c) Jeroen Broks - licensed under the CC0 License",
// with basically comes down to the same lack of
// restriction the public domain offers. (YAY!)
// *********************************************************** 
// Version 18.10.30
// EndLic

#include <stdio.h>
#include "../JCR6.h"


int main(void){
	jcr6_chat=true; // This is a debugging session after all!
	printf(" node = %ld\n",sizeof( jcr6_TCompressionDriveNode));
	printf("*node = %ld\n",sizeof(struct tjcr6_TCompressionDriveNode));
	char  rc[10];
	jcr6_init();
	bool rb = jcr6_Recognize("test.jcr");
	printf("\n\nRecognise success %d => %s\n\n",rb,jcr6_recas);
	jcr6_dispose();
}
