// Lic:
//   JCR6.c
//   JCR6 for C
//   version: 18.10.31
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
char * jcr6_recas = "NVRCHECK";
jcr6_TDirDriver jcr6_recdrv;



// internal header (for private stuff only)
static void yell(char *errormessage);
static void warn(char *warning);
static void chat(char *dbgchat);
static void mchat(int num,...);
static void chatvalue(char *field,int value);

// JCR6 is a LittleEndian based file system, and I must make sure that on PPC too everything is read in LittleEndian, even when the CPU will calculate in BigEndian.
bool IsLittleEndian () {
    int i=1;
    return (int)*((unsigned char *)&i)==1;
}
// Thanks to Robert Allan Hennigan Leahy for posting this on StackOverflow. ;)


// A few core reading functions
long int streamsize(FILE *stream){
	long int old = ftell(stream);
	fseek(stream, 0L, SEEK_END);
	long int sz = ftell(stream);
	fseek(stream, old, SEEK_SET);
	return sz;
}
union eint{
	int myint;
	char mychar[4];
};
union elong{
	long myint;
	char mychar[8];
};

int stream_readint(FILE *stream){
	//static bool LittleEndian=IsLittleEndian();
	union eint value[2];
	for (int i=0;i<4;i++) {
		value[true] .mychar[  i]=fgetc(stream);
		value[false].mychar[3-i]=value[true].mychar[i];
	}
	return value[IsLittleEndian()].myint;
}

// buffer read functions
bufread buf_start(char * buffer,int size){
	bufread ret = malloc(sizeof(struct tbufread));
	ret->size=size;
	ret->position=0;
	ret->littleendian=true;
	ret->buffer=buffer;
	return ret;
}

bufread buf_start_bigendian(char *buffer,int size){
	bufread ret = buf_start(buffer,size);
	ret->littleendian=false;
	return ret;
}

char buf_read(bufread buffer){
	if (buffer->position>=buffer->size) { yell("End of buffer reached!"); return 26; }
	//chatvalue("Readbyte.position",buffer->position);
	char ret=buffer->buffer[buffer->position];
	//chat("Done");
	buffer->position++;
	return ret;
}

int buf_readint(bufread buffer){
	union eint ret[2];
	for(int i=0;i<4;i++){
		ret[true ].mychar[  i]=buf_read(buffer);
		ret[false].mychar[3-i]=ret[true].mychar[i];
	}
	return ret[IsLittleEndian()==buffer->littleendian].myint;
}
long buf_readlong(bufread buffer){
	union elong ret[2];
	for(int i=0;i<8;i++){
		ret[true ].mychar[  i]=buf_read(buffer);
		ret[false].mychar[7-i]=ret[true].mychar[i];
	}
	return ret[IsLittleEndian()==buffer->littleendian].myint;
}
void buf_readfixed(bufread buffer, char * output, int size){
	for(int i=0;i<size;i++) output[i]=buf_read(buffer);
}
void buf_readstring(bufread buffer, char * output){
	int size=buf_readint(buffer);
	buf_readfixed(buffer,output,size);
}
void buf_readstringcap(bufread buffer,char * output,int max){
	int size=buf_readint(buffer);
	memset(output,0,max);
	//printf("\n\nReadStringCap {\n\tSize: %5d\n\tMax:  %5d\n }\n\n",size,max);
	if (size>=max-1){
		warn("String bigger than set maximum! Truncated for memory protection!");
		buf_readfixed(buffer,output,max-1);
		buffer->position+=(size-(max-1));
	} else {
		buf_readfixed(buffer,output,size);
	}
}
void buf_close(bufread buffer){ // If you do not want the buffer inside to be destroyed, then just free the "bufread" variable itself.
	free(buffer->buffer);
	free(buffer);
}



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

