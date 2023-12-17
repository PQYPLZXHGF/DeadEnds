//
//  DeadEnds
//
//  validate.c -- Functions that validate Gedcom records.
//
//  Created by Thomas Wetmore on 12 April 2023.
//  Last changed on 17 December 2023.
//

#include "validate.h"
#include "gnode.h"
#include "gedcom.h"
#include "recordindex.h"
#include "lineage.h"
#include "errors.h"

bool validateDatabase(Database*, ErrorLog*);

static bool debugging = true;

static bool validatePersonIndex(Database*, ErrorLog*);
static bool validateFamilyIndex(Database*, ErrorLog*);
static bool validatePerson(GNode*, Database*, ErrorLog*);
static bool validateFamily(GNode*, Database*, ErrorLog*);
static void validateSource(GNode*, Database*, ErrorLog*);
static void validateEvent(GNode*, Database*, ErrorLog*);
static void validateOther(GNode*, Database*, ErrorLog*);

//  personLineNumber -- Given a person root node, return its location (line number) in the
//    original Gedcom file. Uses searchHashTable to get the RecordIndexEl of the record and
//    takes the line number from there. Obviously won't work correctly for records that
//    arrived from another source.
//--------------------------------------------------------------------------------------------------
int personLineNumber (GNode *person, Database* database)
{
	RecordIndexEl *element = searchHashTable(database->personIndex, person->key);
	if (!element) return 0;  // Should not happen.
	return element->lineNumber;  // Assume this is zero for records not from Gedcom files.
}

int numValidations = 0;  //  DEBUG.

//  validateDatabase
//--------------------------------------------------------------------------------------------------
bool validateDatabase(Database *database, ErrorLog *errorLog)
{
	ASSERT(database);
	validatePersonIndex(database, errorLog);
	//validateFamilyIndex(database, errorLog);
	//if (!validateIndex(database->sourceIndex)) isOkay = false;
	//if (!validateIndex(database->eventIndex)) isOkay = false;
	//if (!validateIndex(database->otherIndex)) isOkay = false;
	//return isOkay;
	if (lengthList(errorLog)) {
		printf("THERE WERE ERRORS DURING VALIDATION:\n");
		showErrorLog(errorLog);
		return false;
	}
	return true;
}

//  validatePersonIndex -- Validate the person index of the current database.
//-------------------------------------------------------------------------------------------------
bool validatePersonIndex(Database *database, ErrorLog *errorLog)
{
	bool valid = true;
	FORHASHTABLE(database->personIndex, element)
		GNode* person = ((RecordIndexEl*) element)->root;
		if (!validatePerson(person, database, errorLog)) valid = false;
	ENDHASHTABLE
	return valid;
}

//  validateFamilyIndex -- Validate the family index of the current database.
//-------------------------------------------------------------------------------------------------
bool validateFamilyIndex(Database *database, ErrorLog *errorLog)
{
	bool valid = true;
	FORHASHTABLE(database->familyIndex, element)
		GNode *family = ((RecordIndexEl*) element)->root;
		if (!validateFamily(family, database, errorLog)) valid = false;
	ENDHASHTABLE
	return valid;
}

extern String nameString(String);

