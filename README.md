# V Music Player

Qt desktop music player with playlists, tray support, single-instance startup, and built-in offline translations.

## 🚀 Download & Install

If you just want to use the player, go to the [**GitHub Releases**](https://github.com/YOUR_USERNAME/V/releases) page and download the latest `V_Player_Setup.exe`. 

After downloading, run the installer to install the player. Once finished, you can launch it directly from your desktop or start menu.

## Quick Start

### Build

Recommended (project-provided script):

```powershell
build.bat
```

Manual build:

```powershell
qmake MusicPlayer.pro
mingw32-make
```

Output executable: `bin/v.exe`

### Package Installer

Use Inno Setup with `scripts/installer.iss`.

## Requirements

- Qt 6 (`Core`, `Gui`, `Widgets`, `Multimedia`)
- C++17 toolchain
- FFmpeg headers/libs in `third_party/ffmpeg`
- Inno Setup (for installer packaging)

## Language Behavior

### In-app option

- Language dropdown contains `跟随系统` (`default` setting value).
- Settings key:
  - Organization: `MyCompany`
  - Application: `MusicPlayer`
  - Key: `General/Language`

### `跟随系统` resolution order

Implemented in `src/main.cpp`:

1. Exact system locale match (example: `fr_FR`)
2. Same language prefix fallback (example: `fr_*`)
3. Final fallback `zh_CN`

### Restart after language change

- Selection change writes setting immediately with `settings.sync()`
- Relaunch uses `--restart`
- Startup has restart-safe single-instance retry to avoid race condition

## Offline Translations

Runtime does not use online translation.

- Editable sources: `resources/translations/v_*.ts`
- Compiled files used by app: `resources/translations/v_*.qm`
- Resource registration: `resources/resources.qrc`

Generate/update `.qm` after editing `.ts`:

```powershell
lrelease MusicPlayer.pro
```

If `lrelease` is not in `PATH`, run it from your local Qt installation directory.

## Installer Language

Configured in `scripts/installer.iss`:

- `UsePreviousLanguage=no`
- `ShowLanguageDialog=yes`
- `LanguageDetectionMethod=uilanguage`

Effect:

- Installer defaults to OS UI language
- User can still manually choose language on installer language page

## Documentation

- Localization workflow: `docs/LOCALIZATION.md`
- Installer behavior/details: `docs/INSTALLER.md`

## Project Layout

- `src/`: app source
- `resources/`: icon and translation resources
- `scripts/`: build/deploy helper scripts
- `third_party/ffmpeg/`: FFmpeg dependency files
