#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include "JCR6.h"

bool jcr6_autodel = true;
bool jcr6_yell = true;
bool jcr6_crash = true;
bool jcr6_chat = false;
char * jcr6_error = "";


// internal header (for private stuff only)
static void yell(char *errormessage);
static void chat(char *dbgchat);
static void mchat(int num,...);



// debug chat
static void chat(char *dbgchat){
	if (jcr6_chat) {
		printf("DEBUG: %s\n",dbgchat);
	}
}

static void mchat(int num,...){
	if (jcr6_chat){
		va_list va;
		//int num;
		va_start(va, num);
		printf("DEBUG:");
		for (int i = 0; i < num; i++){
			printf(" ");
			printf("%s",va_arg(va,char *));
		}
		printf("\n");
	}
}


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
	if (Drivers->first==NULL){
		chat("FIRST DRIVER NODE!");
		ndrv=malloc(sizeof(jcr6_TCompressionDriveNode)); chat("= Allocated");
		ndrv->Driver=d; chat("= Assigned");
		strcpy(ndrv->id,id); chat("= ID");
		Drivers->first=ndrv; chat("= In first node");
		chat("First node creation done!");
	} else {
		chat("Checking driver nodes");
		for(jcr6_TCompressionDriveNode drv=Drivers->first; drv!=NULL; drv=drv->next){
			if (strcmp(drv->id,id)==0) { yell("Duplicate string driver!"); return; }
			ndrv=drv;
		}
		ndrv->next=malloc(sizeof(jcr6_TCompressionDriveNode));
		strcpy(ndrv->next->id,id);
		ndrv->next->Driver=d;
		ndrv->next->prev=ndrv;
	}
	//char * dbg;
	//sprintf(dbg,"Registered compression algorithm %s",id);
	mchat(2,"Registered: ",id);
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
	chat("JCR6 start up");
	chat("Allocating memory for compression algorithm drivers");
	Drivers = malloc(sizeof(jcr6_TCompressionDriveMap));
	Drivers->first=NULL;
	chat("Allocating memory for Store");
	jcr6_TCompressDriver Store = malloc(sizeof(jcr6_TCompressDriver));
	chat("Store storage functions are being added");
	Store->compress=&store_compress;
	Store->expand=&store_expand;
	chat("Registering");
	jcr6_registercompressiondriver("Store",Store);
}

// Free JCR dir (and all data it contains in sub branches)
void jcr6_free(jcr6_TDir j){

}

void jcr6_dispose(void){
	// Unload compression drivers and the entire driver map
	{
		jcr6_TCompressionDriveNode last;
		for(jcr6_TCompressionDriveNode drv=Drivers->first; drv!=NULL; drv=drv->next){
			free(drv->Driver); mchat(2,"Released: ",drv->id);
			if (drv->prev!=NULL) free(drv->prev);
			last=drv;
		}
		free(last);
		free(Drivers);
	}

}
