
import os
import re

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
reFunctionArg = re.compile(r"((?:(?:ODE_IN|ODE_OUT|ODE_IN_OUT|ODE_OUT_RETURN) )?[\w\s\*]+[\s\*])(\w+)\s*$")
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
    src = '\n#ifdef __EMSCRIPTEN__\n'
    src += '#ifndef NAPI_BINDINGS\n\n'
    src += '#include <array>\n'
    src += '#include <emscripten/bind.h>\n'
    src += '#include '+('"utils.h"' if 'ode-essentials' in apiPath else '<ode/utils.h>')+'\n'
    src += '#include "'+os.path.basename(apiPath)+'"\n\n'
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
    src += '#endif // NAPI_BINDINGS\n'
    src += '#endif // __EMSCRIPTEN__\n'
    return src

###########################
#   N-API BINDINGS        #
###########################

def generateNapiFunctionBinding(entity, emName, fullName):
    is_result = entity.type == 'ODE_Result'
    return_type = 'void' if is_result else 'Napi::Value'
    empty_return = 'return;' if is_result else 'return Napi::Value();'
    signature = f'{return_type} node_napi_{emName}(const Napi::CallbackInfo& info)'
    definition = ""

    definition += (
        f'    Napi::Env env = info.Env();\n'
    )
    call = ""
    i = 0
    outputs = ""
    return_arg = ""
    for mem in entity.members:
        if mem.type.startswith("ODE_OUT_RETURN"):
            if not is_result:
                raise Exception("ODE_OUT_RETURN is only supported on functions returning ODE_Result")
            signature = f'Napi::Value node_napi_{emName}(const Napi::CallbackInfo& info)'
            return_arg = f'    return ode_napi_serialize(env, {mem.name});\n'
            empty_return = 'return Napi::Value();'

    for mem in entity.members:
        i += 1
        t = mem.type
        if call != "":
            call += ", "

        inout_type = "IN"
        if t.startswith("ODE_IN_OUT"): inout_type = "INOUT"
        elif t.startswith("ODE_IN"): inout_type = "IN"
        elif t.startswith("ODE_OUT_RETURN"): inout_type = "RETURN"
        elif t.startswith("ODE_OUT"): inout_type = "OUT"
        elif t.endswith("*") and t.startswith("const "): inout_type = "IN"
        elif t.endswith("*"): inout_type = "INOUT"

        if t.startswith("const") and t.endswith("*"):
            t = t[6:-1].strip()
            call += f"&{mem.name}"
        elif t.endswith("*"):
            t = t[:-1].strip()
            call += f"&{mem.name}"
        else:
            call += mem.name
        
        definition += f'    {t} {mem.name};\n'
        if inout_type == "INOUT" or inout_type == "IN":
            definition += (
                f'    if(!Autobind<{t}>::read_into(info[{i-1}], {mem.name})) {{\n'
                f'        auto ex = env.GetAndClearPendingException();\n'
                f'        Napi::Error::New(env, "Failed to parse argument {mem.name} ("+ ex.Message() +")").ThrowAsJavaScriptException();\n'
                f'        {empty_return}\n'
                f'    }}\n'
            )
        if inout_type == "INOUT" or inout_type == "OUT":
            outputs += f'    Autobind<{t}>::write_from(info[{i-1}], {mem.name});\n'
        if inout_type == "RETURN":
            i -= 1

    if is_result:
        definition += (
            f'    auto result = {fullName}({call});\n'
            f'{outputs}'
            f'    if(!check_result(env,result)) {empty_return}\n'
            f'{return_arg}'
        )
    else:
        definition += (
            f'    auto result = {fullName}({call});\n'
            f'{outputs}'
            f'    return ode_napi_serialize(env, result);\n'
        )
    return [
        signature + ';\n',
        f'{signature} {{\n{definition}}}\n\n'
    ]

