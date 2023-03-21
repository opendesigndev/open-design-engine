
# This script parses ODE API header files and processes them in various ways

import sys
import os
import re
import json

# Data structures for the parsed entities (struct, enum, function etc.)
class Entity:
    def __init__(self, category, type, namespace, name, description, members):
        self.category = category
        self.type = type
        self.namespace = namespace
        self.name = name
        self.description = description
        self.members = members
    def __repr__(self):
        return f"<Entity category:{self.category} type:{self.type} namespace:{self.namespace} name:{self.name} description:\"{self.description if self.description else ''}\" members:["+('\n'+'\n'.join(["  "+str(x) for x in self.members])+'\n' if self.members else "")+"]>\n"

class Member: # a struct member, enum value, or function argument
    def __init__(self, category, type, name, value, description):
        self.category = category
        self.type = type
        self.name = name
        self.value = value
        self.description = description
    def __repr__(self):
        return str(self.__dict__)

typesUsedAsPtrInAPI = set()



###########################
#         PARSING         #
###########################

def nestedBrackets(left, right, depth):
    nonBrackets = "[^\\"+left+"\\"+right+"]*"
    if depth == 0:
        return nonBrackets
    return nonBrackets+"(\\"+left+nestedBrackets(left, right, depth-1)+"\\"+right+nonBrackets+")*"

# Regular expressions to match parts of C header
reDefine = re.compile(r"^#define[^\S\n]+(ODE_[A-Z0-9_]+)[^\S\n]+([0-9\+\-\.][\w\+\-\.]*|\([^\S\n]*[0-9\+\-\.][\w\+\-\.]*[^\S\n]*\))[^\S\n]*(\/\/.*)?\n")
reConstant = re.compile(r"^extern\s+ODE_API\s+(const\s+[\w\s\*]+[\s\*]|\w[\w\s\*]*[\s\*]const)(\w+)(\s*=.*)?;")
reEnum = re.compile(r"^(typedef\s+|const\s+)?enum\s*(\w*)\s*\{\s*(.*?)\s*\}\s*((\*\s*)?\w*)\s*;", re.DOTALL)
reTypedef = re.compile(r"^typedef\s+(\w[\w\s\*]*[\s\*])\s*(\w+)\s*(\s*\[\s*[0-9]+\s*\])*\s*;")
reHandle = re.compile(r"^ODE_HANDLE_DECL\s*\(\s*(\w+)\s*\)\s*(\w+)\s*;")
reStruct = re.compile(r"^(typedef\s+|const\s+)?struct\s*(\w*)\s*\{\s*("+nestedBrackets('{', '}', 4)+ r")\s*\}\s*((\*\s*)?\w*)\s*;", re.DOTALL) # Increase nesting if necessary
reFunction = re.compile(r"^\n\s*([\w\s\*]+[\s\*])ODE_API\s+(\w+)\s*\(\s*(.*?)\s*\)\s*;", re.DOTALL)
reDescCommentA = re.compile(r"^\/\/\/\s*(.*)")
reDescCommentB = re.compile(r"^\/\*\*\s*(.*?)\s*\*\/", re.DOTALL)
reSkipA = re.compile(r"^(#|\/\/).*") # Directive or inline comment
reSkipB = re.compile(r"^\/\*.*?\*\/", re.DOTALL) # Multi-line comment

