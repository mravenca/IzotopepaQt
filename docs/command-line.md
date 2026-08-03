# Command-line options

Developer Tools DT1 adds direct startup options to the game executable.

```bash
./build/IzotopepaQtGameV3 --level 7
./build/IzotopepaQtGameV3 --level-file assets/levels/test.json
./build/IzotopepaQtGameV3 --debug
./build/IzotopepaQtGameV3 --god
```

Options may be combined:

```bash
./build/IzotopepaQtGameV3 --level 10 --debug --god
```

`--level` uses human-readable numbers from 1 through 10. Internally the
campaign remains zero-based. `--level` and `--level-file` are mutually
exclusive.

`--level-file` uses the existing level-path resolver, so absolute paths,
relative paths, files under `assets/levels`, files beside the executable,
and embedded resource levels are supported.

DT1 only adds command-line startup behavior. Save isolation and runtime
reload shortcuts are planned for the later Developer Tools phases.