def generateNapiBindings(entities, apiPath):
    name = os.path.basename(apiPath)
    header = (
        "#pragma once\n"
        "#include <napi.h>\n"
        f"#include <ode/{name}>\n"
        "\n"
    )
    src_head = (
        '#include <string>\n'
        '#include "addon.h"\n'
        '#include "napi-wrap.h"\n'
        '#include "gen.h"\n'
        '\n'
    )
    name = os.path.splitext(name)[0].replace("-", "_")
    src_tail = ""
    src_body = f"Napi::Object init_gen_{name}(Napi::Env env, Napi::Object exports) {{\n"

    duplicate = set()

    for entity in entities:
        fullName = namespacedName(entity.namespace, entity.name)
        emName = jsTypeName(fullName)

        if fullName in duplicate:
            continue
        duplicate.add(fullName)

        src_body += "\n"

        # Define or constant
        if entity.category == 'define' or entity.category == 'const':
            src_body += f"    exports.Set(\"{emName}\", uint32_t({fullName}));\n"
        
        # Enumeration
        elif entity.category == 'enum':
            commonPrefixLen = commonEnumPrefixLength(entity)
            src_body += "    {\n"
            src_body += f"        auto {emName} = Napi::Object::New(env);\n"
            for value in entity.members:
                k = value.name[commonPrefixLen:]
                v = namespacedName(entity.namespace, value.name)
                src_body += f"        {emName}.Set(\"{k}\", uint32_t({v}));\n"
            src_body += f"        exports.Set(\"{emName}\", {emName});\n"
            src_body += "    }\n"
            header += f'std::string ode_napi_enum_to_string({fullName} value);\n'
            src_tail += (
                f"std::string ode_napi_enum_to_string({fullName} value) {{\n"
                "    switch(value) {\n"
            )
            for value in entity.members:
                k = value.name[commonPrefixLen:]
                full = namespacedName(entity.namespace, value.name)
                src_tail += f'        case {full}: return "{k}";\n'
            serialize_signature = f'Napi::Value ode_napi_serialize(Napi::Env env, const {fullName}& source)'
            header += f'{serialize_signature};\n'
            src_tail += (
                f'        default: return "UNKNOWN_{emName}_"+std::to_string(uint32_t(value));\n'
                "    }\n"
                "}\n"
                f'{serialize_signature} {{\n'
                f'    return Napi::String::New(env, ode_napi_enum_to_string(source));\n'
                f'}}\n'
                "\n"
            )
        # Handle
        elif entity.category == 'handle':
            serialize_signature = f'Napi::Value ode_napi_serialize(Napi::Env env, const {fullName}& source)'
            header += f'{serialize_signature};\n'
            src_tail += (
                f"template<>\n"
                f'const char* Handle<{fullName}>::name = "{emName}";\n'
                f'template<>\n'
                f'bool Autobind<{fullName}>::read_into(const Napi::Value& value, {fullName}& target) {{\n'
                f'    auto optional = Handle<{fullName}>::Read(value);\n'
                '    if(optional) { target = *optional; return true; }\n'
                '    return false;\n'
                '}\n'
                f'{serialize_signature} {{\n'
                f'    return Handle<{fullName}>::serialize(env, source);\n'
                '}\n'
                "\n"
            )
            src_body += f"    Handle<{fullName}>::Export(exports);"
        
        # Function
        elif entity.category == 'function':
            [declaration,definition] = generateNapiFunctionBinding(entity, emName, fullName)
            src_tail += definition
            src_head += declaration
            src_body += f'    exports.Set("{emName}", Napi::Function::New<node_napi_{emName}>(env, "{emName}"));'
        elif entity.category == 'array_instance':
            src_body += f"    // TODO: {entity.category} {emName}\n"
        elif entity.category == 'struct':
            serialize_signature = f'Napi::Value ode_napi_serialize(Napi::Env env, const {fullName}& source)'
            header += f'{serialize_signature};\n'
            read_into = (
                f'template<>\n'
                f'bool Autobind<{fullName}>::read_into(const Napi::Value& value, {fullName}& parsed){{\n'
                f'    Napi::Env env = value.Env();\n'
                f'    Napi::Object obj = value.As<Napi::Object>();\n'
            )
            serialize = (
                f'{serialize_signature} {{\n'
                f'    Napi::Object obj = Napi::Object::New(env);\n'
            )
            for member in entity.members:
                if member.category == 'member_variable' and (member.type.endswith("*") or member.type.endswith("Ptr")):
                    read_into += (
                        f'    uintptr_t ptr_{member.name};\n'
                        f'    if(Autobind<uintptr_t>::read_into(obj.Get("{member.name}"), ptr_{member.name})) {{\n'
                        f'        parsed.{member.name} = reinterpret_cast<{member.type}>(ptr_{member.name});\n'
                        f'    }} else {{\n'
                        f'        env.GetAndClearPendingException();\n'
                        f'        Napi::Error::New(env, "Invalid value for field {member.name}").ThrowAsJavaScriptException();\n'
                        f'        return false;\n'
                        f'    }}\n'
                    )
                    serialize += f'    Napi::Value {member.name} = ode_napi_serialize(env, (uintptr_t)source.{member.name});\n'
                elif member.category == 'member_variable':
                    read_into += (
                        f'    if(!Autobind<{member.type}>::read_into(obj.Get("{member.name}"), parsed.{member.name})) {{\n'
                        f'        env.GetAndClearPendingException();\n'
                        f'        Napi::Error::New(env, "Invalid value for field {member.name}").ThrowAsJavaScriptException();\n'
                        f'        return false;\n'
                        f'    }}\n'
                    )
                    serialize += f'    Napi::Value {member.name} = ode_napi_serialize(env, source.{member.name});\n'
                elif member.category == 'array_getter_bind':
                    print(member)
                    [entries,n] = member.value.split(',')
                    fn_name = f'{emName}_{member.name}'
                    signature = f'Napi::Value node_napi_{fn_name}(const Napi::CallbackInfo& info)'
                    src_head += f'{signature};\n'
                    src_tail += (
                        f'{signature} {{\n'
                        f'    //{fullName} self;\n'
                        f'    //if(!Autobind<{fullName}>::read_into(info[0], self)) {{ return Napi::Value(); }};\n'
                        f'    //int i = info[1].As<Napi::Number>().Uint32Value();\n'
                        f'    //ODE_ASSERT(i >= 0 && i < self.{n});\n'
                        f'    //return ode_napi_serialize(info.Env(), self.{entries}[idx]);\n'
                        f'    return Napi::String::New(info.Env(), "TODO");\n'
                        f'}}\n'
                    )
                    src_body += f'    exports.Set("{fn_name}", Napi::Function::New<node_napi_{fn_name}>(env, "{fn_name}"));\n'
                else:
                    read_into += f"    // TODO: {member.category}\n"
                
                if member.category == 'member_variable':
                    serialize += (
                        f'    if({member.name}.IsEmpty()) return Napi::Value();\n'
                        f'    obj.Set("{member.name}", {member.name});\n'
                    )
            read_into += (
                f'    return true;\n'
                f'}}\n'
            ) 
            serialize += (
                f'    return obj;\n'
                f'}}\n\n'
            )
            src_tail += read_into + serialize
        else:
            src_body += f"    // TODO: {entity.category} {emName}\n"

    src_body += "    return exports;\n}\n\n"
    return (src_head + "\n" + src_body + src_tail, header)



