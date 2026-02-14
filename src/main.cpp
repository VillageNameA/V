#include <QApplication>
#include "gui/mainwindow.h"
#include <QMessageBox>
#include <QLocalSocket>
#include <QSettings>
#include <QTranslator>
#include <QLocale>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <exception>
#include <QThread>

static void appendLanguageLog(const QString &message)
{
    QFile logFile(QDir::tempPath() + "/V_language_startup.log");
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }
    QTextStream out(&logFile);
    out << QDateTime::currentDateTime().toString(Qt::ISODate) << " " << message << "\n";
}

static QString pickSystemLanguage()
{
    static const QStringList supported = {
        "en_US","zh_HK","zh_TW","hi_IN","es_ES","fr_FR","ar_SA","bn_BD","pt_PT","ru_RU",
        "ur_PK","id_ID","de_DE","ja_JP","mr_IN","te_IN","tr_TR","ta_IN","vi_VN","tl_PH",
        "ko_KR","fa_IR","ha_NG","sw_KE","jv_ID","it_IT","pa_PK","kn_IN","gu_IN","th_TH",
        "am_ET","yo_NG","my_MM","ps_AF","uk_UA","su_ID","pl_PL","uz_UZ","ml_IN","sd_PK",
        "ro_RO","az_AZ","ku_TR","nl_NL","ig_NG","zu_ZA","cs_CZ","el_GR","hu_HU","sv_SE"
    };

    const QString systemLocale = QLocale::system().name();
    if (supported.contains(systemLocale)) {
        return systemLocale;
    }

    const QString languagePrefix = systemLocale.section('_', 0, 0);
    for (const QString &locale : supported) {
        if (locale.startsWith(languagePrefix + "_")) {
            return locale;
        }
    }

    return "zh_CN";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    const QStringList arguments = app.arguments();
    const bool isRestartLaunch = arguments.contains("--restart");

    // 启动时加载语言
    QSettings settings("MyCompany", "MusicPlayer");
    const QString settingLang = settings.value("General/Language", "default").toString();
    QString resolvedLang = settingLang;
    if (resolvedLang == "default") {
        resolvedLang = pickSystemLanguage();
    }

    QTranslator translator;
    const QString qmPath = QString(":/translations/translations/v_%1.qm").arg(resolvedLang);
    const bool qmExists = QFile::exists(qmPath);
    bool translatorLoaded = false;
    if (resolvedLang != "zh_CN") {
        if (translator.load(qmPath) && !translator.isEmpty()) {
            app.installTranslator(&translator);
            translatorLoaded = true;
        }
    }
    appendLanguageLog(QString("exe=%1 setting=%2 resolved=%3 qmPath=%4 exists=%5 loaded=%6")
        .arg(QCoreApplication::applicationFilePath())
        .arg(settingLang)
        .arg(resolvedLang)
        .arg(qmPath)
        .arg(qmExists ? "true" : "false")
        .arg(translatorLoaded ? "true" : "false"));

    // 单实例运行检查：使用本地套接字尝试连接已有实例
    QString serverName = "V_MusicPlayer_Server";
    QLocalSocket socket;
    bool hasRunningInstance = false;
    for (int i = 0; i < (isRestartLaunch ? 12 : 1); ++i) {
        socket.abort();
        socket.connectToServer(serverName);
        if (socket.waitForConnected(150)) {
            hasRunningInstance = true;
            socket.disconnectFromServer();
            if (!isRestartLaunch) {
                return 0;
            }
            QThread::msleep(120);
            continue;
        }
        hasRunningInstance = false;
        break;
    }
    if (hasRunningInstance) {
        return 0;
    }

    try {
        MainWindow window;
        window.show();
        
        return app.exec();
    } catch (const std::exception &e) {
        QMessageBox::critical(nullptr, "错误", QString("程序异常: %1").arg(e.what()));
        return 1;
    } catch (...) {
        QMessageBox::critical(nullptr, "错误", "程序发生未知异常");
        return 1;
    }
}
