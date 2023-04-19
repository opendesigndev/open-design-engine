import process from 'node:process'
import fs from 'node:fs'
export { wasm } from '@opendesign/engine-wasm'

export default async function init() {
    const native = await findAndImport()
    return native.default()
}

async function findAndImport() {
    const { optionalDependencies } = JSON.parse(
        await fs.promises.readFile(new URL('./package.json', import.meta.url), 'utf-8'),
    )
    for (const dep of Object.keys(optionalDependencies)) {
        try {
            return await import(dep)
        } catch {}
    }
    throw new Error("Failed to import any native module package for Open Design Engine.")
}

export const version = '${ODE_NPM_VERSION}'