###########################
#   TYPESCRIPT BINDINGS   #
###########################

tsTypes = []

def tsType(type):
    if type.startswith("ODE_IN "): type = type[7:]
    if type.startswith("ODE_OUT "): type = type[8:]
    if type.startswith("ODE_IN_OUT "): type = type[11:]
    if type.startswith("ODE_OUT_RETURN "): type = type[15:]
    if type:
        type = (' '+type+' ').replace('*', ' ').replace(' const ', '').strip()
        if (jsType := jsTypeName(type)):
            tsType = jsType[0].upper()+jsType[1:]
            return 'ode.'+tsType
    return 'unknown'

def tsDescription(description, padding = ''):
    if description:
        if '\n' in description:
            return padding+'/**\n'+padding+' * '+('\n'+padding+' * ').join(description.split('\n'))+'\n'+padding+' */\n'
        return padding+'/** '+description.replace('\n', ' ')+' */\n'
    return ''

def tsTranslateArgs(api):
    argList = api.split('(', 1)[1].replace(')', '').split(',')
    for i in range(len(argList)):
        if not argList[i].strip():
            continue
        simplifiedArg = (' '+argList[i]+' ').replace('*', ' ').replace('&', ' ').replace(' const ', '').strip()
        argName = 'arg'+str(i)
        if ' ' in simplifiedArg:
            argName = simplifiedArg[simplifiedArg.rindex(' ')+1:]
            simplifiedArg = simplifiedArg[:simplifiedArg.rindex(' ')].strip()
        argList[i] = argName+': '+tsType(simplifiedArg)
    return ', '.join(argList)