reOddBackslashesNL = re.compile(r"([^\\](\\\\)*)\\\n") # An escaped newline
reMultiLineComment = re.compile(r"\/\*.*?\*\/", re.DOTALL)
reEnumValue = re.compile(r"(\w+)\s*(=\s*([\w\.\+\-]+))\s*$")
reFunctionArg = re.compile(r"([\w\s\*]+[\s\*])(\w+)\s*$")
reMixedMember = re.compile(r"(^|[\w\s\*]*[\s\*])\s*(\w+)\s*$", re.DOTALL)
reMemberVariable = re.compile(r"^(\w[\w\s\*]*\w|\w)([\s\*][\w\s\*,]*\w)\s*(\[[\w\s\[\]]*\])?\s*;")
reMemberVariableName = re.compile(r"^(|.*[\s\*])(\w+)$")
reTupleMarker = re.compile(r"^ODE_TUPLE[\s\n]")
reBindConstructor = re.compile(r"^ODE_BIND_CONSTRUCTOR\s*\(\s*\(\s*([\w\s\:\*&\,]*)\s*\)\s*,\s*("+nestedBrackets('(', ')', 6)+r")\s*\)\s*;")
reBindMethod = re.compile(r"^ODE_BIND_METHOD\s*\(\s*([\w\s\:\*&]+)\s*\(\s*([\w\s\:\*&\,]*)\s*\)\s*,\s*(\w+)\s*,\s*("+nestedBrackets('(', ')', 6)+r")\s*\)\s*;")
reBindPtrGetter = re.compile(r"^ODE_BIND_PTR_GETTER\s*\(\s*(\w+)\s*,\s*(\w+)\s*\)\s*;")
reBindArrayGetter = re.compile(r"^ODE_BIND_ARRAY_GETTER\s*\(\s*(\w+)\s*,\s*(\w+)\s*,\s*(\w+)\s*\)\s*;")
rePtrType = re.compile(r"([A-Za-z]\w*)(\s+const)?\s*\*")
reParamDesc = re.compile(r"^param\s*(\w+)\s*\-?\s*")
reArrayType = re.compile(r"\[\s*([0-9]+)\s*\]\s*$")
reArgAttrib = re.compile(r"^\s*(ODE_(IN|OUT|INOUT|OUT_RETURN))\s")
reReturnArg = re.compile(r"^\s*ODE_OUT_RETURN\s")

# Split string but ignore separators within comments
def safeSplit(s, separator):
    # Remove escaped newlines
    s = reOddBackslashesNL.sub(r"\1", s)
    # Escape the '$' character, which will be used in our custom escape sequences
    s = s.replace('$', "$D")
    # Sanitize separators within inline comments
    lines = s.split('\n')
    for i in range(len(lines)):
        if "//" in lines[i]:
            line, comment = lines[i].split("//", 1)
            lines[i] = line+"//"+comment.replace(separator, "$S")
    s = '\n'.join(lines)
    # Sanitize separators within multi-line comments
    s = reMultiLineComment.sub(lambda m: m.group(0).replace(separator, "$S"), s)
    # Split, unescape, and trim whitespace
    elems = [x.replace("$S", separator).replace("$D", '$').strip() for x in s.split(separator)]
    # Remove the last element if it is empty
    if elems and not elems[-1]:
        elems.pop()
    return elems

def namespacedName(namespace, name):
    fullName = ""
    for ns in namespace:
        fullName += ns+"::"
    fullName += name
    return fullName

def guessNumberType(value):
    value = value.lower()
    if "lf" in value:
        return "double"
    elif "f" in value and not "x" in value:
        return "float"
    elif "." in value or ("e" in value and not "x" in value):
        return "double"
    elif "ull" in value:
        return "unsigned long long"
    elif "ll" in value:
        return "long long"
    elif "u" in value:
        return "unsigned"
    return "int"

def updateTypesUsedAsPtrInAPI(apiArgType):
    for match in rePtrType.finditer(apiArgType):
        typesUsedAsPtrInAPI.add(match.group(1))

# Reconstructs the full type of a nested enum/struct and member variable in one declaration
def mixedMemberType(prefix, namespace, name, suffix):
    return ((prefix.strip()+' ' if prefix and not prefix.startswith("typedef") else "")+namespacedName(namespace, name)+' '+suffix.strip()).replace("* ", '*').strip()

# Prunes extra whitespace and decorating asterisks from description comments
def cleanupDescription(s):
    lines = s.strip().split('\n')
    for i in range(len(lines)):
        lines[i] = lines[i].strip()
        if lines[i].startswith('*'):
            lines[i] = lines[i][1:].strip()
    return '\n'.join(lines).strip()

# Extracts descriptions of arguments (@param x - desc) from a description comment
def extractArgumentDescription(desc, argMap):
    parts = desc.split('@')
    nonArgParts = []
    if parts:
        nonArgParts.append(parts[0])
    for part in parts[1:]:
        if (match := reParamDesc.search(part)):
            argName = match.group(1)
            argDesc = part[match.end():].strip()
            argMap[argName] = argDesc
        else:
            nonArgParts.append(part)
    return '@'.join(nonArgParts).strip()

# Parse description from comments
def parseDescription(s):
    if (match := reDescCommentA.search(s)):
        return cleanupDescription(match.group(1))
    if (match := reDescCommentB.search(s)):
        return cleanupDescription(match.group(1))
    return None

