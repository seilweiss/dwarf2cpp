#include "elf.h"
#include "dwarf.h"
#include "cpp.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>

namespace filesystem = std::experimental::filesystem;

std::vector<Cpp::File*> cppFiles;
std::map<Dwarf::Entry*, Cpp::UserType*> entryUTPairs;
std::map<std::string, std::vector<Cpp::UserType*>> nameUTListPairs;

int currentCompileUnitIndex = 0;

Cpp::File* findCppFile(Dwarf::Entry *entry, const char **outFilename);
void fixUserTypeNames();

bool processDwarf(Dwarf *dwarf);
bool processCompileUnit(Dwarf::Entry *entry, Cpp::File *cpp);
bool processVariable(Dwarf::Entry *entry, Cpp::Variable *var);
bool processTypeAttr(Dwarf::Attribute *attr, Cpp::Type *type);
bool processLocationAttr(Dwarf::Attribute *attr, int *location);
bool findUserType(Dwarf *dwarf, Elf32_Off ref, Cpp::UserType **u);
bool processUserType(Dwarf::Entry *entry, Cpp::UserType *u);
bool processClassType(Dwarf::Entry *entry, Cpp::ClassType *c);
bool processMember(Dwarf::Entry *entry, Cpp::ClassType::Member *m);
bool processInheritance(Dwarf::Entry *entry, Cpp::ClassType::Inheritance *i_);
bool processEnumType(Dwarf::Entry *entry, Cpp::EnumType *e);
bool processElementList(Dwarf::Attribute *attr, Cpp::EnumType *e, int byte_size);
bool processFunctionType(Dwarf::Entry *entry, Cpp::FunctionType *f);
bool processParameter(Dwarf::Entry *entry, Cpp::FunctionType::Parameter *p);
bool processFunction(Dwarf::Entry *entry, Cpp::Function *f);
bool processLexicalBlock(Dwarf::Entry *entry, Cpp::Function *f);
bool processArrayType(Dwarf::Entry *entry, Cpp::ArrayType *a);
bool processSubscriptData(Dwarf::Attribute *attr, Cpp::ArrayType *a);
void replaceChar(char *str, char ch, char newCh);

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cout << "Usage: dwarf2cpp <input ELF file> <output directory>";
		return 1;
	}

	char *elfFilename = argv[1];
	char *outDirectory = argv[2];

	std::cout << "Loading ELF file " << elfFilename << "..." << std::endl;

	ElfFile *elf = new ElfFile(elfFilename);

	if (elf->getError())
		return 1;

	std::cout << "Loading DWARFv1 information..." << std::endl;

	Dwarf *dwarf = new Dwarf(elf);

	if (dwarf->getError())
		return 1;

	std::cout << "Converting DWARFv1 entries to C++ data..." << std::endl;

	if (!processDwarf(dwarf))
		return 1;

	std::cout << "Done converting DWARFv1 data!" << std::endl;
	std::cout << "\tNumber of C++ files: " << cppFiles.size() << std::endl << std::endl;

	for (Cpp::File *cpp : cppFiles)
	{
		size_t pos;
		while ((pos = cpp->filename.find("\\")) != std::string::npos)
		{
			cpp->filename.replace(pos, 1, "/");
		}

		filesystem::path filename(cpp->filename);
		filesystem::path path(outDirectory);

		path /= filename.relative_path();
		path = path.make_preferred();

		filesystem::create_directories(path.parent_path());

		std::cout << "Writing file " << path << "..." << std::endl;

		std::ofstream file(path);
		file << cpp->toString(false, false);
		file.close();
	}

	std::cout << "Done." << std::endl;

	return 0;
}

Cpp::File* findCppFile(Dwarf::Entry *entry, const char **outFilename)
{
	*outFilename = nullptr;

	for (int i = 0; i < entry->numAttributes; i++)
	{
		Dwarf::Attribute *attr = &entry->attributes[i];

		if (attr->name == DW_AT_name)
		{
			*outFilename = attr->getString();
			break;
		}
	}

	if (*outFilename)
	{
		for (Cpp::File *cpp : cppFiles)
		{
			if (cpp->filename == *outFilename)
				return cpp;
		}
	}

	return nullptr;
}

