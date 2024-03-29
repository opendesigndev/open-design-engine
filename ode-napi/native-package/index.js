import mod from 'node:module'
const require = mod.createRequire(import.meta.url)
const ode = require('${ODE_NATIVE_MODULE_PATH}')

export default async function init() {
    return ode
}

export const wasm = 'node native module'
export const version = '${ODE_NPM_VERSION}'