# Enumeration parser
def parseEnum(groups):
    # Determine name
    name = None
    if groups[0] and groups[0].startswith("typedef"):
        name = groups[3].strip()
    if not name:
        name = groups[1].strip()
    if not name:
        return None
    # Split enumeration values
    rawValues = safeSplit(groups[2], ',')
    # Inspect enumeration values and transform them into members
    values = []
    for rawValue in rawValues:
        if (match := reEnumValue.search(rawValue)):
            valueName = match.group(1).strip()
            valueValue = match.group(3).strip()
            desc = parseDescription(rawValue)
            values.append(Member("enum_value", None, valueName, valueValue, desc))
    return Entity("enum", None, [], name, None, values)

# Function header parser
def parseFunction(groups, desc):
    # Extract argument description
    argDescMap = {}
    desc = extractArgumentDescription(desc, argDescMap)
    # Determine return type and function's name
    returnType = groups[0].strip()
    name = groups[1].strip()
    if not (name and returnType):
        return None
    # Split arguments
    rawArgs = safeSplit(groups[2], ',')
    args = []
    for rawArg in rawArgs:
        if (match := reFunctionArg.search(rawArg)):
            argType = match.group(1).strip()
            argName = match.group(2).strip()
            argDesc = parseDescription(rawArg)
            if not argDesc and argName in argDescMap:
                argDesc = argDescMap[argName]
            args.append(Member("argument", argType, argName, None, argDesc))
            updateTypesUsedAsPtrInAPI(argType)
    return Entity("function", returnType, [], name, desc, args)

# Structure parser - may contain nested structs and enums
def parseStruct(entities, groups, desc, namespace = []):
    category = "struct"
    # Determine name
    name = None
    if groups[0] and groups[0].startswith("typedef"):
        name = groups[-2].strip()
    if not name:
        name = groups[1].strip()
    if not name:
        return None
    childNamespace = namespace+[ name ]
    # Parse body
    body = groups[2]
    memberDesc = None
    members = []
    while body:

        # ODE_TUPLE marker
        if (match := reTupleMarker.search(body)):
            category = "tuple"
            memberDesc = None
            body = body[match.end():]

        # ODE_BIND_CONSTRUCTOR
        elif (match := reBindConstructor.search(body)):
            api = '('+','.join([x.strip() for x in match.group(1).split(',')])+')'
            members.append(Member("constructor_bind", api, match.group(2).strip(), None, memberDesc))
            memberDesc = None
            body = body[match.end():]

        # ODE_BIND_METHOD
        elif (match := reBindMethod.search(body)):
            api = match.group(1).strip()+'('+','.join([x.strip() for x in match.group(2).split(',')])+')'
            members.append(Member("method_bind", api, match.group(3).strip(), match.group(4).strip(), memberDesc))
            memberDesc = None
            body = body[match.end():]

        # ODE_BIND_PTR_GETTER
        elif (match := reBindPtrGetter.search(body)):
            members.append(Member("ptr_getter_bind", None, match.group(1).strip(), match.group(2).strip(), memberDesc))
            memberDesc = None
            body = body[match.end():]

        # ODE_BIND_ARRAY_GETTER
        elif (match := reBindArrayGetter.search(body)):
            members.append(Member("array_getter_bind", None, match.group(1).strip(), match.group(2).strip()+','+match.group(3).strip(), memberDesc))
            memberDesc = None
            body = body[match.end():]

        # Enumeration
        elif (match := reEnum.search(body)):
            if (entity := parseEnum(match.groups())):
                entity.namespace = childNamespace
                entity.description = memberDesc
                entities.append(entity)
                # Also a variable
                if not (match.group(1) and match.group(1).startswith("typedef")):
                    if (subMatch := reMixedMember.search(match.group(4))):
                        members.append(Member("member_variable", mixedMemberType(match.group(1), childNamespace, entity.name, subMatch.group(1)), subMatch.group(2).strip(), None, memberDesc))
            memberDesc = None
            body = body[match.end():]

        # Struct
        elif (match := reStruct.search(body)):
            if (entity := parseStruct(entities, match.groups(), memberDesc, childNamespace)):
                entities.append(entity)
                # Also a variable
                if not (match.group(1) and match.group(1).startswith("typedef")):
                    if (subMatch := reMixedMember.search(match.groups()[-2])):
                        members.append(Member("member_variable", mixedMemberType(match.group(1), childNamespace, entity.name, subMatch.group(1)), subMatch.group(2).strip(), None, memberDesc))
            memberDesc = None
            body = body[match.end():]

        # Member variable
        elif (match := reMemberVariable.search(body)):
            for rawMemberVariable in match.group(2).split(','):
                if (subMatch := reMemberVariableName.search(rawMemberVariable)):
                    varType = match.group(1).strip()
                    if subMatch.group(1).strip(): # pointer / reference
                        varType = varType+' '+subMatch.group(1).strip()
                    if match.group(3): # array suffix
                        arrayInstanceName = varType+"_array"+'_'.join([x.replace(']', '').strip() for x in match.group(3).split('[')])
                        varType = varType+match.group(3).strip()
                        entities.append(Entity("array_instance", varType, [], arrayInstanceName, None, []))
                        varType = arrayInstanceName
                    varName = subMatch.group(2).strip()
                    # TODO remember nested structures and enum names and add namespace to type if variable type is one of them
                    members.append(Member("member_variable", varType, varName, None, memberDesc))
            memberDesc = None
            body = body[match.end():]

        # Description comments
        elif (match := reDescCommentA.search(body)):
            memberDesc = cleanupDescription(match.group(1))
            body = body[match.end():]
        elif (match := reDescCommentB.search(body)):
            memberDesc = cleanupDescription(match.group(1))
            body = body[match.end():]

        # Skip directives and normal comments
        elif (match := reSkipA.search(body)):
            memberDesc = None
            body = body[match.end():]
        elif (match := reSkipB.search(body)):
            memberDesc = None
            body = body[match.end():]

        else:
            body = body[1:]

    return Entity(category, None, namespace, name, desc, members)

