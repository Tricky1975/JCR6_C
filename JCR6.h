// Lic:
//   JCR6.h
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

#ifndef __JEROENS_COLLECTED_RESOURCE_VERSION_6_LOADED__
#define __JEROENS_COLLECTED_RESOURCE_VERSION_6_LOADED__

#include <stdbool.h>

// extern
extern bool jcr6_autodel;
extern bool jcr6_yell;
extern bool jcr6_crash;
extern bool jcr6_chat;
extern char * jcr6_error;



// Entry mapping
typedef struct{
	char * entry;
	char * mainfile;
	int size;
	int compressedsize;
	char * storagemethod;
	char * author;
	char * notes;
	// TODO: Config map could be added in future versions, as it could deal with the stuff outside the default JCR6 scope.

} jcr6_TEntry;

typedef struct tjcr6_TEntryNode{
	char * id;
	jcr6_TEntry * entry;
	struct tjcr6_TEntryNode * next;
	struct tjcr6_TEntryNode * prev;
} jcr6_TEntryNode;

typedef struct{
	jcr6_TEntryNode * first;

	} jcr6_TEntryMap;

typedef struct {
	jcr6_TEntryMap * Entries;
} * jcr6_TDir;


#define FOREACHENTRY(jdir) for(jcr6_EntryNode node=jdir->Entries->first;node->next!=NULL;node=node->next)


// Dir file base driver
typedef struct tjcr6_TDirDriver {
	bool (*recognize)(char * file);
	jcr6_TDir (*dir)(char * file);
} * jcr6_TDirDriver;

// Compression base driver
typedef struct {
	void (*compress)(char * originalbuf,int originalsize,char * compressedbuf,int * compressedsize);
	void (*expand)(char * originalbuf,int originalsize,char * expandedbuf,int expandedsize);
} * jcr6_TCompressDriver;


// Driver map
typedef struct tjcr6_TDirDriveNode{
	char id[10];
	jcr6_TDirDriver Driver;
	struct tjcr6_TDirDriveNode next;
	struct tjcr6_TDirDriveNode prev;
} * jcr6_TDirDriveNode;

typedef struct tjcr6_TDirDriveMap{
	jcr6_TDirDriveNode first;
} * jcr6_TDirDriveMap;



// Compression map
typedef struct tjcr6_TCompressionDriveNode{
	char id[10];
	jcr6_TCompressDriver Driver;
	struct tjcr6_TCompressionDriveNode * next;
	struct tjcr6_TCompressionDriveNode * prev;
} * jcr6_TCompressionDriveNode;

typedef struct tjcr6_TCompressionDriveMap{
	struct tjcr6_TCompressionDriveNode * first;
} * jcr6_TCompressionDriveMap;




// macro
#define jcr_newCompressDriver malloc(sizeof(jcr6_TCompressDriver));
#define jcr_newDirDriver malloc(sizeof(jcr6_TDirDriver));

// function headers
void jcr6_registercompressiondriver(char * id,jcr6_TCompressDriver d);
void jcr6_init(void);
jcr6_TDir jcr6_Dir(char * myfile);
void jcr6_free(jcr6_TDir j);
void jcr6_dispose(void);
bool jcr6_Recognize(char * recas[10],char * myfile);


#endif
