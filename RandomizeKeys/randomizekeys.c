// DeadEnds Tool
//
// randomizekeys.c is the DeadEnds tool that randomizes the keys in a Gedcom file.
//
// Created by Thomas Wetmore on 14 July 2024.
// Last changed on 10 August 2024.

#include "randomizekeys.h"

static void getArguments(int, char**, String*);
static void getEnvironment(String*);
static void usage(void);
static void goAway(ErrorLog*);

static bool debugging = true;

// main is the main program of the randomize keys batch program.
int main(int argc, char** argv) {
	String gedcomFile = null;
	String searchPath = null;
	printf("%s: RandomizeKeys begin.\n", getMillisecondsString());
	getArguments(argc, argv, &gedcomFile);
	getEnvironment(&searchPath);
	gedcomFile = resolveFile(gedcomFile, searchPath);
	if (debugging) printf("Resolved file: %s\n", gedcomFile);
	// Get the Gedcom records from a file.
	File* file = openFile(gedcomFile, "r");
	ErrorLog* log = createErrorLog();

	// Parse the Gedcom file and build a GNodeList of all of its records.
	GNodeList* roots = getGNodeTreesFromFile(file, log);
	printf("ramdomize keys: %s: read gedcom file.\n", getMillisecondsString());
	if (lengthList(log) > 0) goAway(log);
	closeFile(file);

	// Validate the keys.
	checkKeysAndReferences(roots, file->name, log);
	printf("ramdomize keys: %s: validated keys.\n", getMillisecondsString());
	if (lengthList(log)) {
		deleteGNodeList(roots, true);
		goAway(log);
	}

	// Create a table that maps existing keys to random keys.
	StringTable* keyTable = createStringTable(1025);
	initRecordKeyGenerator();
	FORLIST(roots, element)
		GNodeListEl* el = (GNodeListEl*) element;
		GNode* root = el->node;
		String key = root->key;
		if (!key) continue;
		RecordType r = recordType(root);
		String newKey = generateRecordKey(r); // Get a random key.
		addToStringTable(keyTable, key, newKey);
	ENDLIST
	printf("ramdomize keys: %s: created remap table.\n", getMillisecondsString());

	// Change the keys throughout the roots list.
	FORLIST(roots, element)
		// Change the key on the root.
		GNodeListEl* el = (GNodeListEl*) element;
		GNode* root = el->node;
		String key = root->key;
		if (!key) continue;
		String new = searchStringTable(keyTable, key);
		root->key = new;
		// Change all values that are keys.
		FORTRAVERSE(root, node)
			if (isKey(node->value)) {
				new = searchStringTable(keyTable, node->value);
				node->value = strsave(new);
			}
		ENDTRAVERSE
	ENDLIST
	printf("ramdomize keys: %s: rekeyed records.\n", getMillisecondsString());

	// Write the modified GNodeList to standard out.
	FORLIST(roots, element)
		GNodeListEl* el = (GNodeListEl*) element;
		writeGNodeRecord(stdout, el->node, false);
	ENDLIST
	printf("ramdomize keys: %s: wrote gedcom file.\n", getMillisecondsString());
	return 0;
}

// getFileArguments gets the file names from the command line. They are mandatory.
static void getArguments(int argc, char* argv[], String* gedcomFile) {
	int ch;
	while ((ch = getopt(argc, argv, "g:")) != -1) {
		switch(ch) {
		case 'g':
			*gedcomFile = strsave(optarg);
			break;
		case '?':
		default:
			usage();
			exit(1);
		}
	}
	if (!*gedcomFile) {
		usage();
		exit(1);
	}
}

// getEnvironment checks for the DE_GEDCOM_PATH env variable.
static void getEnvironment(String* gedcomPath) {
	*gedcomPath = getenv("DE_GEDCOM_PATH");
	if (!*gedcomPath) *gedcomPath = ".";
}

// usage prints the RunScript usage message.
static void usage(void) {
	fprintf(stderr, "usage: RandomizeKeys -g gedcomfile\n");
}

static void goAway(ErrorLog* log) {
	printf("randomizekeys: cancelled due to errors\n");
	showErrorLog(log);;
	exit(1);
}
