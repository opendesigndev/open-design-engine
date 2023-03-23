
import sys
import os
import re

def relPath(path):
    return os.path.join(os.path.dirname(__file__), path.replace('/', os.sep))

# This is apparently how you do an include in Python ......
apiUtilsPath = relPath('../api-utils.py')
globalName = __name__
globalFile = __file__
__name__ = 'generateApiBindings'
__file__ = apiUtilsPath
exec(compile(open(apiUtilsPath, 'rb').read(), apiUtilsPath, 'exec'), globals())
__name__ = globalName
__file__ = globalFile



###########################
#     N-API BINDINGS      #
###########################

reConstModifier = re.compile(r'\bconst\b')

def generateNapiFunctionBinding(entity, emName, fullName):
    hasResult = entity.type == 'ODE_Result'
    returnType = 'void' if hasResult else 'Napi::Value'
    emptyReturn = 'return' if hasResult else 'return Napi::Value()'
    signature = f'{returnType} call_{emName}(const Napi::CallbackInfo &info)'

    body = '    Napi::Env env = info.Env();\n'
    callArgs = ''
    i = 0
    outputs = ''
    returnArg = ''
    for member in entity.members:
        if reReturnArg.search(member.type):
            if not hasResult:
                raise Exception('ODE_OUT_RETURN is only supported on functions returning ODE_Result')
            signature = f'Napi::Value call_{emName}(const Napi::CallbackInfo &info)'
            returnArg = f'    return wrap(env, {member.name});\n'
            emptyReturn = 'return Napi::Value()'

    for member in entity.members:
        argType = member.type
        if callArgs != '':
            callArgs += ', '

        argAttrib = 'IN'
        if (match := reArgAttrib.search(argType)):
            argAttrib = match.group(2)

        # Remove pointer & const
        if argType.endswith('*'):
            argType = argType[:-1].strip()
            argTypeCParts = reConstModifier.split(argType)
            argType = 'const'.join(argTypeCParts[:-1]).strip()+argTypeCParts[-1].strip()
            callArgs += f'&{member.name}'
        else:
            callArgs += member.name

        body += f'    {argType} {member.name};\n'
        if argAttrib == 'IN' or argAttrib == 'INOUT':
            body += (
                f'    if (!unwrap({member.name}, info[{i}])) ' '{\n'
                f'        Napi::Error error = env.GetAndClearPendingException();\n'
                f'        Napi::Error::New(env, "Failed to parse argument {i} {member.name} in {emName} ("+ error.Message() +")").ThrowAsJavaScriptException();\n'
                f'        {emptyReturn};\n'
                 '    }\n'
            )
        if argAttrib == 'INOUT' or argAttrib == 'OUT':
            outputs += f'    copyWrapped(info[{i}], {member.name});\n'
        if argAttrib != 'OUT_RETURN':
            i += 1

    if hasResult:
        body += (
            f'    ODE_Result result = {fullName}({callArgs});\n'
            f'{outputs}'
            f'    if (!checkResult(env, result))\n'
            f'        {emptyReturn};\n'
            f'{returnArg}'
        )
    else:
        body += (
            f'    {entity.type} result = {fullName}({callArgs});\n'
            f'{outputs}'
            f'    return wrap(env, result);\n'
        )
    return [
        f'{signature};\n',
        f'{signature} ' '{\n'
        f'{body}'
        '}\n\n'
    ]

