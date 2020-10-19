#pragma once

#include "dwarf.h"
#include <vector>
#include <map>
#include <string>
#include <sstream>

namespace Cpp
{
struct File;
struct Type;
struct Variable;
struct UserType;
struct ClassType;
struct EnumType;
struct ArrayType;
struct FunctionType;
struct Function;

enum FundamentalType
{
	CHAR               = 0x01,
	SIGNED_CHAR        = 0x02,
	UNSIGNED_CHAR      = 0x03,
	SHORT              = 0x04,
	SIGNED_SHORT       = 0x05,
	UNSIGNED_SHORT     = 0x06,
	INT                = 0x07,
	SIGNED_INT         = 0x08,
	UNSIGNED_INT       = 0x09,
	LONG               = 0x0a,
	SIGNED_LONG        = 0x0b,
	UNSIGNED_LONG      = 0x0c,
	FLOAT              = 0x0e,
	DOUBLE             = 0x0f,
	LONG_DOUBLE        = 0x10,
	VOID               = 0x14,
	BOOL               = 0x15,
	LONG_LONG          = 0x8008,
	SIGNED_LONG_LONG   = 0x8108,
	UNSIGNED_LONG_LONG = 0x8208
};

struct File
{
	std::string filename;
	std::vector<Variable> variables;
	std::vector<UserType*> userTypes;
	std::vector<Function> functions;

	std::string toString(bool justUserTypes, bool includeComments);
};

struct Type
{
	enum Modifier
	{
		CONST = 0x0,
		POINTER_TO = 0x1,
		REFERENCE_TO = 0x2,
		VOLATILE = 0x3
	};

	bool isFundamentalType;
	std::vector<Modifier> modifiers;

	union
	{
		FundamentalType fundamentalType;
		UserType *userType;
	};

	int size();
	std::string toString();
	static std::string ModifierToString(Modifier m);
};

struct Variable
{
	std::string name;
	bool isGlobal;
	Type type;

	std::string toString();
};

struct UserType
{
	enum { CLASS, UNION, STRUCT, ENUM, ARRAY, FUNCTION } type;
	std::string name;
	int index;

	union
	{
		ClassType *classData;
		EnumType *enumData;
		ArrayType *arrayData;
		FunctionType *functionData;
	};

	std::string toDeclarationString();
	std::string toDefinitionString(bool includeComments);
	std::string toNameString(bool includeSize, bool includeInheritances);
};

struct ClassType
{
	struct Member
	{
		int offset;
		std::string name;
		Type type;
		int bit_offset;
		int bit_size;

		std::string toString(bool includeOffset);
	};

	struct Inheritance
	{
		int offset;
		Type type;
	};

	UserType* parent;
	int size;
	std::vector<Member> members;
	std::vector<Inheritance> inheritances;
	std::vector<Function> functions;

	std::string toNameString(std::string name, bool includeSize, bool includeInheritances);
	std::string toBodyString(bool includeOffsets);
	bool isUnion();
};

struct EnumType
{
	struct Element
	{
		std::string name;
		long constValue;

		std::string toString(int lastValue);
	};

	FundamentalType baseType;
	std::vector<Element> elements;

	std::string toNameString(std::string name);
	std::string toBodyString();
};

struct ArrayType
{
	struct Dimension
	{
		int size;
	};

	Type type;
	std::vector<Dimension> dimensions;

	std::string toNameString(std::string name);
};

struct FunctionType
{
	struct Parameter
	{
		std::string name;
		Type type;

		std::string toString();
	};

	Type returnType;
	std::vector<Parameter> parameters;

	std::string toNameString(std::string name);
	std::string toParametersString();
};

struct Function : FunctionType
{

	bool isGlobal;
	std::string name;
	std::string mangledName;
	unsigned int startAddress;
	std::vector<Variable> variables;
	UserType* typeOwner;
	Dwarf* dwarf;

	std::string toNameString();
	std::string toNameString(bool skipNamespace);
	std::string toDeclarationString();
	std::string toDefinitionString();
};

std::string FundamentalTypeToString(FundamentalType ft);
int GetFundamentalTypeSize(FundamentalType ft);
std::string CommentToString(std::string comment);
std::string StarCommentToString(std::string comment, bool multiline);
std::string IndentToString(int level);
}