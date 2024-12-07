// DeadEnds
//
// testsequence.c has code to test the Sequence data type.
//
// Created by Thomas Wetmore on 2 May 2024.
// Last changed on 16 May 2024.

#include "sequence.h"
#include "utils.h"

static void checkTest(String, int, int);
static Sequence* tomsAncestors(Database*);
static Sequence* lusAncestors(Database*);
static Sequence* tomAndLusAncestorsClosed(Database*);

// testSequence is the starting function to test the Sequence type.
void testSequence(Database* database, int testNumber) {
	printf("%d: TEST SEQUENCE: %2.3f\n", testNumber, getMseconds());

	// Create a Sequence.
	Sequence* sequence = createSequence(database);
	int length = lengthSequence(sequence);
	checkTest("Sequence should be empty", 0, length);

	// Add an element to a Sequence.
	appendToSequence(sequence, "@I1@", null);
	length = lengthSequence(sequence);
	checkTest("Sequence should have one element", 1, length);
	showSequence(sequence, "added I1 elememnt");

	// Test emptySequence.
	emptySequence(sequence);
	length = lengthSequence(sequence);
	checkTest("Empty sequence", 0, length);

	// Test copySequence.
	appendToSequence(sequence, "@I1@", null);
	length = lengthSequence(sequence);
	checkTest("Added element to emptied sequence", 1, length);
	Sequence* copied = copySequence(sequence);
	length = lengthSequence(copied);
	checkTest("Copied Sequence should have 1 element", 1, length);
	showSequence(sequence, "original");
	showSequence(copied, "copied");

	// Test isInSequence.
	appendToSequence(sequence, "@I2@", null);
	showSequence(sequence, "added I2");
	bool isIn = isInSequence(sequence, "@I1@");
	checkTest("I1 should be in", isIn ? 1 : 0, 1);
	isIn = isInSequence(sequence, "@I2@");
	checkTest("I2 should be in", isIn ? 1 : 0, 1);
	isIn = isInSequence(sequence, "@I3@");
	checkTest("I3 should not be in", isIn ? 1 : 0, 0);

	// Test removeFromSequence.
	bool removed = removeFromSequence(sequence, "@I1@");
	checkTest("I1 should be removed", removed ? 1 : 0, 1);
	showSequence(sequence, "removed I1");

	// Test keySortSequence.
	appendToSequence(sequence, "@I1@", null);
	appendToSequence(sequence, "@I6@", null);
	appendToSequence(sequence, "@I3@", null);
	appendToSequence(sequence, "@I5@", null);
	appendToSequence(sequence, "@I4@", null);
	showSequence(sequence, "I1, I5, I3, I5, I4");
	printf("Calling keySortSequence\n");
	keySortSequence(sequence);
	showSequence(sequence, "key sorted");

	// Test nameSortSequence.
	printf("Calling nameSortSequence\n");
	nameSortSequence(sequence);
	showSequence(sequence, "name sorted");

	// Test ancestorSequence.
	printf("Testing ancestorSequence\n");
	Sequence* ancestors = tomsAncestors(database);
	showSequence(ancestors, "Tom's Ancestors");
	printf("Sort ancestors by key\n");
	keySortSequence(ancestors);
	showSequence(ancestors, "Ancestors key sorted");
	printf("Sort ancestors by name\n");
	nameSortSequence(ancestors);
	showSequence(ancestors, "ancestors name sorted");
	// Test closed form of ancestorSequence.
	printf("Testing ancestorSequence with close set to true\n");
	emptySequence(ancestors);
	ancestors = tomAndLusAncestorsClosed(database);
	showSequence(ancestors, "tom and lu's ancestors closed");

	// Test uniqueSequence.
	printf("Setting up to test uniqueSequence\n");
	emptySequence(sequence);
	emptySequence(ancestors);
	appendToSequence(sequence, "@I2@", null);
	ancestors = ancestorSequence(sequence, false);
	printf("THIS SHOULD BE LU'S ANCESTORS\n");
	showSequence(ancestors, "Lu's ancestors");
	emptySequence(copied);
	copied = copySequence(ancestors);
	printf("THIS SOULD BE A COPY OF LU'S ANCESTORS\n");
	showSequence(copied, "Copy of Lu's ancestors");
	printf("THIS SHOULD BE A SEQUENCE WITH ALL OF LU'S ANCESTORS TWICE\n");
	appendSequenceToSequence(ancestors, copied);
	showSequence(ancestors, "Two copies of Lu's ancestors");
	printf("Now doing the uniqueing\n");
	Sequence* uniqued = uniqueSequence(ancestors);
	showSequence(uniqued, "Lu's two copied ancestors uniqued");
	printf("Now doing the uniqueing in place -- first three copies\n");
	appendSequenceToSequence(uniqued, ancestors);
	showSequence(uniqued, "Another copy of ancestors added");
	uniqueSequenceInPlace(uniqued);
	printf("And now one copy\n");
	showSequence(uniqued, "Those uniqued in place");

	// Test personToChildren
	printf("Testing personToChildren, personToFathers, personToMothers\n");
	GNode* tom = getRecord("@I1@", database);
	Sequence* kids = personToChildren(tom, database);
	showSequence(kids, "Kids of I1");
	deleteSequence(kids);
	// Test personToFathers, personToMothers
	Sequence* fathers = personToFathers(tom, database);
	showSequence(fathers, "fathers of I1");
	Sequence* mothers = personToMothers(tom, database);
	showSequence(mothers, "Mothers of I1");
	deleteSequence(fathers);
	deleteSequence(mothers);
	// Test familyToChildren, familyToFathers, familyToMothers
	printf("Testing familyToChildren, familyToFathers, familyToMothers\n");
	GNode* fam = getRecord("@F1@", database);
	kids = familyToChildren(fam, database);
	fathers = familyToFathers(fam, database);
	mothers = familyToMothers(fam, database);
	showSequence(kids, "Kids of F1");
	showSequence(fathers, "Fathers of F1");
	showSequence(mothers, "Mothers of F1");
	deleteSequence(kids);
	deleteSequence(fathers);
	deleteSequence(mothers);
	// Test personToSpouses
	printf("Testing personToSpouses\n");
	tom = getRecord("@I25@", database);
	Sequence* spouses = personToSpouses(tom, database);
	showSequence(spouses, "Spouses of I25");
	// Test nameToSequence.
	printf("Testing nameToSequence\n");
	Sequence* tomwets = nameToSequence("Thomas Trask/Wtmre/", database);
	showSequence(tomwets, "Seauence of Thomas Trask/WTmre/s");
	printf("Testing wild card name feature on */grenda/\n");
	Sequence* grendas = nameToSequence("*/Grenda", database);
	showSequence(grendas, "Sequence of */Grenda");
	// Test childSequence.
	printf("Testing childSequence\n");
	Sequence* children = childSequence(tomwets);
	showSequence(children, "Children of tomwets");
	// Test parentSequence
	printf("Testing parentSequence\n");
	Sequence* parents = parentSequence(tomwets);
	showSequence(parents, "Parents of tomwets");
	// Test spouseSequence.
	printf("Testing spouseSequence\n");
	spouses = spouseSequence(tomwets);
	showSequence(spouses, "Spouses of tomwets");
	// Test descendentSequence.
	printf("Testing descendentSequence\n");
	Sequence* desc = descendentSequence(tomwets, false);
	showSequence(desc, "Descendents of tomwets");
	// Test siblingSequence
	printf("Testing siblingSequence\n");
	Sequence* sibs = siblingSequence(tomwets, 0);
	showSequence(sibs, "Siblings of tomwets");
	// Test elementFromSequence.
	printf("Testing elementFromSequence\n");
	for (int i = 0; i < lengthSequence(tomwets); i++) {
		String key, name;
		elementFromSequence(tomwets, i, &key, &name);
		printf("%d: %s: %s\n", i, key, name);
	}
	// Test unionSequence.
	printf("Testing unionSequence\n");
	Sequence* toms = tomsAncestors(database);
	Sequence* lus = lusAncestors(database);
	Sequence* unionseq = unionSequence(toms, lus);
	showSequence(unionseq, "Union of Tom and Lu's ancestors");
	// Test intersectSequence.
	printf("Testing intersectSequence\n");
	Sequence* intersectseq = intersectSequence(toms, lus);
	showSequence(intersectseq, "Intersection of Tom and Lu's ancestors");
	// Test differenceSequence.
	printf("Testing differenceSequence\n");
	Sequence* tomDiffLu = differenceSequence(toms, lus);
	Sequence* luDiffTom = differenceSequence(lus, toms);
	showSequence(tomDiffLu, "difference of Tom's and Lu's ancestors");
	showSequence(luDiffTom, "Difference of Lu's and Tom's ancestors");

	printf("END TEST SEQUENCE: %2.3f\n", getMseconds());
//	Sequence *personToFamilies(GNode *person, bool, Database*);
//	Sequence *refn_to_indiseq(String refn);
//	void renameElementInSequence(Sequence* sequence, String key);
//	void sequenceToGedcom(Sequence*, FILE*);
}