def generateNapiBindings(entities, apiPath, donePaths):
    fileName = os.path.basename(apiPath)
    header = (
        f'\n#pragma once\n\n'
        f'#include <napi-base.h>\n'
        f'#include <ode/{fileName}>\n\n'
         'namespace ode {\n'
         'namespace napi {\n\n'
    )
    src = (
        f'\n#include "napi-{fileName}"\n\n'
        f'#include <string>\n'
    )
    for donePath in donePaths:
        src += f'#include "napi-{os.path.basename(donePath)}"\n'
    src += (
        '#include <napi-utils.h>\n'
        # napi-wrap.h must be included last!
        '#include <napi-wrap.h>\n\n'
        'namespace ode {\n'
        'namespace napi {\n\n'
    )
    namespaceEnd = '}\n}\n'
    name = os.path.splitext(fileName)[0].replace('-', '_')
    header += f'Napi::Object {name}_initialize(Napi::Env env, Napi::Object exports);\n'
    header += f'void {name}_export(const Napi::Env &env, Napi::Object &exports);\n\n'
    src += (
        f'Napi::Object {name}_initialize(Napi::Env env, Napi::Object exports)' ' {\n'
        f'    {name}_export(env, exports);\n'
         '    return exports;\n'
         '}\n\n'
    )
    srcExports = ''

    for entity in entities:

        # TODO actually solve this normally
        if entity.name == 'ODE_MemoryRef' or entity.name == 'ODE_Transformation':
            continue

        fullName = namespacedName(entity.namespace, entity.name)
        emName = jsTypeName(fullName)

        # Define or constant
        if entity.category == 'define' or entity.category == 'const':
            srcExports += f'    exports.Set("{emName}", uint32_t({fullName}));\n'

        # Enumeration
        elif entity.category == 'enum':
            commonPrefixLen = commonEnumPrefixLength(entity)
            srcExports += '    {\n'
            srcExports += f'        Napi::Object obj{emName} = Napi::Object::New(env);\n'
            for value in entity.members:
                k = value.name[commonPrefixLen:]
                v = namespacedName(entity.namespace, value.name)
                srcExports += f'        obj{emName}.Set("{k}", int32_t({v}));\n'
            srcExports += f'        exports.Set("{emName}", (Napi::Object &&) obj{emName});\n'
            srcExports += '    }\n'
            wrapSignature = f'Napi::Value wrap(const Napi::Env &env, const {fullName} &src)'
            header += (
                f'std::string enumString({fullName} value);\n'
                f'{wrapSignature};\n'
                f'bool unwrap({fullName} &dst, const Napi::Value &src);\n\n'
            )
            src += (
                f'std::string enumString({fullName} value)' ' {\n'
                 '    switch(value) {\n'
            )
            for value in entity.members:
                k = value.name[commonPrefixLen:]
                full = namespacedName(entity.namespace, value.name)
                src += f'        case {full}: return "{k}";\n'
            src += (
                f'        default: return "!UNKNOWN_{emName}_"+std::to_string(int32_t(value));\n'
                 '    }\n'
                 '}\n\n'
                f'{wrapSignature}' ' {\n'
                f'    return Napi::String::New(env, enumString(src));\n'
                 '}\n\n'
                f'bool unwrap({fullName} &dst, const Napi::Value &src)' '{\n'
                f'    std::string text = src.As<Napi::String>();\n'
            )
            for value in entity.members:
                k = value.name[commonPrefixLen:]
                full = namespacedName(entity.namespace, value.name)
                src += f'    if (text == "{k}") return dst = {full}, true;\n'
            src += (
                f'    return false;\n'
                 '}\n\n'
            )

        # Handle
        elif entity.category == 'handle':
            wrapSignature = f'Napi::Value wrap(const Napi::Env &env, const {fullName} &src)'
            unwrapSignature = f'bool unwrap({fullName} &dst, const Napi::Value &src)'
            header += f'{wrapSignature};\n{unwrapSignature};\n\n'
            src += (
                f'{wrapSignature} {{\n'
                f'    return wrapHandle(env, "{emName}", src);\n'
                 '}\n\n'
                f'{unwrapSignature} {{\n'
                f'    return unwrapHandle(dst, "{emName}", src);\n'
                 '}\n\n'
            )

        # Function
        elif entity.category == 'function':
            [declaration, body] = generateNapiFunctionBinding(entity, emName, fullName)
            src += body
            srcExports += f'    exports.Set("{emName}", Napi::Function::New<call_{emName}>(env, "{emName}"));\n'

        # Array instance
        elif entity.category == 'array_instance':
            srcExports += f'    // TODO: {entity.category} {emName}\n'

        # Struct
        elif entity.category == 'struct':
            wrapSignature = f'Napi::Value wrap(const Napi::Env &env, const {fullName} &src)'
            unwrapSignature = f'bool unwrap({fullName} &dst, const Napi::Value &value)'
            header += f'{wrapSignature};\n{unwrapSignature};\n\n'
            wrap = (
                f'{wrapSignature} {{\n'
                f'    Napi::Object obj = Napi::Object::New(env);\n'
            )
            unwrap = (
                f'{unwrapSignature} {{\n'
                f'    Napi::Env env = value.Env();\n'
                f'    Napi::Object obj = value.As<Napi::Object>();\n'
            )
            for member in entity.members:
                if member.category == 'member_variable':
                    wrap += f'    Napi::Value {member.name} = wrap(env, src.{member.name});\n'
                    unwrap += (
                        f'    if (!unwrap(dst.{member.name}, obj.Get("{member.name}")))' ' {\n'
                        f'        Napi::Error error = env.GetAndClearPendingException();\n'
                        f'        Napi::Error::New(env, "Invalid value for field {member.name} of {fullName} ("+error.Message()+")").ThrowAsJavaScriptException();\n'
                        f'        return false;\n'
                         '    }\n'
                    )
                elif member.category == 'array_getter_bind':
                    [entries, n] = member.value.split(',')
                    fnName = f'{emName}_{member.name}'
                    signature = f'Napi::Value call_{fnName}(const Napi::CallbackInfo &info)'
                    src += (
                        f'{signature}' ' {\n'
                        f'    {fullName} self;\n'
                        f'    Napi::Env env = info.Env();\n'
                        f'    if (!unwrap(self, info[0]))\n'
                        f'        return Napi::Value();\n'
                        f'    int i = info[1].As<Napi::Number>().Int32Value();\n'
                        f'    if (i < 0 || i >= self.{n})' ' {\n'
                        f'        Napi::Error::New(env, "Index out of range").ThrowAsJavaScriptException();\n'
                        f'        return Napi::Value();\n'
                         '    }\n'
                        f'    return wrap(info.Env(), self.{entries}[i]);\n'
                         '}\n\n'
                    )
                    srcExports += f'    exports.Set("{fnName}", Napi::Function::New<call_{fnName}>(env, "{fnName}"));\n'
                else:
                    unwrap += f'    // TODO: {member.category}\n'

                if member.category == 'member_variable':
                    wrap += (
                        f'    if ({member.name}.IsEmpty())\n'
                        f'        return Napi::Value();\n'
                        f'    obj.Set("{member.name}", {member.name});\n'
                    )
            wrap += (
                f'    return obj;\n'
                 '}\n\n'
            )
            unwrap += (
                f'    return true;\n'
                 '}\n\n'
            )
            src += wrap+unwrap

        # Tuple struct
        elif entity.category == 'tuple':
            wrapSignature = f'Napi::Value wrap(const Napi::Env &env, const {fullName} &src)'
            unwrapSignature = f'bool unwrap({fullName} &dst, const Napi::Value &value)'
            header += f'{wrapSignature};\n{unwrapSignature};\n\n'
            wrap = (
                f'{wrapSignature}' ' {\n'
                f'    Napi::Array obj = Napi::Array::New(env);\n'
            )
            unwrap = (
                f'{unwrapSignature}' ' {\n'
                f'    Napi::Env env = value.Env();\n'
                f'    Napi::Array obj = value.As<Napi::Array>();\n'
            )
            i = 0
            for member in entity.members:
                wrap += (
                    f'    Napi::Value {member.name} = wrap(env, src.{member.name});\n'
                    f'    if ({member.name}.IsEmpty()) return Napi::Value();\n'
                    f'    obj.Set(uint32_t({i}), {member.name});\n'
                )
                unwrap += (
                    f'    if (!unwrap(dst.{member.name}, obj.Get(uint32_t({i}))))' ' {\n'
                    f'        Napi::Error error = env.GetAndClearPendingException();\n'
                    f'        Napi::Error::New(env, "Invalid value for field {member.name} of {fullName} ("+error.Message()+") got: "+(std::string)obj.Get(uint32_t({i})).Unwrap().ToString().Unwrap()).ThrowAsJavaScriptException();\n'
                    f'        return false;\n'
                     '    }\n'
                )
                i += 1
            wrap += (
                f'    return obj;\n'
                 '}\n\n'
            )
            unwrap += (
                f'    return true;\n'
                 '}\n\n'
            )
            src += wrap+unwrap

        else:
            srcExports += f'    // TODO: {entity.category} {emName}\n'

    header += namespaceEnd
    src += (
        f'void {name}_export(const Napi::Env &env, Napi::Object &exports)' ' {\n'
        f'{srcExports}'
         '}\n\n'
        f'{namespaceEnd}'
    )
    return (src, header)



