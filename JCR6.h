#ifndef __JEROENS_COLLECTED_RESOURCE_VERSION_6_LOADED__
#define __JEROENS_COLLECTED_RESOURCE_VERSION_6_LOADED__


// Entry mapping
typedef struct{
	char * entry;
	char * mainfile;
	int size;
	int compressedsize;
	char * storagemethod;
	char * author;
	char * notes;
	// TODO: config map could be added in future versions, as it could deal with the stuff outside the default JCR6 scope.

} jcr6_TEntry;

typedef struct{
	char * id;
	jcr6_TEntry * entry;
	jcr6_TEntryNode * next;
	jcr6_TEntryNode * prev;
} jcr6_TEntryNode;

typedef struct{
	jcr6_TEntryNode * first;

	} jcr6_TEntryMap;



#endif
