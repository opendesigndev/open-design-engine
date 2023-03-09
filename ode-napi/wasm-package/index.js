import load from './ode.js'
import * as emnapi from '@emnapi/runtime'

export default async function init(options) {
    const odeMod = await load(options)
    return odeMod.emnapiInit({ context: emnapi.getDefaultContext() })
}

export const wasm = new URL("./ode.wasm", import.meta.url).href
export const version = '${ODE_VERSION_NPM}'