###########################
#   TYPESCRIPT BINDINGS   #
###########################

tsTypes = []

def tsType(type):
    type = type.strip()
    wrap = ''
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
            if entity.type == 'ODE_Result':
                returnType = 'void'
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

def generateBindings(headerPath, outPath, donePaths):
    napiBindingsPath = os.path.join(outPath, 'generated', 'napi-'+os.path.splitext(os.path.basename(headerPath))[0])
    typescriptBindingsPath = os.path.join(outPath, 'generated', os.path.splitext(os.path.basename(headerPath))[0]+'.d.ts')
    with open(headerPath, 'r') as f:
        header = f.read()
    entities = parseHeader(header)
    (napiBindings, napiHeader) = generateNapiBindings(entities, headerPath, donePaths)
    writeFile(napiBindingsPath+'.cpp', preamble()+napiBindings)
    writeFile(napiBindingsPath+'.h', preamble()+napiHeader)
    typescriptBindings = generateTypescriptBindings(entities)
    writeFile(typescriptBindingsPath, preamble()+typescriptBindings)

def generateTopLevelBindings(paths, outPath):
    typescriptBindingsPath = os.path.join(outPath, 'generated', 'index.d.ts')
    typescriptBindings = generateTopLevelTypescriptBindings()
    writeFile(typescriptBindingsPath, preamble()+typescriptBindings)

if len(sys.argv) >= 2:
    outPath = sys.argv[1]
    paths = sys.argv[2:]
    donePaths = []
    for path in paths:
        generateBindings(path, outPath, donePaths)
        donePaths.append(path)
    generateTopLevelBindings(donePaths, outPath)

else:
    print("Usage: python generate-api-bindings.py <output path> <api headers ...>", file=sys.stderr)