//  validatePerson -- Validate a person record. Persons do not require NAME or SEX lines, but
//    if there is a SEX line its value is checked. Check all FAMC and FAMS links to families,
//    and that the families have the correct links back.
//--------------------------------------------------------------------------------------------------
static bool validatePerson(GNode *person, Database *database, ErrorLog *errorLog)
{
	if (debugging) printf("Validating %s %s\n", person->key, NAME(person)->value);
	String segment = database->lastSegment;
	int errorCount = 0;
	static char s[4096];

	//  Make sure all FAMC and FAMS values link to families.
	FORFAMCS(person, family, key, database)
		if (!family) {
			int lineNumber = personLineNumber(person, database);
			sprintf(s, "INDI %s (line %d): FAMC %s (line %d) does not refer to a family.",
					person->key, lineNumber, key, lineNumber + countNodesBefore(__node));
			addErrorToLog(errorLog, createError(linkageError, segment, 0, s));
			errorCount++;
		}
	ENDFAMCS
	FORFAMSS(person, family, key, database)
		if (!family) {
				int lineNumber = personLineNumber(person, database);
				sprintf(s, "INDI %s (line %d): FAMS %s (line %d) does not refer to a family.",
						person->key, lineNumber, key, lineNumber + countNodesBefore(__node));
				addErrorToLog(errorLog, createError(linkageError, segment, 0, s));
			errorCount++;
		}
	ENDFAMSS
	if (errorCount) return false;

	// Loop through the families the person is a child in. Be sure the family has a CHIL link back.
	FORFAMCS(person, family, key, database)
		if (debugging) printf("Person is a child in family %s.\n", key);
		int numOccurrences = 0;
		//  Loop through the children in a family the person should be a child in.
		FORCHILDREN(family, child, count, database)
			if (debugging) { printf("    Child %d: %s %s\n", count, child->key, NAME(child)->value); }
			if (person == child) numOccurrences++;
		ENDCHILDREN
		if (numOccurrences == 0) {
			addErrorToLog(errorLog, createError(linkageError, segment, 0, "Child not found"));
			errorCount++;
		} else if (numOccurrences > 1) {
			addErrorToLog(errorLog, createError(linkageError, segment, 0, "Too many children found"));
			errorCount++;
		}
	ENDFAMCS
	if (errorCount) return false;

	// Loop through the families the person is a spouse in. Be sure the family has a HUSB or WIFE
	// link back.
	if (debugging) printf("Doing the FAMS part.\n");
	SexType sex = SEXV(person);
	FORFAMILIES(person, family, database) {
		if (debugging) printf("  person should be a spouse in family %s.\n", family->key);
		GNode *parent = null;
		if (sex == sexMale) {
			parent = familyToHusband(family, database);
		} else if (sex == sexFemale) {
			parent = familyToWife(family, database);
		} else {
			addErrorToLog(errorLog, createError(linkageError, segment, 0, "Person used as spouse has no sex."));
			errorCount++;
		}
		if (person != parent) {
			addErrorToLog(errorLog, createError(linkageError, segment, 0, "Family does not link back to spouse."));
			errorCount++;
		}
	} ENDFAMILIES

	//  Validate existance of NAME and SEX lines.
	//  Find all other links in the record and validate them.
	return errorCount == 0;
}

//  validateFamily -- Validate a family node tree record. Check all HUSB, WIFE and CHIL links
//    to persons.
//--------------------------------------------------------------------------------------------------
static bool validateFamily(GNode *family, Database *database, ErrorLog* errorLog)
{
	if (debugging) printf("validateFamily(%s)\n", family->key);
	String segment = database->lastSegment;
	// For each HUSB line in the family (multiples in non-traditional cases).
	FORHUSBS(family, husband, database)
		// The husband must have one FAMS link back to this family.
		int numOccurences = 0;
		FORFAMSS(husband, fam, key, database)
			numValidations++;
			if (family == fam) numOccurences++;
		ENDFAMSS
		ASSERT(numOccurences == 1);
	ENDHUSBS

	//  For each WIFE line in the family (multiples in non-traditional cases)...
	FORWIFES(family, wife, database) {
		int numOccurences = 0;
		FORFAMSS(wife, fam, key, database) {
			numValidations++;
			if (family == fam) numOccurences++;
		} ENDFAMSS
		ASSERT(numOccurences == 1);
	} ENDWIFES

	//  For each CHIL node in the family.
	FORCHILDREN(family, child, n, database)
		int numOccurences = 0;
		FORFAMCS(child, fam, key, database)
			numValidations++;
			if (family == fam) numOccurences++;
		ENDFAMCS
		ASSERT(numOccurences == 1);
	ENDFAMCS

	bool hasHusb = HUSB(family) != null;
	bool hasWife = WIFE(family) != null;
	bool hasChild = CHIL(family) != null;
	if (!(hasHusb || hasWife || hasChild)) {
		addErrorToLog(errorLog, createError(linkageError, segment, 0, "Family has no HUSB, WIFE or CHIL links"));
	}
	//printf("validate family: %s\n", family->gKey);
	//  Validate existance of at least one of HUSB, WIFE, CHIL.
	//  Validate that the HUSBs are male.
	//  Validate that the WIFEs are female.
	//  Validate all other links.
	return true;  // TODO: Deal with the errors properly.
}

void validateSource(GNode *source, Database *database, ErrorLog* errorLog) {}

void validateEvent(GNode *event, Database *database, ErrorLog* errorLog) {}

void validateOther(GNode *other, Database *database, ErrorLog* errorLog) {}


static GNode *getFamily(String key, RecordIndex *index)
{
	GNode *root = searchRecordIndex(index, key);
	return root && recordType(root) == GRFamily ? root : null;
}

static GNode *getPerson(String key, RecordIndex *index)
{
	GNode *root = searchRecordIndex(index, key);
	return root && recordType(root) == GRPerson ? root : null;
}
