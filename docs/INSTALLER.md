# Installer Guide (Inno Setup)

Installer script: `scripts/installer.iss`

## Language Behavior

Current setup:

- `UsePreviousLanguage=no`
- `ShowLanguageDialog=yes`
- `LanguageDetectionMethod=uilanguage`

Meaning:

- installer defaults to current OS UI language
- language selection page is shown every install
- user can override language manually

## Included Installer Languages

Defined in `scripts/installer.iss`:

- English (`en`)
- Simplified Chinese (`zh`)

These are installer UI languages only. They are independent from app runtime language packs.

## Build Installer

1. Build app binaries into `bin/`
2. Open `scripts/installer.iss` with Inno Setup
3. Compile script

Output:

- folder: `install/`
- file name: `V_Player_Setup.exe`

## What Gets Packaged

From `[Files]` section:

- everything under `..\bin\*` is copied to `{app}`
- recursive copy is enabled

Ensure `bin/` contains required Qt and FFmpeg runtime DLLs before packaging.

## Post-install Launch

From `[Run]` section:

- launches `{app}\v.exe` after install
- skipped in silent mode

## Troubleshooting

### Installer UI language is unexpected

- verify Windows display language
- verify installer has matching language entry
- verify `LanguageDetectionMethod=uilanguage` remains unchanged

### App language after install is unexpected

Installer language and app language are separate:

- installer controls setup wizard text
- app runtime language uses app setting `General/Language`
- when app is set to `跟随系统`, app resolves locale in `src/main.cpp`
