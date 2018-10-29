#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "JCR6.h"

static jcr6_TCompressionDriveMap Drivers;

void yell(char *errormessage){
	strcpy(jcr6_error,errormessage);
	if (jcr6_yell) printf("ERROR: %s!\n",errormessage);
	if (jcr6_crash) {
		jcr6_dispose();
		exit(1);
	}
}

char * jcr6_TRecognize(char * myfile){
	char * ret = "NONE";
	if (Drivers==NULL) {
		yell("No directory drivers loaded. Has JCR6 been properly initialized?\n");
		return ret;
	}
	if (Drivers->first==NULL){
		yell("Directory driver map empty. Has JCR6 been properly initialized?\n");
		return ret;
	}
	for(TJCRDirNode node==Drivers->first;node!=NULL;node!=node->next){
		if (node->Driver->Recognize(myfile)) strcpy(ret, node->id);
	}
	return ret;
}

jcr6_TDir jcr6_Dir(char * myfile){

}

// Init JCR6 and make sure the default drivers are all in!
void jcr6_init(){

}

// Free JCR dir (and all data it contains in sub branches)
void jcr6_free(jcr6_TDir j){

}

void jcr6_dispose(void){
	// Freeing all used driver memory
}
