#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "JCR6.h"

bool jcr6_autodel = true;
bool jcr6_yell = true;
bool jcr6_crash = true;
char * jcr6_error = "";


// yell header
void yell(char *errormessage);

// Driver map
static jcr6_TCompressionDriveMap Drivers;
static jcr6_TDirDriveMap DirDrivers;

// Store Driver functions
static void store_compress(char * originalbuf,int originalsize,char * compressedbuf,int * compressedsize){
	compressedsize=&originalsize;
	compressedbuf=originalbuf;
}
static void store_expand(char * originalbuf,int originalsize,char * expandedbuf,int expandedsize){
	if (expandedsize!=originalsize) yell("Expanded size doesn't match original size, which should not be possible in the Store method");
	expandedbuf=originalbuf;
}

void jcr6_registercompressiondriver(char * id,jcr6_TCompressDriver d){
	jcr6_TCompressionDriveNode ndrv;
	for(jcr6_TCompressionDriveNode drv=Drivers->first; drv!=NULL; drv=drv->next){
		if (strcmp(drv->id,id)==0) { yell("Duplicate string driver!"); return; }
		ndrv=drv;
	}
	ndrv->next=malloc(sizeof(jcr6_TCompressionDriveNode));
	strcpy(ndrv->next->id,id);
	ndrv->next->Driver=d;
	ndrv->next->prev=ndrv;

}

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
	for(jcr6_TDirDriveNode node=DirDrivers->first;node!=NULL;node=node->next){
		if (node->Driver.recognize(myfile)) strcpy(ret, node->id);
	}
	return ret;
}

jcr6_TDir jcr6_Dir(char * myfile){
	return NULL;
}

// Init JCR6 and make sure the default drivers are all in!
void jcr6_init(void){
	jcr6_TCompressDriver Store = malloc(sizeof(jcr6_TCompressDriver));
	Store->compress=&store_compress;
	Store->expand=&store_expand;
	jcr6_registercompressiondriver("Store",Store);
}

// Free JCR dir (and all data it contains in sub branches)
void jcr6_free(jcr6_TDir j){

}

void jcr6_dispose(void){
	// Freeing all used compression driver memory

}
