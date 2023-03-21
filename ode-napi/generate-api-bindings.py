
import sys
import os
import re

def relPath(path):
    return os.path.join(os.path.dirname(__file__), path.replace("/", os.sep))

# This is apparently how you do an include in Python ......
apiUtilsPath = relPath("../api-utils.py")
globalName = __name__
globalFile = __file__
__name__ = "generateApiBindings"
__file__ = apiUtilsPath
exec(compile(open(apiUtilsPath, "rb").read(), apiUtilsPath, 'exec'), globals())
__name__ = globalName
__file__ = globalFile

###########################
#     N-API BINDINGS      #
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
        if t.startswith("ODE_INOUT"): inout_type = "INOUT"
        elif t.startswith("ODE_IN"): inout_type = "IN"
        elif t.startswith("ODE_OUT_RETURN"): inout_type = "RETURN"
        elif t.startswith("ODE_OUT"): inout_type = "OUT"

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
                f'    if (!ode_napi_read_into(info[{i-1}], {mem.name})) {{\n'
                f'        auto ex = env.GetAndClearPendingException();\n'
                f'        Napi::Error::New(env, "Failed to parse argument{i} {mem.name} in {emName} ("+ ex.Message() +")").ThrowAsJavaScriptException();\n'
                f'        {empty_return}\n'
                f'    }}\n'
            )
        if inout_type == "INOUT" or inout_type == "OUT":
            outputs += f'    ode_napi_write_from(info[{i-1}], {mem.name});\n'
        if inout_type == "RETURN":
            i -= 1

    if is_result:
        definition += (
            f'    auto result = {fullName}({call});\n'
            f'{outputs}'
            f'    if (!check_result(env,result)) {empty_return}\n'
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

        # TODO actually solve this normally
        if entity.name == "ODE_MemoryRef" or entity.name == "ODE_Transformation":
            continue

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
            header += (
                f'std::string ode_napi_enum_to_string({fullName} value);\n'
                f'bool ode_napi_read_into(const Napi::Value& value, {fullName}& parsed);\n'
            )
            src_tail += (
                f'bool ode_napi_read_into(const Napi::Value& value, {fullName}& parsed) {{\n'
                f'    std::string text = value.As<Napi::String>();\n'
            )
            for value in entity.members:
                k = value.name[commonPrefixLen:]
                full = namespacedName(entity.namespace, value.name)
                src_tail += f'    if (text == "{k}") {{ parsed = {full}; return true; }}'
            src_tail += (
                f'    return false;\n'
                f'}}\n'
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
            read_into_signature = f'bool ode_napi_read_into(const Napi::Value& value, {fullName}& target)'
            header += f'{serialize_signature};\n{read_into_signature};\n'
            src_tail += (
                f'{read_into_signature} {{\n'
                f'    return ode_napi_handle_read_into(value, "{emName}", target);\n'
                '}\n'
                f'{serialize_signature} {{\n'
                f'    return ode_napi_handle_serialize(env, "{emName}", source);\n'
                '}\n'
                "\n"
            )

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
            read_into_signature = f'bool ode_napi_read_into(const Napi::Value& value, {fullName}& parsed)'
            header += f'{serialize_signature};\n{read_into_signature};\n'
            read_into = (
                f'{read_into_signature} {{\n'
                f'    Napi::Env env = value.Env();\n'
                f'    Napi::Object obj = value.As<Napi::Object>();\n'
            )
            serialize = (
                f'{serialize_signature} {{\n'
                f'    Napi::Object obj = Napi::Object::New(env);\n'
            )
            for member in entity.members:
                if member.category == 'member_variable':
                    read_into += (
                        f'    if (!ode_napi_read_into(obj.Get("{member.name}"), parsed.{member.name})) {{\n'
                        f'        auto ex = env.GetAndClearPendingException();\n'
                        f'        Napi::Error::New(env, "Invalid value for field {member.name} of {fullName} ("+ex.Message()+")").ThrowAsJavaScriptException();\n'
                        f'        return false;\n'
                        f'    }}\n'
                    )
                    serialize += f'    Napi::Value {member.name} = ode_napi_serialize(env, source.{member.name});\n'
                elif member.category == 'array_getter_bind':
                    [entries,n] = member.value.split(',')
                    fn_name = f'{emName}_{member.name}'
                    signature = f'Napi::Value node_napi_{fn_name}(const Napi::CallbackInfo& info)'
                    src_head += f'{signature};\n'
                    src_tail += (
                        f'{signature} {{\n'
                        f'    {fullName} self;\n'
                        f'    Napi::Env env = info.Env();\n'
                        f'    if (!ode_napi_read_into(info[0], self)) {{ return Napi::Value(); }};\n'
                        f'    int i = info[1].As<Napi::Number>().Uint32Value();\n'
                        f'    if (i < 0 || i >= self.{n}) {{ Napi::Error::New(env, "Index out of range").ThrowAsJavaScriptException(); return Napi::Value(); }}\n'
                        f'    return ode_napi_serialize(info.Env(), self.{entries}[i]);\n'
                        f'}}\n'
                    )
                    src_body += f'    exports.Set("{fn_name}", Napi::Function::New<node_napi_{fn_name}>(env, "{fn_name}"));\n'
                else:
                    read_into += f"    // TODO: {member.category}\n"

                if member.category == 'member_variable':
                    serialize += (
                        f'    if ({member.name}.IsEmpty()) return Napi::Value();\n'
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

        # Tuple
        elif entity.category == 'tuple':
            serialize_signature = f'Napi::Value ode_napi_serialize(Napi::Env env, const {fullName}& source)'
            read_into_signature = f'bool ode_napi_read_into(const Napi::Value& value, {fullName}& parsed)'
            header += f'{serialize_signature};\n{read_into_signature};\n'
            read_into = (
                f'{read_into_signature} {{\n'
                f'    Napi::Env env = value.Env();\n'
                f'    Napi::Array obj = value.As<Napi::Array>();\n'
            )
            serialize = (
                f'{serialize_signature} {{\n'
                f'    Napi::Array obj = Napi::Array::New(env);\n'
            )
            i = 0
            for member in entity.members:
                read_into += (
                    f'    if (!ode_napi_read_into(obj.Get(uint32_t({i})), parsed.{member.name})) {{\n'
                    f'        auto ex = env.GetAndClearPendingException();\n'
                    f'        Napi::Error::New(env, "Invalid value for field {member.name} of {fullName} ("+ex.Message()+") got: "+(std::string)obj.Get(uint32_t({i})).Unwrap().ToString().Unwrap()).ThrowAsJavaScriptException();\n'
                    f'        return false;\n'
                    f'    }}\n'
                )
                serialize += f'    Napi::Value {member.name} = ode_napi_serialize(env, source.{member.name});\n'


                serialize += (
                    f'    if ({member.name}.IsEmpty()) return Napi::Value();\n'
                    f'    obj.Set(uint32_t({i}), {member.name});\n'
                )
                i += 1
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
    type = type.strip()
    wrap = ""
    if (match := reArgAttrib.search(type)):
        if match.group(1) == 'ODE_OUT_RETURN':
            # The argument object does not have to contain any values, but it has
            # to be there for us to be able to write into it.
            wrap = 'Partial'
        type = type[match.end():].strip()
    if type:
        type = (' '+type+' ').replace('*', ' ').replace(' const ', '').strip()
        if (jsType := jsTypeName(type)):
            tsType = jsType[0].upper()+jsType[1:]
            if wrap:
                return wrap+'<ode.'+tsType+'>'
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
            ts += 'export type '+name+' = keyof '+name+'_Map;\n'
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
            ts += 'export type '+name+' = {\n'
            ts += padding+name+': number;\n'
            ts += '};\n\n'

        # Value object struct
        elif isValueObject(entity):
            tsTypes.append(name)
            ts += tsDescription(entity.description)
            ts += 'export type '+name+' = {\n'
            for member in entity.members:
                if member.category == 'member_variable':
                    ts += tsDescription(member.description, padding)
                    ts += padding+member.name+': '+tsType(member.type)+';\n'
            ts += '};\n\n'

        # Struct
        elif entity.category == 'struct':
            tsTypes.append(name)
            ts += tsDescription(entity.description)
            ts += 'export type '+name+' = {\n'
            for member in entity.members:
                if member.category == 'member_variable':
                    if not '*' in member.type:
                        ts += tsDescription(member.description, padding)
                        ts += padding+member.name+': '+tsType(member.type)+';\n'
            ts += '};\n\n'
            for member in entity.members:
                if member.category == 'array_getter_bind':
                    arrayMember, lengthMember = member.value.split(',', 1)
                    arrayElementType = findMemberType(entity.members, arrayMember).strip()[:-1].strip()
                    ts += tsDescription(member.description, '')
                    ts += 'export function '+name+'_'+member.name+'(self: '+name+', i: ode.Int): '+tsType(arrayElementType)+';\n'
            ts += '\n'

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
                if arg.description and not reReturnArg.search(arg.type):
                    fullDescription += '\n@param '+arg.name+' '+arg.description
            for arg in entity.members:
                if arg.description and reReturnArg.search(arg.type):
                    fullDescription += '\n@returns '+arg.name+' '+arg.description
            ts += tsDescription(fullDescription)
            ts += 'export function '+name+'(\n'
            for arg in entity.members:
                if reReturnArg.search(arg.type):
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
    instantiateWasm?: (importObject: any, receiveInstance: (instance: any) => void) => void;
};

export default function loadODE(options?: LoadODEOptions): Promise<ODE>;
export const version: string;
export const wasm: string;
'''

def generateTopLevelTypescriptBindings():
    ts = tsLoader+'\n'
    ts += 'export {\n'
    for typeName in tsTypes:
        ts += padding+'type '+typeName+',\n'
    ts += '} from "./exports.js";\n'
    return ts

# Make sure not to rewrite the file if not necessary to make recompilation
# reasonably fast.
def writeFile(file, contents):
    dirname = os.path.dirname(file)
    if not os.path.exists(dirname):
        os.makedirs(dirname)
    with open(file, "w") as f:
        f.write(contents)

def generateBindings(headerPath, outPath):
    napiBindingsPath = os.path.join(outPath, "codegen", "gen-"+os.path.splitext(os.path.basename(headerPath))[0])
    typescriptBindingsPath = os.path.join(outPath, "codegen", os.path.splitext(os.path.basename(headerPath))[0]+".d.ts")
    with open(headerPath, "r") as f:
        header = f.read()
    entities = parseHeader(header)
    (napiBindings, napiHeader) = generateNapiBindings(entities, headerPath)
    writeFile(napiBindingsPath+".cpp", preamble+napiBindings)
    writeFile(napiBindingsPath+".h", preamble+napiHeader)
    typescriptBindings = generateTypescriptBindings(entities)
    writeFile(typescriptBindingsPath, preamble+typescriptBindings)

def generateTopLevelBindings(paths, outPath):
    typescriptBindingsPath = os.path.join(outPath, "codegen", "index.d.ts")
    typescriptBindings = generateTopLevelTypescriptBindings()
    writeFile(typescriptBindingsPath, preamble+typescriptBindings)

    includes = ""
    for path in paths:
        basename = os.path.basename(path)
        includes += f'#include "gen-{basename}"\n'
    writeFile(os.path.join(outPath, "codegen", "gen.h"), "#pragma once\n"+preamble+includes)

outPath = sys.argv[1]
paths = sys.argv[2:]
for path in paths:
    generateBindings(path, outPath)
generateTopLevelBindings(paths, outPath)