static void chatvalue(char * field,int value){
	if (jcr6_chat){
		printf("DEBUG: %s = %5d\n",field,value);
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
	jcr6_TCompressionDriveNode ret = malloc(sizeof(struct tjcr6_TCompressionDriveNode));
	ret->next=NULL;
	ret->prev=NULL;
	ret->Driver=NULL;
	strcpy(ret->id,"?");
	chat("= New compression node created");
	return ret;
}
jcr6_TDirDriveNode newDirDriveNode(void){
	return malloc(sizeof(struct tjcr6_TDirDriveNode));
}

void jcr6_registercompressiondriver(char * id,jcr6_TCompressDriver d){
	static int count=0; count++;
	jcr6_TCompressionDriveNode ndrv;
	d->destroyoriginal=true;
	if (Drivers->first==NULL){
		chat("FIRST COMPRESS DRIVER NODE!");
		ndrv=newDriverNode(); chat("= Allocated");
		ndrv->Driver=d; chat("= Assigned");
		//printf("%ld\n",sizeof(ndrv->id));
		memset(ndrv->id,0,sizeof(ndrv->id));
		strcpy(ndrv->id,id); mchat(4,"= ID",ndrv->id," <= ",id);
		Drivers->first=ndrv; chat("= In first node");
		ndrv->next=NULL; chat("= NULLED next");
		ndrv->prev=NULL; chat("= NULLED prev");
		ndrv->count=count;
		chat("First node creation done!");
		printf("%s",ndrv->id);
	} else {
		chat("Checking compression driver nodes");
		for(jcr6_TCompressionDriveNode drv=Drivers->first; drv!=NULL; drv=drv->next){
			if (strcmp(drv->id,id)==0) { yell("Duplicate string driver!"); return; }
			ndrv=drv;
		}
		ndrv->next->count=count;
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

jcr6_TCompressDriver jcr6_GetCompressionDriver(char * id){
	mchat(2,"= Looking up storage driver: ",id);
	if (Drivers==NULL) { yell("JCR6 not properly initialized?"); return NULL; }
	if (Drivers->first==NULL) { yell("JCR6 not properly initialized?"); return NULL; }
	for(jcr6_TCompressionDriveNode drv=Drivers->first; drv!=NULL; drv=drv->next){
		if (jcr6_chat) printf("DEBUG: = Count: %d\n",drv->count);
		mchat(2,"= Found: ",drv->id);
		//* debug */ for (int iii=0;iii<10;iii++) printf("%d > %3d\n",iii,drv->id[iii]);
		if (strcmp(drv->id,id)==0) { chat("= We got what we wanted, let's go back home!"); return drv->Driver; }
		chat("= Nope! I don't want ya! Is there more?");
		if (drv->next!=NULL) {chat("= Onto the next!"); } else {chat("= Last one! I smell an error coming up!"); break;}
	}
	yell("Unknown compression method!");
	return NULL;
}

void jcr6_registerdirdriver(char * id, jcr6_TDirDriver d){
	jcr6_TDirDriveNode ndrv;
	if (DirDrivers->first==NULL){
		chat("FIRST DIR DRIVER NODE!");
		ndrv=newDirDriveNode(); chat("= Allocated");
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
		ndrv->next=newDirDriveNode();
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
	jcr6_error=errormessage;
	if (jcr6_yell) printf("ERROR: %s!\n",errormessage);
	if (jcr6_crash) {
		jcr6_dispose();
		exit(1);
	}
}

static void warn(char *warning){
	if (jcr6_yell) printf("WARNING: %s!\n",warning);
}

// Recognize a standard JCR6 file
bool recognize_jcr6(char * file){
	char header[5] = {'J','C','R','6',26};
	char readheader[6];
	mchat(2,"= Trying to recognize file: ",file);
	FILE * bt = fopen(file,"rb");
	if (bt==NULL) { chat("= Error opening file"); return false; }
	if (streamsize(bt)<10) {
		chat("= Streamsize is lower than 10. I don't believe this is a real JCR6 file.");
		fclose(bt);
		return false;
	}
	fgets(readheader,6,(FILE*)bt);
	fclose(bt);
	for (int i=0; i<5; i++){
		if (jcr6_chat){ printf("\tpos %d; readvalue: %3d; truevalue: %3d\n",i,readheader[i],header[i]); }
		if (header[i]!=readheader[i]) return false;
	}
	chat("= All cool!");
	return true;
}

static jcr6_TDir dir_jcr6(char * myfile){
	mchat(2,"= Reading: ",myfile);
	FILE * bt = fopen(myfile,"rb");
	if (bt==NULL) { chat("= Error opening file"); yell("Error opening file"); return NULL; }
	chat("= Opening succseful!");
	for(int i=0;i<5;i++) fgetc(bt); // No need to read the header again. It's already been done.
	chat("= Header skipped");
	// initiate directory object
	chat("= Allocating memory for directory map");
	jcr6_TDir ret = malloc(sizeof(struct tjcr6_TDir));
	// fat offset
	chat("= Reading offset");
	ret->fat_offset = stream_readint(bt);
	// TODO: Reading header config
	// Go to the offset
	fseek(bt,ret->fat_offset,SEEK_SET);
	chat("= Grabbing size data!");
	ret->fat_size      = stream_readint(bt);
	ret->fat_csize     = stream_readint(bt);
	chat("= Reading storage data");
	int storage_length = stream_readint(bt);
	ret->fat_storage   = malloc(storage_length+1);
	fread(ret->fat_storage,storage_length,1,bt);
	ret->fat_storage[storage_length]=0;
	// Get the FAT as a temporary buffer
	mchat(2,"= Working out with storage method: ",ret->fat_storage);
	char * fat_compressedbuffer = malloc(ret->fat_csize);
	char * fat_buffer;
	fread(fat_compressedbuffer,ret->fat_csize,1,bt);
	jcr6_TCompressDriver storagedriver = jcr6_GetCompressionDriver(ret->fat_storage);
	if (storagedriver->destroyoriginal) {
		storagedriver->expand(fat_compressedbuffer,ret->fat_csize,fat_buffer,ret->fat_size);
		free(fat_compressedbuffer); chat("= Destroying packed data as we no longer need that");
	} else {
		chat("= Keeping packed data. Apparently it's still required!");
		fat_buffer=fat_compressedbuffer;
	}

	// close file
	chat("= Closing file, as we can now move on to the memory!");
	fclose(bt);

	// Open the uncompressed fat buffer so we can read that
	chat("= Start reading from buffer!");
	bufread buf = buf_start(fat_buffer,ret->fat_size);
	bool first = true;
	bool theend = false;
	do{
		chat("= New read cycle");
		if (buf->position>=buf->size) { yell("FAT out of bounds. Must be missing a proper ending tag!"); break; }
		chat("= Reading byte tag");
		unsigned char tag = buf_read(buf);
		chatvalue("= Byte Tag",tag);
		char stag[10]; // It's not unthinkable that future versions require this number to be higher...
		switch(tag){
			case 0x0f:
				chat("End of FAT!");
				theend=true;
				break;
			case 0x01:
				chat("= Instruction tag found!");
				memset(stag,0,sizeof(stag));
				buf_readstring(buf,stag);
				mchat(2,"= Instruction: ",stag);
				if (strcmp(stag,"FILE")==0){
					chat("FILE tag!");
					char fkey[JCR6_MAX_CFGSTRING];
					char fstring[JCR6_MAX_CFGSTRING];
					char fbl;
					int  fint;
					unsigned char ftag=0;
					jcr6_TEntry E = malloc(sizeof(struct tjcr6_TEntry));
					jcr6_TEntryNode ENode = malloc(sizeof(struct tjcr6_TEntryNode));
					// theend=true; // DEBUG ONLY!!!
					do{
						// TODO: https://github.com/Tricky1975/JCR6_C/issues#1
						ftag=buf_read(buf);
						chatvalue("= File config tag ",ftag);
						switch(ftag){
							case 255: break;
							case   1:
								buf_readstringcap(buf,fkey,sizeof(fkey));
								buf_readstringcap(buf,fstring,sizeof(fstring));
								mchat(3,fkey,"=",fstring);
								break;
							case   2:
								buf_readstringcap(buf,fkey,sizeof(fkey));
								fbl=buf_read(buf);
								chatvalue(fkey,fbl);
								break;
							case   3:
								buf_readstringcap(buf,fkey,sizeof(fkey));
								fint=buf_readint(buf);
								chatvalue(fkey,fint);
								break;
						}
					} while (ftag!=0xff);
				}
				break;
			default:
				yell("Unknown instruction tag!");
				theend=true;
		}
	} while(theend==false);





	// close release the buffer we no longer need
	buf_close(buf);

	return ret;
}


bool jcr6_Recognize(char * myfile){
	mchat(2,"Recognize:",myfile);
	jcr6_recas="NONE";
	chat("= Drivers loaded at all?");
	if (DirDrivers==NULL) {
		yell("No directory drivers loaded. Has JCR6 been properly initialized?\n");
		return false;
	}
	chat("= Drivers properly initiated?");
	if (DirDrivers->first==NULL){
		yell("Directory driver map empty. Has JCR6 been properly initialized?\n");
		return false;
	}
	bool rv=false;
	for(jcr6_TDirDriveNode node=DirDrivers->first;node!=NULL;node=node->next){
		mchat(2,"\t\t\tCHECKING: ",node->id);
		if (node->Driver->recognize(myfile)) {
			chat("= Setting");
			jcr6_recas= node->id;
			jcr6_recdrv=node->Driver;
			/*
			for(int i=0;i<10 && node->id[i]!=0;i++) {
				chat("c");
				printf("%d %d %s\n",i,node->id[i],node->id);
				recas[i]=node->id[i];
			}
			*/
			chat("= SET!");
			rv=true;
		}
	}
	return rv;
}

jcr6_TDir jcr6_Dir(char * myfile){
	if (!jcr6_Recognize(myfile)) { yell("File not recognized for reading in JCR!"); return NULL; }
	mchat(4,"File ",myfile," has been recognized as ",jcr6_recas);
	return jcr6_recdrv->dir(myfile);
}

// Init JCR6 and make sure the default drivers are all in!
void jcr6_init(void){
	chat("JCR6 start up");
	chat("Allocating memory for compression algorithm drivers");
	Drivers = malloc(sizeof(struct tjcr6_TCompressionDriveMap));
	Drivers->first=NULL;
	chat("Allocating memory for Store");
	jcr6_TCompressDriver Store = jcr_newCompressDriver;
	chat("Store storage functions are being added");
	Store->compress=&store_compress;
	Store->expand=&store_expand;
	chat("= Registering");
	jcr6_registercompressiondriver("Store",Store);
	Store->destroyoriginal=false; // Must be done AFTER registering!
	chat("Allocating memory for compression algorithm drivers");
	DirDrivers = malloc(sizeof(struct tjcr6_TDirDriveMap));
	DirDrivers->first=NULL;
	chat("JCR6 file setup driver");
	jcr6_TDirDriver DJCR=malloc(sizeof(struct tjcr6_TDirDriver));
	DJCR->recognize=recognize_jcr6;
	DJCR->dir=dir_jcr6;
	chat("= Registering");
	jcr6_registerdirdriver("JCR6",DJCR);
}

// Free JCR dir (and all data it contains in sub branches)
void jcr6_free(jcr6_TDir j){
	free(j->fat_storage);
	free(j); chat("= Freed JCR directory object");
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
		chat("Unloading dir drivers");
		jcr6_TDirDriveNode last;
		for(jcr6_TDirDriveNode drv=DirDrivers->first; drv!=NULL; drv=drv->next){
			free(drv->Driver); drv->Driver=NULL; mchat(2,"= Released Dir Driver: ",drv->id);
			if (drv->prev!=NULL) {free(drv->prev); drv->prev=NULL; chat("= Prevnode release.");}
			last=drv;
		}
		free(last); chat("= Lastnode release");
		free(DirDrivers); chat("= Driver map release");
	}

}