# Whole header parser
def parseHeader(header):
    entities = []
    desc = None
    header = reOddBackslashesNL.sub(r"\1", header)
    while header:

        # Define
        if (match := reDefine.search(header)):
            name = match.group(1).strip()
            type = guessNumberType(match.group(2))
            if type and name:
                entities.append(Entity("define", type, [], name, desc, []))
            desc = None
            header = header[match.end():]

        # Constant
        elif (match := reConstant.search(header)):
            type = match.group(1).strip()
            name = match.group(2).strip()
            if type and name:
                entities.append(Entity("const", type, [], name, desc, []))
            desc = None
            header = header[match.end():]

        # Enumeration
        elif (match := reEnum.search(header)):
            if (entity := parseEnum(match.groups())):
                entity.description = desc
                entities.append(entity)
            desc = None
            header = header[match.end():]

        # Typedef
        elif (match := reTypedef.search(header)):
            type = match.group(1).strip()
            if match.group(3):
                type = type+match.group(3).strip()
            name = match.group(2).strip()
            if type and name:
                entities.append(Entity("typedef", type, [], name, desc, []))
            desc = None
            header = header[match.end():]

        # Handle
        elif (match := reHandle.search(header)):
            type = match.group(1).strip()
            name = match.group(2).strip()
            if type and name:
                entities.append(Entity("handle", type, [], name, desc, []))
            desc = None
            header = header[match.end():]

        # Struct
        elif (match := reStruct.search(header)):
            if (entity := parseStruct(entities, match.groups(), desc)):
                entities.append(entity)
            desc = None
            header = header[match.end():]

        # Function
        elif (match := reFunction.search(header)):
            if (entity := parseFunction(match.groups(), desc)):
                entities.append(entity)
            desc = None
            header = header[match.end():]

        # Description comments
        elif (match := reDescCommentA.search(header)):
            desc = cleanupDescription(match.group(1))
            header = header[match.end():]
        elif (match := reDescCommentB.search(header)):
            desc = cleanupDescription(match.group(1))
            header = header[match.end():]

        # Skip directives and normal comments
        elif (match := reSkipA.search(header)):
            desc = None
            header = header[match.end():]
        elif (match := reSkipB.search(header)):
            desc = None
            header = header[match.end():]

        else:
            header = header[1:]

    return entities



###########################
#   EMSCRIPTEN BINDINGS   #
###########################

padding = '    '
reOdePrefix = re.compile(r"^(ODE|ode)_")

def removePrefix(s):
    return reOdePrefix.sub('', s)

def jsTypeName(name):
    return removePrefix(name).replace('::', '_')

def makeMethodFunctionName(emName, methodName):
    functionName = 'ode_'
    prevChar = '_'
    for c in emName:
        if prevChar == '_':
            functionName += c.lower()
        else:
            functionName += c
        prevChar = c
    return functionName+'_'+methodName