void fixUserTypeNames()
{
	for (auto const &x : nameUTListPairs)
	{
		bool noname = x.first.empty();
		bool duplicate = x.second.size() > 1;

		if (noname || duplicate)
		{
			for (size_t i = 0; i < x.second.size(); i++)
			{
				Cpp::UserType *ut = x.second[i];

				if (noname)
					ut->name = "type";

				if (duplicate)
					ut->name += "_" + std::to_string(i);
			}
		}
	}
}

bool processDwarf(Dwarf *dwarf)
{
	Dwarf::Entry *entry = dwarf->entries;
	Dwarf::Entry *end = dwarf->entries + dwarf->numEntries;

	while (entry && entry < end)
	{
		switch (entry->tag)
		{
		case DW_TAG_compile_unit:
		{
			const char *filename;
			Cpp::File *cpp = findCppFile(entry, &filename);

			bool found = (cpp != nullptr);

			if (!found)
			{
				cpp = new Cpp::File;
				cpp->filename = filename;
			}

			if (!processCompileUnit(entry, cpp))
				return false;

			if (!found)
				cppFiles.push_back(cpp);

			//std::cout << "Found compile unit " << cpp->filename << std::endl;
			//std::cout << "\t" << std::to_string(cpp->userTypes.size()) << " user types" << std::endl;
			//std::cout << "\t" << std::to_string(cpp->variables.size()) << " variables" << std::endl;

			break;
		}
		}

		entry = entry->getSibling();
	}

	return true;
}

bool processCompileUnit(Dwarf::Entry *entry, Cpp::File *cpp)
{
	nameUTListPairs.clear();

	Dwarf::Entry *next = entry->getSibling();

	for (int i = 0; i < entry->numAttributes; i++)
	{
		Dwarf::Attribute *attr = &entry->attributes[i];

		switch (attr->name)
		{
		case DW_AT_name:
			cpp->filename = attr->getString();
			break;
		}
	}

	Dwarf::Entry *start = ++entry;

	while (entry && entry < next)
	{
		switch (entry->tag)
		{
		case DW_TAG_class_type:
		case DW_TAG_structure_type:
		case DW_TAG_enumeration_type:
		case DW_TAG_array_type:
		case DW_TAG_subroutine_type:
		case DW_TAG_union_type:
		{
			entryUTPairs[entry] = new Cpp::UserType;
		}
		}

		entry = entry->getSibling();
	}

	entry = start;

	while (entry && entry < next)
	{
		switch (entry->tag)
		{
		case DW_TAG_global_variable:
		case DW_TAG_local_variable:
		{
			Cpp::Variable var;

			if (!processVariable(entry, &var))
				return false;

			cpp->variables.push_back(var);
			break;
		}
		case DW_TAG_class_type:
		case DW_TAG_structure_type:
		case DW_TAG_enumeration_type:
		case DW_TAG_array_type:
		case DW_TAG_subroutine_type:
		case DW_TAG_union_type:
		{
			Cpp::UserType *userType = entryUTPairs[entry];
			processUserType(entry, userType);

			userType->index = cpp->userTypes.size();
			cpp->userTypes.push_back(userType);

			nameUTListPairs[userType->name].push_back(userType);
			break;
		}
		case DW_TAG_global_subroutine:
		case DW_TAG_subroutine:
		case DW_TAG_inlined_subroutine:
		{
			Cpp::Function f;
			f.dwarf = entry->dwarf;

			if (!processFunctionType(entry, &f))
				return false;

			if (!processFunction(entry, &f))
				return false;

			cpp->functions.push_back(f);
		}
		}

		entry = entry->getSibling();
	}

	fixUserTypeNames();

	return true;
}

bool processVariable(Dwarf::Entry *entry, Cpp::Variable *var)
{
	var->isGlobal = (entry->tag == DW_TAG_global_variable);

	for (int i = 0; i < entry->numAttributes; i++)
	{
		Dwarf::Attribute *attr = &entry->attributes[i];

		switch (attr->name)
		{
		case DW_AT_name:
			var->name = attr->getString();
			break;
		case DW_AT_fund_type:
		case DW_AT_user_def_type:
		case DW_AT_mod_fund_type:
		case DW_AT_mod_u_d_type:
			if (!processTypeAttr(attr, &var->type))
				return false;
			break;
		}
	}

	return true;
}

