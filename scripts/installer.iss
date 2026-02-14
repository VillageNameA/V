; V Music Player 安装脚本 - 52 语言选项 (50 种语言 + 跟随系统 + 简体中文)

[Setup]
AppName=V Music Player
AppVersion=1.0
DefaultDirName={autopf}\V
DefaultGroupName=V
UninstallDisplayIcon={app}\v.exe
Compression=lzma2/ultra64
InternalCompressLevel=max
SolidCompression=yes
LZMADictionarySize=65536
OutputDir=..\install
OutputBaseFilename=V_Player_Setup
SetupIconFile=..\resources\app_icon.ico

; --- 逻辑控制 ---
LanguageDetectionMethod=none
ShowLanguageDialog=yes
UsePreviousLanguage=no

DisableWelcomePage=no
DisableDirPage=no
DisableProgramGroupPage=yes
DisableReadyPage=yes

[Languages]
; 顺序：compiler:Default.isl 在前提供英文基础，自定义 .isl 在后覆盖
; === 1. 简体中文 (默认) ===
Name: "zh"; MessagesFile: "compiler:Default.isl,lang\zh.isl"
; === 2. 跟随系统 ===
Name: "sys"; MessagesFile: "compiler:Default.isl,lang\sys.isl"
; === 3. 英语 ===
Name: "en"; MessagesFile: "compiler:Default.isl,lang\en.isl"

; === 4. 其余 47 种语言 (按字母排序) ===
Name: "am"; MessagesFile: "compiler:Default.isl,lang\am.isl"
Name: "ar"; MessagesFile: "compiler:Default.isl,lang\ar.isl"
Name: "az"; MessagesFile: "compiler:Default.isl,lang\az.isl"
Name: "bn"; MessagesFile: "compiler:Default.isl,lang\bn.isl"
Name: "cs"; MessagesFile: "compiler:Default.isl,lang\cs.isl"
Name: "de"; MessagesFile: "compiler:Default.isl,lang\de.isl"
Name: "el"; MessagesFile: "compiler:Default.isl,lang\el.isl"
Name: "es"; MessagesFile: "compiler:Default.isl,lang\es.isl"
Name: "fa"; MessagesFile: "compiler:Default.isl,lang\fa.isl"
Name: "fr"; MessagesFile: "compiler:Default.isl,lang\fr.isl"
Name: "gu"; MessagesFile: "compiler:Default.isl,lang\gu.isl"
Name: "ha"; MessagesFile: "compiler:Default.isl,lang\ha.isl"
Name: "hi"; MessagesFile: "compiler:Default.isl,lang\hi.isl"
Name: "hu"; MessagesFile: "compiler:Default.isl,lang\hu.isl"
Name: "id"; MessagesFile: "compiler:Default.isl,lang\id.isl"
Name: "ig"; MessagesFile: "compiler:Default.isl,lang\ig.isl"
Name: "it"; MessagesFile: "compiler:Default.isl,lang\it.isl"
Name: "ja"; MessagesFile: "compiler:Default.isl,lang\ja.isl"
Name: "jv"; MessagesFile: "compiler:Default.isl,lang\jv.isl"
Name: "kn"; MessagesFile: "compiler:Default.isl,lang\kn.isl"
Name: "ko"; MessagesFile: "compiler:Default.isl,lang\ko.isl"
Name: "ku"; MessagesFile: "compiler:Default.isl,lang\ku.isl"
Name: "ml"; MessagesFile: "compiler:Default.isl,lang\ml.isl"
Name: "mr"; MessagesFile: "compiler:Default.isl,lang\mr.isl"
Name: "my"; MessagesFile: "compiler:Default.isl,lang\my.isl"
Name: "nl"; MessagesFile: "compiler:Default.isl,lang\nl.isl"
Name: "pa"; MessagesFile: "compiler:Default.isl,lang\pa.isl"
Name: "pl"; MessagesFile: "compiler:Default.isl,lang\pl.isl"
Name: "ps"; MessagesFile: "compiler:Default.isl,lang\ps.isl"
Name: "pt"; MessagesFile: "compiler:Default.isl,lang\pt.isl"
Name: "ro"; MessagesFile: "compiler:Default.isl,lang\ro.isl"
Name: "ru"; MessagesFile: "compiler:Default.isl,lang\ru.isl"
Name: "sd"; MessagesFile: "compiler:Default.isl,lang\sd.isl"
Name: "su"; MessagesFile: "compiler:Default.isl,lang\su.isl"
Name: "sv"; MessagesFile: "compiler:Default.isl,lang\sv.isl"
Name: "sw"; MessagesFile: "compiler:Default.isl,lang\sw.isl"
Name: "ta"; MessagesFile: "compiler:Default.isl,lang\ta.isl"
Name: "te"; MessagesFile: "compiler:Default.isl,lang\te.isl"
Name: "th"; MessagesFile: "compiler:Default.isl,lang\th.isl"
Name: "tl"; MessagesFile: "compiler:Default.isl,lang\tl.isl"
Name: "tr"; MessagesFile: "compiler:Default.isl,lang\tr.isl"
Name: "uk"; MessagesFile: "compiler:Default.isl,lang\uk.isl"
Name: "ur"; MessagesFile: "compiler:Default.isl,lang\ur.isl"
Name: "uz"; MessagesFile: "compiler:Default.isl,lang\uz.isl"
Name: "vi"; MessagesFile: "compiler:Default.isl,lang\vi.isl"
Name: "yo"; MessagesFile: "compiler:Default.isl,lang\yo.isl"
Name: "zh_HK"; MessagesFile: "compiler:Default.isl,lang\zh_HK.isl"
Name: "zh_TW"; MessagesFile: "compiler:Default.isl,lang\zh_TW.isl"
Name: "zu"; MessagesFile: "compiler:Default.isl,lang\zu.isl"

[Tasks]
Name: "startmenuicon"; Description: "{cm:CreateStartMenuIcon}"; GroupDescription: "{cm:AdditionIcons}"
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionIcons}"; Flags: unchecked

[Files]
Source: "..\bin\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\V"; Filename: "{app}\v.exe"; Tasks: startmenuicon
Name: "{commondesktop}\V"; Filename: "{app}\v.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\v.exe"; Description: "{cm:LaunchProgram,V Music Player}"; Flags: nowait postinstall skipifsilent