def isValueObject(entity):
    if entity.category == 'struct' and not namespacedName(entity.namespace, entity.name) in typesUsedAsPtrInAPI and entity.members:
        for member in entity.members:
            if member.category != 'member_variable' or '*' in member.type:
                return False
        return True
    return False

def findMemberType(members, name):
    for member in members:
        if member.name == name and member.type:
            return member.type
    return None

def commonEnumPrefixLength(entity):
    if not entity.members:
        return 0
    # Find common prefix of all enumeration values
    prefixLen = len(entity.members[0].name)
    for value in entity.members[1:]:
        prefixLen = min(prefixLen, len(value.name))
        while not value.name.startswith(entity.members[0].name[:prefixLen]):
            prefixLen -= 1
    prefix = entity.members[0].name[:prefixLen]
    # Match enumeration name in the common prefix
    namePos = 0
    prefixPos = 0
    while namePos < len(entity.name) and prefixPos < len(prefix):
        if entity.name[namePos] == '_':
            namePos += 1
            continue
        if prefix[prefixPos] == '_':
            prefixPos += 1
            continue
        if entity.name[namePos].upper() == prefix[prefixPos].upper():
            namePos += 1
            prefixPos += 1
        else:
            break
    if namePos < len(entity.name) and entity.name[namePos] == '_':
        namePos += 1
    if prefixPos < len(prefix) and prefix[prefixPos] == '_':
        prefixPos += 1
    if namePos == len(entity.name) and prefixPos == len(prefix):
        return len(prefix)
    elif prefix.startswith('ODE_'):
        return len('ODE_')
    return 0

def implementPtrGetter(ptrType, name, className, ptrMember):
    src = ptrType+' '+name+'(const '+className+' &x) {\n'
    src += padding+'return reinterpret_cast<'+ptrType+'>(x.'+ptrMember+');\n'
    src += '}\n\n'
    return src

def implementArrayGetter(arrayType, name, className, arrayMember, lengthMember):
    arrayType = arrayType.strip()
    if arrayType.endswith('*'):
        elementType = arrayType[:-1]
    else:
        return ''
    elementConstRef = elementType.strip()+' const &' if '*' in elementType else 'const '+elementType+'&'
    src = elementConstRef+name+'(const '+className+' &x, int i) {\n'
    src += padding+'ODE_ASSERT(i >= 0 && i < x.'+lengthMember+');\n'
    src += padding+'return x.'+arrayMember+'[i];\n'
    src += '}\n\n'
    return src

