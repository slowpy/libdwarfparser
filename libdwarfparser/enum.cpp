#include "enum.h"
#include "dwarfparser.h"
#include "dwarfexception.h"

#include <string>
#include <iostream>

#include "helpers.h"

#include <libdwarf/dwarf.h>
#include <libdwarf/libdwarf.h>

Enum::Enum(DwarfParser *parser, const Dwarf_Die &object, const std::string &name):
	BaseType(parser, object, name), enumValues(), enumMutex(){
}

Enum::~Enum(){}

void Enum::addEnum(DwarfParser *parser, const Dwarf_Die &object, const std::string &name){
//TODO get ENUM Value
	uint32_t enumValue = parser->getDieAttributeNumber(object, DW_AT_const_value);

	enumMutex.lock();
	enumValues[enumValue] = name;
	enumMutex.unlock();
}

std::string Enum::enumName(uint32_t value){
	EnumValues::const_iterator it;
	it = enumValues.find(value);	
	if(it != enumValues.end()) { return it->second; }
	throw DwarfException("Enum value not found");
}

uint32_t Enum::enumValue(const std::string &name){
	for (auto& it : enumValues)	{
		if (it.second == name){
			return it.first;
		}
	}
	throw DwarfException("Enum name not found");
}

void Enum::printEnumMembers(std::ostream &stream){
	for (auto& it : enumValues)	{
		stream << it.second << ": " << it.first << std::endl;
	}
}