bool processTypeAttr(Dwarf::Attribute *attr, Cpp::Type *type)
{
	Dwarf *dwarf = attr->entry->dwarf;

	switch (attr->name)
	{
	case DW_AT_fund_type:
	{
		type->isFundamentalType = true;
		type->fundamentalType = (Cpp::FundamentalType)attr->getHword();
		break;
	}
	case DW_AT_user_def_type:
	{
		type->isFundamentalType = false;

		if (!findUserType(dwarf, attr->getReference(), &type->userType))
			return false;

		break;
	}
	case DW_AT_mod_fund_type:
	{
		type->isFundamentalType = true;

		char *mod = attr->getBlock();
		char *end = mod + attr->size - sizeof(Elf32_Half);

		type->fundamentalType = (Cpp::FundamentalType)dwarf->read<Elf32_Half>(end);

		while (mod < end)
		{
			type->modifiers.push_back((Cpp::Type::Modifier)*mod);
			mod++;
		}

		break;
	}
	case DW_AT_mod_u_d_type:
	{
		type->isFundamentalType = false;

		char *mod = attr->getBlock();
		char *end = mod + attr->size - sizeof(Elf32_Off);

		if (!findUserType(dwarf, dwarf->read<Elf32_Off>(end), &type->userType))
			return false;

		while (mod < end)
		{
			type->modifiers.push_back((Cpp::Type::Modifier)*mod);
			mod++;
		}

		break;
	}
	}

	return true;
}

bool processLocationAttr(Dwarf::Attribute *attr, int *location)
{
	// I don't really know how location is supposed to be handled,
	// so I just look for a DW_OP_CONST and use that as the "location"

	Dwarf *dwarf = attr->entry->dwarf;

	char *block = attr->getBlock();
	char *end = block + attr->size;

	while (block < end)
	{
		char op = dwarf->read<char>(block);
		block += sizeof(char);

		if (op == DW_OP_CONST)
		{
			*location = dwarf->read<Elf32_Word>(block);
			break;
		}
	}

	return true;
}

bool findUserType(Dwarf *dwarf, Elf32_Off ref, Cpp::UserType **u)
{
	Dwarf::Entry *entry = dwarf->getEntryFromReference(ref);

	if (!entry || entryUTPairs.count(entry) == 0)
		return false;

	*u = entryUTPairs[entry];

	return true;
}

bool processUserType(Dwarf::Entry *entry, Cpp::UserType *userType)
{
	for (int i = 0; i < entry->numAttributes; i++)
	{
		Dwarf::Attribute *attr = &entry->attributes[i];

		switch (attr->name)
		{
		case DW_AT_name:
		{
			char *name = attr->getString();
			replaceChar(name, '@', '_');
			userType->name = name;
			break;
		}
		}
	}

	switch (entry->tag)
	{
	case DW_TAG_class_type:
	case DW_TAG_structure_type:
	case DW_TAG_union_type:
		userType->type = (entry->tag == DW_TAG_structure_type) ? Cpp::UserType::STRUCT : ((entry->tag == DW_TAG_union_type) ? Cpp::UserType::UNION : Cpp::UserType::CLASS);
		userType->classData = new Cpp::ClassType;
		userType->classData->parent = userType;

		if (!processClassType(entry, userType->classData))
			return false;

		break;
	case DW_TAG_enumeration_type:
		userType->type = Cpp::UserType::ENUM;
		userType->enumData = new Cpp::EnumType;

		if (!processEnumType(entry, userType->enumData))
			return false;

		break;
	case DW_TAG_array_type:
		userType->type = Cpp::UserType::ARRAY;
		userType->arrayData = new Cpp::ArrayType;

		if (!processArrayType(entry, userType->arrayData))
			return false;

		break;
	case DW_TAG_subroutine_type:
		userType->type = Cpp::UserType::FUNCTION;
		userType->functionData = new Cpp::FunctionType;

		if (!processFunctionType(entry, userType->functionData))
			return false;

		break;
	}

	return true;
}

