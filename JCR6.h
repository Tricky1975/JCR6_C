// Lic:
//   JCR6.h
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

#ifndef __JEROENS_COLLECTED_RESOURCE_VERSION_6_LOADED__
#define __JEROENS_COLLECTED_RESOURCE_VERSION_6_LOADED__

#include <stdbool.h>

#define JCR6_MAX_CFGSTRING 255


// Entry mapping
typedef struct tjcr6_TEntry {
	char entry[JCR6_MAX_CFGSTRING];
	char mainfile[JCR6_MAX_CFGSTRING];
	int size;
	int compressedsize;
	int timestamp;
	int unixpermissions;
	char storagemethod[JCR6_MAX_CFGSTRING];
	char author[JCR6_MAX_CFGSTRING];
	char notes[JCR6_MAX_CFGSTRING];
	// TODO: Config map could be added in future versions, as it could deal with the stuff outside the default JCR6 scope.
	// jcr_configmap cfg;  // Just put here for later usage!

} * jcr6_TEntry;

typedef struct tjcr6_TEntryNode{
	char id[JCR6_MAX_CFGSTRING];
	jcr6_TEntry entry;
	struct tjcr6_TEntryNode * next;
	struct tjcr6_TEntryNode * prev;
} * jcr6_TEntryNode;

typedef struct tjcr6_TEntryMap{
	jcr6_TEntryNode first;

	} jcr6_TEntryMap;

typedef struct tjcr6_TDir {
	jcr6_TEntryMap * Entries;
	// Only put in for use by the JCR6 format driver, but I guess WAD can also benefit from this.
	int fat_offset;
	int fat_size;
	int fat_csize;
	char * fat_storage;
} * jcr6_TDir;


#define FOREACHENTRY(jdir) for(jcr6_EntryNode node=jdir->Entries->first;node->next!=NULL;node=node->next)


// Dir file base driver
typedef struct tjcr6_TDirDriver {
	bool (*recognize)(char * file);
	jcr6_TDir (*dir)(char * file);
} * jcr6_TDirDriver;

// Compression base driver
typedef struct tjcr6_TCompressDriver {
	void (*compress)(char * originalbuf,int originalsize,char * compressedbuf,int * compressedsize);
	void (*expand)(char * originalbuf,int originalsize,char * expandedbuf,int expandedsize);
	bool destroyoriginal;
	   /*
	    * NOTE:
	    * The registering function will ALWAYS define true in this, which should always be the value used
	    * (unless you love memory leaks), but in the exceptional cases the original buffer should remain in the
	    * memory (like with the Store method where the original and target will just be a pointer forwarding in
	    * order to save time) this should be set to false AFTER registering the new driver.
	    *
	    * This may be an odd approach, but since C has no automated garbage collector, all read data from before
	    * or after the (de)compression must be disposed/freed manually and in the case of Store that could lead
	    * to very funny outcomes if I didn't take this approach.
	    */
} * jcr6_TCompressDriver;


// Driver map
typedef struct tjcr6_TDirDriveNode{
	char id[10];
	jcr6_TDirDriver Driver;
	struct tjcr6_TDirDriveNode * next;
	struct tjcr6_TDirDriveNode * prev;
} * jcr6_TDirDriveNode;

typedef struct tjcr6_TDirDriveMap{
	jcr6_TDirDriveNode first;
} * jcr6_TDirDriveMap;



// Compression map
typedef struct tjcr6_TCompressionDriveNode{
	char ignoreme; // I don't know why, but this ugly declaration prevents the id from being emptied...
	char id[10];
	int count;
	jcr6_TCompressDriver Driver;
	struct tjcr6_TCompressionDriveNode * next;
	struct tjcr6_TCompressionDriveNode * prev;
} * jcr6_TCompressionDriveNode;

typedef struct tjcr6_TCompressionDriveMap{
	struct tjcr6_TCompressionDriveNode * first;
} * jcr6_TCompressionDriveMap;



// buffer read type
typedef struct tbufread{
	int size;
	int position;
	bool littleendian; // true when LittleEndian false when BigEndian
	char * buffer;
} * bufread;


// macro
#define jcr_newCompressDriver malloc(sizeof(jcr6_TCompressDriver));
#define jcr_newDirDriver malloc(sizeof(jcr6_TDirDriver));

// function headers
void jcr6_registercompressiondriver(char * id,jcr6_TCompressDriver d);
void jcr6_init(void);
jcr6_TDir jcr6_Dir(char * myfile);
void jcr6_free(jcr6_TDir j);
void jcr6_dispose(void);
bool jcr6_Recognize(char * myfile);
jcr6_TCompressDriver jcr6_GetCompressionDriver(char * id); // I doubt you'll ever need this outside the JCR6 library, but what the heck!

// buffer read functions
bufread buf_start(char * buffer, int size);
bufread buf_start_bigendian(char *buffer,int size);
char buf_read(bufread buffer);
int buf_readint(bufread buffer);
long buf_readlong(bufread buffer);
void buf_readfixed(bufread buffer, char * output, int size);
void buf_readstring(bufread buffer, char * output);
void buf_readstringa(bufread buffer, char * output);
void buf_close(bufread buffer); // If you do not want the buffer inside to be destroyed, then just free the "bufread" variable itself.

// extern vars
extern bool jcr6_autodel;
extern bool jcr6_yell;
extern bool jcr6_crash;
extern bool jcr6_chat;
extern char * jcr6_error;
extern char * jcr6_recas;
extern jcr6_TDirDriver jcr6_recdrv;


#endif
