#ifndef _REFERENCINGTYPE_H_
#define _REFERENCINGTYPE_H_

#include <libdwarf/libdwarf.h>

#include "basetype.h"

#include "dwarfparser.h"

class SymbolManager;

class ReferencingType {
public:
	ReferencingType(SymbolManager *mgr,
	                DwarfParser *parser,
	                const Dwarf_Die &object);
	virtual ~ReferencingType();

	BaseType *getBaseType();

protected:
	uint64_t type;
	BaseType *base;

	SymbolManager *const manager;
};

#endif /* _REFERENCINGTYPE_H_ */
