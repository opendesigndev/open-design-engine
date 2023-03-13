const ode = require('${ODE_NATIVE_MODULE_PATH}')

export default async function init() {
    try { return await import("@opendesign/engine-unknown-nonexistent") } catch {}
    try { return await import("@opendesign/engine-${ODE_PLATFORM_OS}-${ODE_PLATFORM_CPU}") } catch {}
    return await import('@opendesign/engine-source')
}

export const wasm = 'node wrapper package'
export const version = '${ODE_VERSION_NPM}'
