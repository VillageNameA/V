QT       += core gui multimedia widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG -= ltcc
CONFIG -= lto
QMAKE_LFLAGS += -fno-use-linker-plugin

QMAKE_CXXFLAGS += -finput-charset=UTF-8 -fexec-charset=UTF-8

# FFmpeg配置
FFMPEG_PATH = $$PWD/third_party/ffmpeg
INCLUDEPATH += $$FFMPEG_PATH/include
LIBS += -L$$FFMPEG_PATH/lib -lavcodec -lavformat -lavutil -lswresample

# 可执行文件名称
TARGET = v
TEMPLATE = app

# 包含路径
INCLUDEPATH += $$PWD/src

# 源文件列表
SOURCES += src/main.cpp \
           src/gui/mainwindow.cpp \
           src/gui/mainwindow_playlist.cpp \
           src/gui/mainwindow_playback.cpp \
           src/gui/PlaylistDelegate.cpp \
           src/gui/PlaylistItem.cpp \
           src/core/AudioPlayer.cpp

# 头文件列表
HEADERS  += src/gui/mainwindow.h \
            src/gui/PlaylistDelegate.h \
            src/gui/PlaylistItem.h \
            src/core/AudioPlayer.h

# UI文件
FORMS    += src/gui/mainwindow.ui

# 资源文件
RESOURCES += resources/resources.qrc

# 多语言翻译文件
TRANSLATIONS += resources/translations/v_en_US.ts \
                resources/translations/v_zh_HK.ts \
                resources/translations/v_zh_TW.ts \
                resources/translations/v_hi_IN.ts \
                resources/translations/v_es_ES.ts \
                resources/translations/v_fr_FR.ts \
                resources/translations/v_ar_SA.ts \
                resources/translations/v_bn_BD.ts \
                resources/translations/v_pt_PT.ts \
                resources/translations/v_ru_RU.ts \
                resources/translations/v_ur_PK.ts \
                resources/translations/v_id_ID.ts \
                resources/translations/v_de_DE.ts \
                resources/translations/v_ja_JP.ts \
                resources/translations/v_mr_IN.ts \
                resources/translations/v_te_IN.ts \
                resources/translations/v_tr_TR.ts \
                resources/translations/v_ta_IN.ts \
                resources/translations/v_vi_VN.ts \
                resources/translations/v_tl_PH.ts \
                resources/translations/v_ko_KR.ts \
                resources/translations/v_fa_IR.ts \
                resources/translations/v_ha_NG.ts \
                resources/translations/v_sw_KE.ts \
                resources/translations/v_jv_ID.ts \
                resources/translations/v_it_IT.ts \
                resources/translations/v_pa_PK.ts \
                resources/translations/v_kn_IN.ts \
                resources/translations/v_gu_IN.ts \
                resources/translations/v_th_TH.ts \
                resources/translations/v_am_ET.ts \
                resources/translations/v_yo_NG.ts \
                resources/translations/v_my_MM.ts \
                resources/translations/v_ps_AF.ts \
                resources/translations/v_uk_UA.ts \
                resources/translations/v_su_ID.ts \
                resources/translations/v_pl_PL.ts \
                resources/translations/v_uz_UZ.ts \
                resources/translations/v_ml_IN.ts \
                resources/translations/v_sd_PK.ts \
                resources/translations/v_ro_RO.ts \
                resources/translations/v_az_AZ.ts \
                resources/translations/v_ku_TR.ts \
                resources/translations/v_nl_NL.ts \
                resources/translations/v_ig_NG.ts \
                resources/translations/v_zu_ZA.ts \
                resources/translations/v_cs_CZ.ts \
                resources/translations/v_el_GR.ts \
                resources/translations/v_hu_HU.ts \
                resources/translations/v_sv_SE.ts

# 调试/发布版本区分
CONFIG(debug, debug|release) {
    DEFINES += QT_DEBUG
} else {
    DEFINES += QT_RELEASE
    DEFINES += QT_NO_DEBUG_OUTPUT  # 关闭发布版本调试输出
}

# 输出路径
DESTDIR = ./bin

RC_ICONS = resources/app_icon.ico