bool processClassType(Dwarf::Entry *entry, Cpp::ClassType *c)
{
	c->size = 0;

	for (int i = 0; i < entry->numAttributes; i++)
	{
		Dwarf::Attribute *attr = &entry->attributes[i];

		switch (attr->name)
		{
		case DW_AT_byte_size:
			c->size = attr->getWord();
			break;
		}
	}

	Dwarf::Entry *next = entry->getSibling();
	Dwarf::Entry *first = entry;

	int memberCount = 0;
	entry++;

	while (entry && entry < next)
	{
		if (entry->tag == DW_TAG_member)
			memberCount++;

		entry = entry->getSibling();
	}

	c->members.reserve(memberCount);
	entry = first + 1;

	while (entry && entry < next)
	{
		switch (entry->tag)
		{
		case DW_TAG_member:
		{
			Cpp::ClassType::Member m;

			if (!processMember(entry, &m))
				return false;

			c->members.push_back(m);
			break;
		}
		case DW_TAG_inheritance:
			Cpp::ClassType::Inheritance i;

			if (!processInheritance(entry, &i))
				return false;

			c->inheritances.push_back(i);
			break;
		}

		entry = entry->getSibling();
	}

	return true;
}

bool processMember(Dwarf::Entry *entry, Cpp::ClassType::Member *m)
{
	m->bit_offset = -1;
	m->bit_size = -1;
	for (int i = 0; i < entry->numAttributes; i++)
	{
		Dwarf::Attribute *attr = &entry->attributes[i];

		switch (attr->name)
		{
		case DW_AT_name:
			m->name = attr->getString();
			break;
		case DW_AT_bit_offset:
			m->bit_offset = attr->getHword();
			break;
		case DW_AT_bit_size:
			m->bit_size = attr->getWord();
			break;
		case DW_AT_fund_type:
		case DW_AT_user_def_type:
		case DW_AT_mod_fund_type:
		case DW_AT_mod_u_d_type:
			if (!processTypeAttr(attr, &m->type))
				return false;
			break;
		case DW_AT_location:
			if (!processLocationAttr(attr, &m->offset))
				return false;
		}
	}

	return true;
}

bool processInheritance(Dwarf::Entry *entry, Cpp::ClassType::Inheritance *i_)
{
	for (int i = 0; i < entry->numAttributes; i++)
	{
		Dwarf::Attribute *attr = &entry->attributes[i];

		switch (attr->name)
		{
		case DW_AT_user_def_type:
			if (!processTypeAttr(attr, &i_->type))
				return false;
			break;
		case DW_AT_location:
			if (!processLocationAttr(attr, &i_->offset))
				return false;
		}
	}

	return true;
}

bool processEnumType(Dwarf::Entry *entry, Cpp::EnumType *e)
{
	int byte_size = 0;
	for (int i = 0; i < entry->numAttributes; i++)
	{
		Dwarf::Attribute *attr = &entry->attributes[i];

		switch (attr->name)
		{
		case DW_AT_byte_size:
			byte_size = attr->getWord();

			switch (byte_size) {
			case 1:
				e->baseType = Cpp::FundamentalType::UNSIGNED_CHAR;
				break;
			case 2:
				e->baseType = Cpp::FundamentalType::UNSIGNED_SHORT;
				break;
			case 4:
				e->baseType = Cpp::FundamentalType::INT;
				break;
			case 8:
				e->baseType = Cpp::FundamentalType::LONG;
				break;
			default:
				std::cout << "Unknown enum size! [" << byte_size << "]" << std::endl;
				return false;
				break;
			}
			break;
		case DW_AT_element_list:
			if (!processElementList(attr, e, byte_size))
				return false;
			break;
		}
	}

	return true;
}

