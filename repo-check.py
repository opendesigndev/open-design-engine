
# This script checks the repository for lint, such as:
#   - Generated files not up to date
#   - Wrong line terminators
#   - Indentation with tabs
#   - Missing line terminator at end of text file
#   - Trailing whitespace on line (just a warning)
#   - Wrong commit message format

import os
import sys
import glob
import re
from fnmatch import fnmatch

textFileExtensions = 'h,hpp,c,cpp,txt,cmake,py,sh,md,json,yml,html,css,ts,shadron,editorconfig,gitattributes,gitconfig,gitignore,dockerignore'

root = os.path.dirname(os.path.realpath(__file__))

with open(os.path.join(root, '.gitignore')) as f:
    gitignore = [line.strip() for line in f.read().splitlines() if line and not '#' in line]
gitignore.append('liboctopus/')
gitignore.append('open-design-text-renderer/')
gitignore.append('third-party/')

ignorePatterns = []
for ignore in gitignore:
    ignorePatterns.append((ignore+'/**').replace('//', '/'))
    if not ignore.startswith('/'):
        ignorePatterns.append(('**/'+ignore+'/**').replace('//', '/'))
        if not ignore.endswith('/'):
            ignorePatterns.append(('**/'+ignore).replace('//', '/'))

def ignoredPath(relPath):
    relPath = relPath.replace(os.path.sep, '/')
    for ignore in ignorePatterns:
        if fnmatch(relPath, ignore):
            return True
    return False

textFileExtensions = ['.'+x for x in textFileExtensions.split(',')]

def isTextFile(path):
    for extension in textFileExtensions:
        if (path.endswith(extension)):
            return True
    if os.path.basename(path) == 'Dockerfile':
        return True
    return False

ok = True

def fileError(relPath, msg, text = None, needle = None, notOk = True):
    fullMsg = ('Error: ' if notOk else 'Warning: ')+msg+' in '+relPath
    if text and needle:
        pos = text.find(needle)
        if pos >= 0:
            lineNo = text.count('\n', 0, pos)+1
            fullMsg += ' @ line '+str(lineNo)
    print(fullMsg, file=sys.stderr)
    if notOk:
        ok = False

if len(sys.argv) >= 2:
    if not re.match(r'^[A-Z`]', sys.argv[1]):
        print('Error: Wrong format of commit message', file=sys.stderr)
        ok = False


for path in glob.iglob(root+'/**/*', recursive=True):
    relPath = os.path.relpath(path, root)
    if isTextFile(relPath) and not ignoredPath(relPath):
        with open(path, 'r') as f:
            text = f.read()
        if '\t' in text:
            fileError(relPath, 'Tabs', text, '\t')
        if '\r' in text:
            fileError(relPath, 'CR', text, '\r')
        if ' \n' in text:
            fileError(relPath, 'Trailing whitespace', text, ' \n', False)
        if not text.endswith('\n'):
            fileError(relPath, 'Terminating newline missing', text)

exit(0 if ok else 1)
