// DeadEnds
//
// addtofamily.c has functions to add existing children and spouses to an existing family.
//
// Created by Thomas Wetmore on 30 May 2024.
// Last changed on 30 May 2024.

#include "stdlib.h"
#include "splitjoin.h"
#include "gnode.h"
#include "gedcom.h"

// addChildToFamily adds an existing child to an existing family in a Database.
bool addChildToFamily (GNode *child, GNode *family, int index, Database *database) {
	// Add CHIL link to family.
	GNode *frefn, *husb, *wife, *chil, *rest;
	splitFamily(family, &frefn, &husb, &wife, &chil, &rest);
	int numChildren = gNodesLength(chil);
	if (index < 0 || index > numChildren) index = numChildren;
	GNode* prev = null;
	GNode* node = chil;
	int j = 0;
	while (j++ < index) {
		prev = node;
		node = node->sibling;
	}
	GNode* new = createGNode(null, "CHIL", child->key, family);
	new->sibling = node;
	if (prev)
		prev->sibling = new;
	else
		chil = new;
	joinFamily(family, frefn, husb, wife, chil, rest);
	// Add FAMC link to child.
	GNode *names, *irefns, *sex, *body, *famcs, *famss;
	splitPerson(child, &names, &irefns, &sex, &body, &famcs, &famss);
	GNode *nfmc = createGNode(null, "FAMC", family->key, child);
	prev = null;
	GNode *this = famcs;
	while (this) {
		prev = this;
		this = this->sibling;
	}
	if (!prev)
		famcs = nfmc;
	else
		prev->sibling = nfmc;
	joinPerson(child, names, irefns, sex, body, famcs, famss);

	return true;
}

//  addSpouseToFamily adds an existing spouse to an existing family.
bool addSpouseToFamily (GNode *spouse, GNode *family, SexType sext, Database *database) {
	GNode *frefn, *husb, *wife, *chil, *rest;
	splitFamily(family, &frefn, &husb, &wife, &chil, &rest);
	//int numChildren = gNodesLength(chil);
	GNode *prev = null;
	GNode *this = null;
	if (sext == sexMale) {
		prev = null;
		this = husb;
		while (this) {
			prev = this;
			this = this->sibling;
		}
		GNode *new = createGNode(NULL, "HUSB", spouse->key, family);
		if (prev)
			prev->sibling = new;
		else
			husb = new;
	} else {
		prev = null;
		this = wife;
		while (this) {
			prev = this;
			this = this->sibling;
		}
		GNode *new = createGNode(NULL, "WIFE", spouse->key, family);
		if (prev)
			prev->sibling = new;
		else
			wife = new;
	}
	joinFamily(family, frefn, husb, wife, chil, rest);

	// Add a FAMS node to the spouse.
	GNode *names, *irefns, *sex, *body, *famcs, *famss;
	splitPerson(spouse, &names, &irefns, &sex, &body, &famcs, &famss);
	GNode *nfams = createGNode(NULL, "FAMS", family->key, spouse);

	prev = null;
	this = famss;
	while (this) {
		prev = this;
		this = this->sibling;
	}
	if (!prev)
		famss = nfams;
	else
		prev->sibling = nfams;
	joinPerson(spouse, names, irefns, sex, body, famcs, famss);
	return true;
}

static int numChildrenInFamily(GNode* firstChild) {
	int count = 0;
	GNode* child = firstChild;
	while (child) {
		count++;
		child = child->sibling;
	}
	return count;
}

