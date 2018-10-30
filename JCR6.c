// Lic:
//   JCR6.c
//   JCR6 for C
//   version: 18.10.30
//   Copyright (C) 2018 Jeroen P. Broks
//   This software is provided 'as-is', without any express or implied
//   warranty.  In no event will the authors be held liable for any damages
//   arising from the use of this software.
//   Permission is granted to anyone to use this software for any purpose,
//   including commercial applications, and to alter it and redistribute it
//   freely, subject to the following restrictions:
//   1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//   2. Altered source versions must be plainly marked as such, and must not be
//      misrepresented as being the original software.
//   3. This notice may not be removed or altered from any source distribution.
// EndLic

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
static jcr6_TCompressionDriveMap Drivers = NULL;
static jcr6_TDirDriveMap DirDrivers = NULL;

// Store Driver functions
static void store_compress(char * originalbuf,int originalsize,char * compressedbuf,int * compressedsize){
	compressedsize=&originalsize;
	compressedbuf=originalbuf;
}
static void store_expand(char * originalbuf,int originalsize,char * expandedbuf,int expandedsize){
	if (expandedsize!=originalsize) yell("Expanded size doesn't match original size, which should not be possible in the Store method");
	expandedbuf=originalbuf;
}

jcr6_TCompressionDriveNode newDriverNode(void){
	return malloc(sizeof(struct tjcr6_TCompressionDriveNode));
}
jcr6_TDirDriveNode newDirDriveNode(void){
	return malloc(sizeof(struct tjcr6_TDirDriveNode));
}

void jcr6_registercompressiondriver(char * id,jcr6_TCompressDriver d){
	jcr6_TCompressionDriveNode ndrv;
	if (Drivers->first==NULL){
		chat("FIRST DRIVER NODE!");
		ndrv=newDriverNode(); chat("= Allocated");
		ndrv->Driver=d; chat("= Assigned");
		strcpy(ndrv->id,id); chat("= ID");
		Drivers->first=ndrv; chat("= In first node");
		ndrv->next=NULL; chat("= NULLED next");
		ndrv->prev=NULL; chat("= NULLED prev");
		chat("First node creation done!");
	} else {
		chat("Checking driver nodes");
		for(jcr6_TCompressionDriveNode drv=Drivers->first; drv!=NULL; drv=drv->next){
			if (strcmp(drv->id,id)==0) { yell("Duplicate string driver!"); return; }
			ndrv=drv;
		}
		ndrv->next=newDriverNode();
		strcpy(ndrv->next->id,id);
		ndrv->next->Driver=d;
		ndrv->next->prev=ndrv;
		ndrv->next->next=NULL;
	}
	//char * dbg;
	//sprintf(dbg,"Registered compression algorithm %s",id);
	mchat(2,"Registered storage: ",id);
}

void jcr6_registerdirdriver(char * id, jcr6_TDirDriver d){
	jcr6_TCompressionDriveNode ndrv;
	if (DirDrivers->first==NULL){
		chat("FIRST DIR DRIVER NODE!");
		ndrv=newDirDriverNode(); chat("= Allocated");
		ndrv->Driver=d; chat("= Assigned");
		strcpy(ndrv->id,id); chat("= ID");
		DirDrivers->first=ndrv; chat("= In first node");
		ndrv->next=NULL; chat("= NULLED next");
		ndrv->prev=NULL; chat("= NULLED prev");
		chat("First node creation done!");
	} else {
		chat("Checking dir driver nodes");
		for(jcr6_TDirDriveNode drv=DirDrivers->first; drv!=NULL; drv=drv->next){
			if (strcmp(drv->id,id)==0) { yell("Duplicate dir driver!"); return; }
			ndrv=drv;
		}
		ndrv->next=newDirDriverNode();
		strcpy(ndrv->next->id,id);
		ndrv->next->Driver=d;
		ndrv->next->prev=ndrv;
		ndrv->next->next=NULL;
	}
	//char * dbg;
	//sprintf(dbg,"Registered compression algorithm %s",id);
	mchat(2,"Registered dir: ",id);

}

static void yell(char *errormessage){
	strcpy(jcr6_error,errormessage);
	if (jcr6_yell) printf("ERROR: %s!\n",errormessage);
	if (jcr6_crash) {
		jcr6_dispose();
		exit(1);
	}
}

// Recognize a standard JCR6 file
bool recognize_jcr6(char * file){
	char header[5] = {'J','C','R','6',26};
	char readheader[5];
	FILE * bt = fopen(file,"rb");
	fgets(readheader,5,(FILE*)bt);
	fclose(bt);
	for (int i=0; i<5; i++){
		if (header[i]!=readheader[i]) return false;
	}
	return true;
}


bool jcr6_Recognize(char * recas[10],char * myfile){
	mchat(2,"Recognize:",myfile);
	*recas="NONE";
	if (DirDrivers==NULL) {
		yell("No directory drivers loaded. Has JCR6 been properly initialized?\n");
		return false;
	}
	if (DirDrivers->first==NULL){
		yell("Directory driver map empty. Has JCR6 been properly initialized?\n");
		return false;
	}
	bool rv=false;
	for(jcr6_TDirDriveNode node=DirDrivers->first;node!=NULL;node=node->next){
		if (node->Driver->recognize(myfile)) {
			strcpy(*recas, node->id); rv=true;
		}
	}
	return rv;
}

jcr6_TDir jcr6_Dir(char * myfile){
	return NULL;
}

// Init JCR6 and make sure the default drivers are all in!
void jcr6_init(void){
	chat("JCR6 start up");
	chat("Allocating memory for compression algorithm drivers");
	Drivers = malloc(sizeof(struct tjcr6_TDirDriveMap));
	Drivers->first=NULL;
	chat("Allocating memory for Store");
	jcr6_TCompressDriver Store = jcr_newCompressDriver;
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
		chat("Unloading compression drivers");
		jcr6_TCompressionDriveNode last;
		for(jcr6_TCompressionDriveNode drv=Drivers->first; drv!=NULL; drv=drv->next){
			free(drv->Driver); drv->Driver=NULL; mchat(2,"= Released Compression driver Driver: ",drv->id);
			if (drv->prev!=NULL) {free(drv->prev); drv->prev=NULL; chat("= Prevnode release.");}
			last=drv;
		}
		free(last); chat("= Lastnode release");
		free(Drivers); chat("= Driver map release");
	}
	// Unload dir drivers and the entire driver map
	{
		chat("Unloading dir drivers")
		jcr6_TDirDriveNode last;
		for(jcr6_TDirDriveNode drv=DirDrivers->first; drv!=NULL; drv=drv->next){
			free(drv->Driver); drv->Driver=NULL; mchat(2,"= Released Dir Driver: ",drv->id);
			if (drv->prev!=NULL) {free(drv->prev); drv->prev=NULL; chat("= Prevnode release.");}
			last=drv;
		}
		free(last); chat("= Lastnode release");
		free(Drivers Drivers); chat("= Driver map release");
	}

}