bool processElementList(Dwarf::Attribute *attr, Cpp::EnumType *e, int byte_size)
{
	Dwarf *dwarf = attr->entry->dwarf;

	char *block = attr->getBlock();
	char *end = block + attr->size;

	while (block < end)
	{
		Cpp::EnumType::Element element;

		if (byte_size == 1) {
			element.constValue = dwarf->read<unsigned char>(block);
		}
		else if (byte_size == 2) {
			element.constValue = dwarf->read<unsigned short>(block);
		}
		else if (byte_size == 4) {
			element.constValue = dwarf->read<int>(block);
		}
		else if (byte_size == 8) {
			element.constValue = dwarf->read<long>(block);
		}
		
		block += byte_size;

		element.name = block;
		block += element.name.size() + 1;

		e->elements.push_back(element);
	}

	return true;
}

bool processFunctionType(Dwarf::Entry *entry, Cpp::FunctionType *f)
{
	Dwarf::Entry *next = entry->getSibling();
	Dwarf::Entry *first = entry;

	int paramCount = 0;
	entry++;

	while (entry && entry < next)
	{
		if (entry->tag == DW_TAG_formal_parameter)
			paramCount++;

		entry = entry->getSibling();
	}

	f->parameters.reserve(paramCount);
	entry = first;

	for (int i = 0; i < entry->numAttributes; i++)
	{
		Dwarf::Attribute *attr = &entry->attributes[i];

		switch (attr->name)
		{
		case DW_AT_fund_type:
		case DW_AT_user_def_type:
		case DW_AT_mod_fund_type:
		case DW_AT_mod_u_d_type:
			if (!processTypeAttr(attr, &f->returnType))
				return false;
			break;
		}
	}

	entry++;

	while (entry && entry < next)
	{
		switch (entry->tag)
		{
		case DW_TAG_formal_parameter:
			Cpp::FunctionType::Parameter p;

			if (!processParameter(entry, &p))
				return false;

			f->parameters.push_back(p);
		}

		entry = entry->getSibling();
	}

	return true;
}

bool processParameter(Dwarf::Entry *entry, Cpp::FunctionType::Parameter *p)
{
	for (int i = 0; i < entry->numAttributes; i++)
	{
		Dwarf::Attribute *attr = &entry->attributes[i];

		switch (attr->name)
		{
		case DW_AT_name:
			p->name = attr->getString();
			break;
		case DW_AT_fund_type:
		case DW_AT_user_def_type:
		case DW_AT_mod_fund_type:
		case DW_AT_mod_u_d_type:
			if (!processTypeAttr(attr, &p->type))
				return false;
			break;
		}
	}

	return true;
}

bool processFunction(Dwarf::Entry *entry, Cpp::Function *f)
{
	f->isGlobal = (entry->tag == DW_TAG_global_subroutine);

	for (int i = 0; i < entry->numAttributes; i++)
	{
		Dwarf::Attribute *attr = &entry->attributes[i];

		switch (attr->name)
		{
		case DW_AT_name:
			f->name = attr->getString();
			break;
		case DW_AT_mangled_name:
			f->mangledName = attr->getString();
			break;
		case DW_AT_low_pc:
			f->startAddress = attr->getAddress();
			break;
		}
	}

	Dwarf::Entry *next = entry->getSibling();

	entry++;

	while (entry && entry < next)
	{
		switch (entry->tag)
		{
		case DW_TAG_lexical_block:
			if (!processLexicalBlock(entry, f))
				return false;
		}

		entry = entry->getSibling();
	}

	f->typeOwner = nullptr;
	if (f->parameters.size() > 0 && f->parameters[0].name.compare("this") == 0) {
		f->typeOwner = f->parameters[0].type.userType;
		f->parameters.erase(f->parameters.begin());
		f->typeOwner->classData->functions.push_back(*f);
	}
	else if (f->mangledName.size() > 2) {
		int foundAt = f->mangledName.find_last_of("__");
		if (foundAt != -1) {
			char temp;
			std::stringstream length;
			int i;
			for (i = foundAt + 1; i < f->mangledName.size(); i++) {
				temp = f->mangledName[i];
				if (temp >= '0' && temp <= '9') {
					length << temp;
				}
				else {
					break;
				}
			}

			std::string lengthStr = length.str();
			if (lengthStr.length() > 0) {
				int lengthCount = std::stoi(lengthStr);
				if (f->mangledName[i + lengthCount] == 'F') {
					std::string className = f->mangledName.substr(i, lengthCount);

					// I tried to access this from the named map, but I couldn't for the life of me figure out how to do it. C++ is terrible, no other languages have runtime libraries that silently fail like this. The map is empty even though the code that adds elements to the map is run. Good grief.
					/*auto it = nameUTListPairs.find(className);
					if (it != nameUTListPairs.end()) {
						std::vector<Cpp::UserType*> vector = it->second;
						if (vector.size() != 1)
							std::cout << "Couldn't find good type for '" << className << "'. (" << vector.size() << ")" << std::endl;
					}
					else {
						std::cout << "Couldn't find good type for '" << className << "'." << std::endl;
					}*/
						
					
					Cpp::UserType* type = nullptr;
					for (std::map<Dwarf::Entry*, Cpp::UserType*>::iterator iter = entryUTPairs.begin(); iter != entryUTPairs.end(); ++iter)
					{
						Dwarf::Entry* k = iter->first;
						Cpp::UserType* value = iter->second;
						if (value->name.compare(className) == 0) {
							type = value;
							break;
						}
					}

					if (type != nullptr) {
						f->typeOwner = type;
						f->typeOwner->classData->functions.push_back(*f);
					}
				}
			}
		}
	}

	return true;
}