static Sequence* tomsAncestors(Database* database) {
	Sequence* s = createSequence(database);
	appendToSequence(s, "@I1@", null);
	Sequence* a = ancestorSequence(s, false);
	deleteSequence(s);
	return a;
}
static Sequence* tomAndLusAncestorsClosed(Database* database) {
	Sequence* s = createSequence(database);
	appendToSequence(s, "@I1@", null);
	appendToSequence(s, "@I2@", null);
	Sequence* a = ancestorSequence(s, true);
	deleteSequence(s);
	return a;
}
static Sequence* lusAncestors(Database* database) {
	Sequence* s = createSequence(database);
	appendToSequence(s, "@I2@", null);
	Sequence* a = ancestorSequence(s, false);
	deleteSequence(s);
	return a;
}
static Sequence* tomsDescendents(Database* database) {
	Sequence* s = createSequence(database);
	appendToSequence(s, "@I1@", null);
	Sequence* d = descendentSequence(s, false);
	deleteSequence(s);
	return d;
}
static Sequence* lusDescendents(Database* database) {
	Sequence* s = createSequence(database);
	appendToSequence(s, "@I2@", null);
	Sequence* d = descendentSequence(s, false);
	deleteSequence(s);
	return d;
}

static void checkTest(String name, int should, int was) {
	printf("TEST: %s: ", name);
	if (should == was) printf("PASSED\n");
	else printf("FAILED: %d != %d\n", should, was);
}