def generateTypescriptBindings(entities):

    ts = '\n'
    ts += 'import { EngineSymbol, Enum, EnumValue } from "./internal.js";\n'
    ts += 'import * as ode from "./exports.js";\n'
    ts += '\n'
    prevCategory = None
    for entity in entities:
        name = jsTypeName(namespacedName(entity.namespace, entity.name))

        # Define or constant
        if entity.category == 'define' or entity.category == 'const':
            if prevCategory == entity.category:
                ts = ts[:-1]
            ts += tsDescription(entity.description)
            ts += 'export const '+name+': '+tsType(entity.type)+';\n\n'

        # Enumeration
        elif entity.category == 'enum':
            tsTypes.append(name)
            commonPrefixLen = commonEnumPrefixLength(entity)
            ts += tsDescription(entity.description)
            ts += 'export const '+name+': Enum<'+name+'_Map>;\n'
            ts += 'export type '+name+' = EnumValue<'+name+'_Index>;\n'
            ts += 'export type '+name+'_Index = '+name+'_Map[keyof '+name+'_Map];\n'
            ts += 'export type '+name+'_Map = {\n'
            for value in entity.members:
                ts += tsDescription(value.description, padding)
                ts += padding+value.name[commonPrefixLen:]+': '+value.value+';\n'
            ts += '};\n\n'

        # Array instance / array typedef
        elif (entity.category == 'array_instance' or (entity.category == 'typedef' and entity.type.count('[') == 1)) and (match := reArrayType.search(entity.type)):
            tsTypes.append(name)
            elemType = tsType(entity.type[:match.start()])
            ts += tsDescription(entity.description if entity.description else 'An array of '+match.group(1)+' '+entity.type[:match.start()]+'s')
            ts += 'export type '+name+' = readonly [\n'
            for i in range(int(match.group(1))):
                ts += padding+elemType+',\n'
            ts += '];\n\n'

        # Handle
        elif entity.category == 'handle':
            tsTypes.append(name)
            ts += tsDescription(entity.description)
            ts += 'export const '+name+': { new (): ode.'+name+' };\n'
            ts += 'export type '+name+' = {\n'
            ts += padding+'[EngineSymbol]: "'+name+'";\n'
            ts += padding+'constructor();\n'
            ts += padding+'delete(): void;\n'
            ts += '};\n\n'

        # Value object struct
        elif isValueObject(entity):
            tsTypes.append(name)
            ts += tsDescription(entity.description)
            ts += 'export type '+name+' = {\n'
            for member in entity.members:
                if member.category == 'member_variable' and not '*' in member.type:
                    ts += tsDescription(member.description, padding)
                    ts += padding+member.name+': '+tsType(member.type)+';\n'
            ts += '};\n\n'

        # Struct
        elif entity.category == 'struct':
            tsTypes.append(name)
            ts += tsDescription(entity.description)
            ts += 'export const '+name+': { new ('
            for member in entity.members:
                if member.category == 'constructor_bind':
                    ts += tsTranslateArgs(member.type)
                    break
            ts += '): ode.'+name+' };\n'
            ts += 'export type '+name+' = {\n'
            ts += padding+'[EngineSymbol]: "'+name+'";\n'
            for member in entity.members:
                if member.category == 'constructor_bind':
                    ts += padding+'constructor();\n'
                elif member.category == 'member_variable':
                    if not '*' in member.type:
                        ts += tsDescription(member.description, padding)
                        ts += padding+member.name+': '+tsType(member.type)+';\n'
                elif member.category == 'method_bind':
                    returnType = member.type.split('(')[0]
                    ts += tsDescription(member.description, padding)
                    ts += padding+member.name+'('+tsTranslateArgs(member.type)+'): '+tsType(returnType)+';\n'
                elif member.category == 'ptr_getter_bind':
                    ptrType = 'ODE_ConstDataPtr' if findMemberType(entity.members, member.value).startswith('const ') else 'ODE_VarDataPtr'
                    ts += tsDescription(member.description, padding)
                    ts += padding+member.name+'(): '+tsType(ptrType)+';\n'
                elif member.category == 'array_getter_bind':
                    arrayMember, lengthMember = member.value.split(',', 1)
                    arrayElementType = findMemberType(entity.members, arrayMember).strip()[:-1].strip()
                    ts += tsDescription(member.description, padding)
                    ts += padding+member.name+'(i: ode.Int): '+tsType(arrayElementType)+';\n'
            ts += padding+'delete(): void;\n'
            ts += '};\n\n'

        # Tuple struct
        elif entity.category == 'tuple':
            tsTypes.append(name)
            ts += tsDescription(entity.description)
            ts += 'export type '+name+' = readonly ['
            tsBody = []
            for member in entity.members:
                tsBody.append('\n'+padding+member.name+': '+tsType(member.type))
            ts += ','.join(tsBody)
            ts += '\n];\n\n'

        # Function
        elif entity.category == 'function':
            returnType = tsType(entity.type)
            if entity.type == "ODE_Result":
                returnType = "void"
            fullDescription = entity.description
            for arg in entity.members:
                if arg.description and not arg.type.startswith("ODE_OUT_RETURN"):
                    fullDescription += '\n@param '+arg.name+' '+arg.description
            for arg in entity.members:
                if arg.description and arg.type.startswith("ODE_OUT_RETURN"):
                    fullDescription += '\n@returns '+arg.name+' '+arg.description
            ts += tsDescription(fullDescription)
            ts += 'export function '+name+'(\n'
            for arg in entity.members:
                if arg.type.startswith("ODE_OUT_RETURN"):
                    returnType = tsType(arg.type)
                else:
                    ts += padding+arg.name+': '+tsType(arg.type)+',\n'
            ts += '): '+returnType+';\n\n'

        prevCategory = entity.category

    return ts[:-1]