bool processLexicalBlock(Dwarf::Entry *entry, Cpp::Function *f)
{
	Dwarf::Entry *next = entry->getSibling();

	entry++;

	while (entry && entry < next)
	{
		switch (entry->tag)
		{
		case DW_TAG_global_variable:
		case DW_TAG_local_variable:
		{
			Cpp::Variable v;
			
			if (!processVariable(entry, &v))
				return false;

			f->variables.push_back(v);
			break;
		}
		}

		entry = entry->getSibling();
	}

	return true;
}

bool processArrayType(Dwarf::Entry *entry, Cpp::ArrayType *a)
{
	for (int i = 0; i < entry->numAttributes; i++)
	{
		Dwarf::Attribute *attr = &entry->attributes[i];

		switch (attr->name)
		{
		case DW_AT_ordering:
			if (attr->getHword() != DW_ORD_row_major) // meh
				return false;
			break;
		case DW_AT_subscr_data:
			if (!processSubscriptData(attr, a))
				return false;
		}
	}

	return true;
}

bool processSubscriptData(Dwarf::Attribute *attr, Cpp::ArrayType *a)
{
	Dwarf *dwarf = attr->entry->dwarf;

	char *block = attr->getBlock();
	char *end = block + attr->size;

	while (block < end)
	{
		char format = dwarf->read<char>(block);
		block += sizeof(char);

		if (format == DW_FMT_ET)
		{
			Dwarf::Attribute *typeAttr;
			Elf32_Off offset = dwarf->pointerToOffset(block);

			offset = dwarf->readAttribute(offset, attr->entry, &typeAttr);
			block = dwarf->offsetToPointer(offset);

			if (!processTypeAttr(typeAttr, &a->type))
				return false;

			break;
		}
		else if (format == DW_FMT_FT_C_C)
		{
			Elf32_Half fundType = dwarf->read<Elf32_Half>(block);
			block += sizeof(Elf32_Half);

			// Only long indices are supported
			if (fundType != DW_FT_long)
				return false;

			Elf32_Word lowBound = dwarf->read<Elf32_Word>(block);
			block += sizeof(Elf32_Word);

			// Only indices starting at 0 are supported
			if (lowBound != 0)
				return false;

			Elf32_Word highBound = dwarf->read<Elf32_Word>(block);
			block += sizeof(Elf32_Word);

			Cpp::ArrayType::Dimension dimension;
			dimension.size = highBound + 1;

			a->dimensions.push_back(dimension);
		}
		else
		{
			// Only fundamental typed (long) indices and
			// constant value bounds are supported
			return false;
		}
	}

	return true;
}

void replaceChar(char *str, char ch, char newCh)
{
	char *end = str + strlen(str);

	while (str < end)
	{
		if (*str == ch)
			*str = newCh;
		str++;
	}
}
