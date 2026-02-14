# Localization Guide

This project uses Qt translation files and ships translations offline inside app resources.

## File Model

- Source texts: code and `.ui` files under `src/`
- Translation sources: `resources/translations/v_*.ts`
- Compiled translation packs: `resources/translations/v_*.qm`
- Resource registration: `resources/resources.qrc`

## Runtime Loading Rules

Entry point: `src/main.cpp`

- Read `General/Language` from `QSettings`
- If value is `default`, resolve using system locale:
1. exact locale
2. same language prefix
3. fallback to `zh_CN`
- For non-`zh_CN`, load `:/translations/v_<locale>.qm`

## Supported Locales

Configured in code and translation files:

- `en_US`, `zh_CN`, `zh_HK`, `zh_TW`, `hi_IN`, `es_ES`, `fr_FR`, `ar_SA`, `bn_BD`, `pt_PT`
- `ru_RU`, `ur_PK`, `id_ID`, `de_DE`, `ja_JP`, `mr_IN`, `te_IN`, `tr_TR`, `ta_IN`, `vi_VN`
- `tl_PH`, `ko_KR`, `fa_IR`, `ha_NG`, `sw_KE`, `jv_ID`, `it_IT`, `pa_PK`, `kn_IN`, `gu_IN`
- `th_TH`, `am_ET`, `yo_NG`, `my_MM`, `ps_AF`, `uk_UA`, `su_ID`, `pl_PL`, `uz_UZ`, `ml_IN`
- `sd_PK`, `ro_RO`, `az_AZ`, `ku_TR`, `nl_NL`, `ig_NG`, `zu_ZA`, `cs_CZ`, `el_GR`, `hu_HU`, `sv_SE`

## Edit Workflow (Offline)

1. Edit target `.ts` file(s), example: `resources/translations/v_fr_FR.ts`
2. Keep placeholders unchanged, example `%1`, `%2`
3. Compile all translations:

```powershell
lrelease MusicPlayer.pro
```

4. Rebuild app and verify UI text

## Batch Helper Script

Script: `scripts/apply_offline_translations.py`

Purpose:

- applies predefined offline translation mappings to a subset of locales
- useful for repetitive string updates

Run:

```powershell
python scripts/apply_offline_translations.py
lrelease MusicPlayer.pro
```

## Verification Checklist

- selected language persists across restart
- `跟随系统` chooses expected locale on fresh start
- no untranslated critical controls:
  - window title
  - play/pause/next/prev
  - playlist actions
  - close confirmation dialog
  - language-change restart prompt

## Common Issues

### `lrelease` not found

Run `lrelease.exe` from Qt install path or add Qt `bin` to `PATH`.

### Language changed but text did not update

- ensure corresponding `.qm` exists and is non-empty
- ensure `.qm` is included in `resources/resources.qrc`
- restart app after change (by design)