tsLoader = '''
import * as ODE from "./exports.js";
export type ODE = typeof ODE;

export type LoadODEOptions = {
    locateFile?: () => string;
};

export default function loadODE(options?: LoadODEOptions): Promise<ODE>;
export const version: string;
'''

def generateTopLevelTypescriptBindings():
    ts = tsLoader+'\n'
    ts += 'export {\n'
    for typeName in tsTypes:
        ts += padding+'type '+typeName+',\n'
    ts += '} from "./exports.js";\n'
    return ts



preamble = '\n// FILE GENERATED BY '+os.path.basename(__file__)+'\n'

def relPath(path):
    return os.path.join(os.path.dirname(__file__), path.replace("/", os.sep))

def generateBindings(headerPath):
    emscriptenBindingsPath = os.path.join(os.path.dirname(headerPath), "emscripten-bindings.cpp")
    napiBindingsPath = os.path.join(relPath("ode-napi"), "gen-"+os.path.splitext(os.path.basename(headerPath))[0])
    typescriptBindingsPath = os.path.join(relPath("typescript-bindings"), os.path.splitext(os.path.basename(headerPath))[0]+".d.ts")
    with open(headerPath, "r") as f:
        header = f.read()
    entities = parseHeader(header)
    emscriptenBindings = generateEmscriptenBindings(entities, headerPath)
    with open(emscriptenBindingsPath, "w") as f:
        f.write(preamble+emscriptenBindings)
    (napiBindings, napiHeader) = generateNapiBindings(entities, headerPath)
    with open(napiBindingsPath+".cpp", "w") as f:
        f.write(preamble+napiBindings)
    with open(napiBindingsPath+".h", "w") as f:
        f.write(preamble+napiHeader)
    typescriptBindings = generateTypescriptBindings(entities)
    with open(typescriptBindingsPath, "w") as f:
        f.write(preamble+typescriptBindings)

def generateTopLevelBindings(paths):
    typescriptBindingsPath = os.path.join(relPath("typescript-bindings/ode.d.ts"))
    typescriptBindings = generateTopLevelTypescriptBindings()
    with open(typescriptBindingsPath, "w") as f:
        f.write(preamble+typescriptBindings)
    
    includes = ""
    for path in paths:
        basename = os.path.basename(path)
        includes += f'#include "gen-{basename}"\n'
    with open(os.path.join(relPath("ode-napi"), "gen.h"), "w") as f:
        f.write("#pragma once\n"+preamble+includes)

paths = ["ode-essentials/ode/api-base.h","ode-logic/ode/logic-api.h","ode-renderer/ode/renderer-api.h"]
for path in paths:
    generateBindings(relPath(path))
generateTopLevelBindings(paths)
