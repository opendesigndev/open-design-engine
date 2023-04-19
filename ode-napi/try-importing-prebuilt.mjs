import process from 'node:process'
import fs from 'node:fs'

// This file is here to skip building from source, if prebuilt package is already
// available
(async () => {
    const { optionalDependencies } = JSON.parse(
        await fs.promises.readFile(new URL('./wrapper-package/package.json', import.meta.url), 'utf-8'),
    )
    for (const dep of Object.keys(optionalDependencies)) {
        try { return await import(dep) } catch {}
    }
    return false
})().then((res) => process.exit(res ? 0 : 1))