def generateEmscriptenBindings(entities, apiPath):
    src = '\n#ifdef __EMSCRIPTEN__\n\n'
    src += '#include <array>\n'
    src += '#include <emscripten/bind.h>\n'
    src += '#include <ode/utils.h>\n'
    src += '#include <ode/'+os.path.basename(apiPath)+'>\n\n'
    src += 'using namespace emscripten;\n\n'
    implementation = ''
    bindings = 'EMSCRIPTEN_BINDINGS(ode) {\n'

    prevCategory = None
    for entity in entities:
        fullName = namespacedName(entity.namespace, entity.name)
        emName = jsTypeName(fullName)

        # Separate different categories by newline
        if entity.category != prevCategory:
            bindings += '\n'

        # Define or constant
        if entity.category == 'define' or entity.category == 'const':
            bindings += padding+'constant("'+emName+'", '+fullName+');\n'
            prevCategory = entity.category

        # Enumeration
        elif entity.category == 'enum':
            commonPrefixLen = commonEnumPrefixLength(entity)
            bindings += padding+'enum_<'+fullName+'>("'+emName+'")'
            for value in entity.members:
                bindings += '\n'+padding+padding+'.value("'+value.name[commonPrefixLen:]+'", '+namespacedName(entity.namespace, value.name)+')'
            bindings += ';\n'

        # Array instance / array typedef
        elif (entity.category == 'array_instance' or (entity.category == 'typedef' and entity.type.count('[') == 1)) and (match := reArrayType.search(entity.type)):
            bindings += padding+'value_array<std::array<'+entity.type[:match.start()]+', '+match.group(1)+'> >("'+emName+'")'
            for i in range(int(match.group(1))):
                bindings += '\n'+padding+padding+'.element(emscripten::index<'+str(i)+'>())'
            bindings += ';\n'

        # Handle
        elif entity.category == 'handle':
            bindings += padding+'class_<'+fullName+'>("'+emName+'").constructor<>();\n'
            prevCategory = entity.category

        # Value object struct
        elif isValueObject(entity):
            bindings += padding+'value_object<'+fullName+'>("'+emName+'")'
            for member in entity.members:
                if member.category == 'member_variable':
                    bindings += '\n'+padding+padding+'.field("'+member.name+'", &'+fullName+'::'+member.name+')'
            bindings += ';\n'

        # Struct
        elif entity.category == 'struct':
            bindings += padding+'class_<'+fullName+'>("'+emName+'").constructor<>'
            for member in entity.members:
                if member.category == 'constructor_bind':
                    bindings += '('+member.name+')'
                    break
            if (bindings.endswith('<>')):
                bindings += '()'
            for member in entity.members:
                if member.category == 'member_variable' and not '*' in member.type:
                    bindings += '\n'+padding+padding+'.property("'+member.name+'", &'+fullName+'::'+member.name+')'
                elif member.category == 'method_bind':
                    bindings += '\n'+padding+padding+'.function("'+member.name+'", '+member.value+')'
                elif member.category == 'ptr_getter_bind':
                    ptrType = 'ODE_ConstDataPtr' if findMemberType(entity.members, member.value).startswith('const ') else 'ODE_VarDataPtr'
                    functionName = makeMethodFunctionName(emName, member.name)
                    bindings += '\n'+padding+padding+'.function("'+member.name+'", &'+functionName+')'
                    implementation += implementPtrGetter(ptrType, functionName, entity.name, member.value)
                elif member.category == 'array_getter_bind':
                    arrayMember, lengthMember = member.value.split(',', 1)
                    functionName = makeMethodFunctionName(emName, member.name)
                    bindings += '\n'+padding+padding+'.function("'+member.name+'", &'+functionName+')'
                    implementation += implementArrayGetter(findMemberType(entity.members, arrayMember), functionName, entity.name, arrayMember, lengthMember)
            bindings += ';\n'

        # Tuple struct
        elif entity.category == 'tuple':
            bindings += padding+'value_array<'+fullName+'>("'+emName+'")'
            for member in entity.members:
                if member.category == 'member_variable':
                    bindings += '\n'+padding+padding+'.element(&'+fullName+'::'+member.name+')'
            bindings += ';\n'

        # Function
        elif entity.category == 'function':
            bindings += padding+'function("'+emName+'", &'+fullName+', allow_raw_pointers());\n'
            prevCategory = entity.category

        else:
            # Undo newline separation if no output
            if entity.category != prevCategory:
                bindings = bindings[:-1]


    bindings += '\n}\n\n'
    src += implementation
    src += bindings
    src += '#endif // __EMSCRIPTEN__\n'
    return src

# TODO: Make sure not to rewrite the file if not necessary to make recompilation reasonably fast
def writeFile(file, contents):
    dirname = os.path.dirname(file)
    if not os.path.exists(dirname):
        os.makedirs(dirname)
    with open(file, "w") as f:
        f.write(contents)

def parseHeaders(headerPaths):
    entityLists = {}
    for headerPath in headerPaths:
        with open(headerPath, "r") as f:
            header = f.read()
        entityLists[headerPath] = parseHeader(header)
    return entityLists

preamble = '\n// FILE GENERATED BY '+os.path.basename(__file__)+'\n'

if __name__ == "__main__":
    headerPaths = []
    embindPath = None
    noMoreSwitches = False
    i = 0
    while (i := i+1) < len(sys.argv):
        if not noMoreSwitches:
            if sys.argv[i] == "--embind":
                if (i := i+1) < len(sys.argv):
                    embindPath = sys.argv[i]
                else:
                    print("Missing embind output path", file=sys.stderr)
                continue
            elif sys.argv[i] == "--":
                noMoreSwitches = True
                continue
            elif sys.argv[i].startswith("-"):
                print("Unknown argument "+sys.argv[i], file=sys.stderr)
                continue
        headerPaths.append(sys.argv[i])

    entityLists = parseHeaders(headerPaths)

    if embindPath is None:
        print(json.dumps(entityLists, default=lambda o: o.__dict__))

    else:
        for headerPath, entities in entityLists.items():
            embindPath = os.path.join(embindPath, os.path.splitext(os.path.basename(headerPath))[0]+"-embind.cpp")
            emscriptenBindings = generateEmscriptenBindings(entities, headerPath)
            writeFile(embindPath, emscriptenBindings)
