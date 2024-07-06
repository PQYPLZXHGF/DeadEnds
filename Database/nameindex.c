// DeadEnds Project
//
// nameindex.c implements the NameIndex, an index that maps Gedcom name keys to the sets of person
// record keys that have the names.
//
// Created by Thomas Wetmore on 26 November 2022.
// Last changed on 5 July 2024.

#include "nameindex.h"
#include "name.h"
#include "sort.h"
#include "set.h"
#include "gedcom.h"

static NameIndexEl* createNameIndexEl(String nameKey, String recordKey);
static int numNameIndexBuckets = 2048;

// getKey gets the name key of a NameIndex element.
static String getKey(void* element) {
	return ((NameIndexEl*) element)->nameKey;
}

// compare compares two name keys.
static int compare(String a, String b) {
	return strcmp(a, b);
}

// delete frees a NameIndex element.
static void delete(void* element) {
	NameIndexEl *nameEl = (NameIndexEl*) element;
	stdfree(nameEl->nameKey);
	deleteSet(nameEl->recordKeys);
	stdfree(nameEl);
}

// createNameIndex creates a NameIndex.
NameIndex *createNameIndex(void) {
	return createHashTable(getKey, compare, delete, numNameIndexBuckets);
}

// deleteNameIndex deletes a name index.
void deleteNameIndex(NameIndex *nameIndex) {
	deleteHashTable(nameIndex);
}

// insertInNameIndex adds a (name key, person key) relationship to a NameIndex.
void insertInNameIndex(NameIndex* index, String nameKey, String recordKey) {
	//printf("insertInNameIndex: nameKey, personKey: %s, %s\n", nameKey, personKey); // DEBUG
	NameIndexEl* element = (NameIndexEl*) searchHashTable(index, nameKey);
	if (!element) {
		element = createNameIndexEl(nameKey, recordKey);
		addToHashTable(index, element, true);
	}
	Set* recordKeys = element->recordKeys;
	if (!isInSet(recordKeys, recordKey)) {
		addToSet(recordKeys, recordKey);
	}
}

// searchNameIndex searches NameIndex for a name and returns the record keys that have the name.
Set* searchNameIndex(NameIndex* index, String name) {
	String nameKey = nameToNameKey(name);
	NameIndexEl* element = searchHashTable(index, nameKey);
	return element == null ? null : element->recordKeys;
}

// showNameIndex shows the contents of a name index.
static void showSetElement(void* setEl) {
	printf("  %s\n", (String) setEl);
}
static void showElement(void* element) {
	Set* recordKeys = ((NameIndexEl*)element)->recordKeys;
	iterateSet(recordKeys, showSetElement);
}
void showNameIndex(NameIndex* index) {
	showHashTable(index, showElement);
}

// getSetKey gets the key of a Set element.
static String getSetKey(void* element) {
	return (String) element;
}

// compareSetKeys compares two keys of Set elements.
static int compareSetKeys(String a, String b) {
	return compareRecordKeys(a, b);
}

// createNameIndexEl creates and returns a NameIndexEl.
static NameIndexEl* createNameIndexEl(String nameKey, String recordKey) {
	NameIndexEl* el = (NameIndexEl*) stdalloc(sizeof(NameIndexEl));
	el->nameKey = strsave(nameKey);  // MNOTE: nameKey is in data space.
	el->recordKeys = createSet(getSetKey, compareSetKeys, null);
	return el;
}
