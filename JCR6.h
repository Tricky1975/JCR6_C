#ifndef __JEROENS_COLLECTED_RESOURCE_VERSION_6_LOADED__
#define __JEROENS_COLLECTED_RESOURCE_VERSION_6_LOADED__
#include <stdbool.h>

bool jcr6_autodel = true;

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

typedef struct tjcr6_TEntryNode{
	char * id;
	jcr6_TEntry * entry;
	struct tjcr6_TEntryNode * next;
	struct tjcr6_TEntryNode * prev;
} jcr6_TEntryNode;

typedef struct{
	jcr6_TEntryNode * first;

	} jcr6_TEntryMap;


#define FOREACHENTRY(jdir) for(jcr6_EntryNode node=jdir->first;node->next!=NULL;node=node->next)

#endif
