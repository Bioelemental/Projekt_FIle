#include "scriptrunner.h"
#include <QFile>
#include <QDir>
#include <QTemporaryFile>
#include <QTimer>
#include <QTextStream>

/**
 * @brief Konstruktor ScriptRunner
 */
ScriptRunner::ScriptRunner(QObject *parent) : QObject(parent)
{
    process = new QProcess(this);
}

/**
 * @brief Wykrywa system operacyjny
 */
QString ScriptRunner::detectOS()
{
#ifdef Q_OS_WIN
    return "Windows";
#else
    return "Linux";
#endif
}


void ScriptRunner::runScript(const QString &scriptContent)
{
    QString os = detectOS();
    QString suffix = (os == "Windows") ? ".ps1" : ".sh";

    QTemporaryFile tmpFile;
    tmpFile.setFileTemplate(QDir::tempPath() + "/organizer_XXXXXX" + suffix);
    tmpFile.setAutoRemove(false);

    if (!tmpFile.open()) {
        emit errorOccurred("Nie można utworzyć pliku tymczasowego!");
        return;
    }

    // Na Windows dopisujemy BOM, aby PowerShell poprawnie czytał polskie znaki w starszych konfiguracjach
#ifdef Q_OS_WIN
    tmpFile.write("\xEF\xBB\xBF");
#endif
    tmpFile.write(scriptContent.toUtf8());
    tmpFile.close();
    QString scriptPath = tmpFile.fileName();

    // Upewnij się że nie ma starych połączeń
    process->disconnect();

    process->setProcessChannelMode(QProcess::MergedChannels);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, scriptPath](int exitCode, QProcess::ExitStatus) {
                QString output = process->readAllStandardOutput();
                QString errors = process->readAllStandardError();

                // Usuń plik tymczasowy
                QFile::remove(scriptPath);

                if (!errors.isEmpty()) {
                    emit errorOccurred(errors.trimmed());
                } else {
                    emit finished(output.trimmed());
                }
            });

    // Timeout wykonania procesu (60s)
    QTimer *procTimer = new QTimer(process);
    procTimer->setSingleShot(true);
    procTimer->start(60000);
    connect(procTimer, &QTimer::timeout, process, [this, scriptPath]() {
        if (process->state() != QProcess::NotRunning) {
            process->kill();
            QFile::remove(scriptPath);
            emit errorOccurred("Przekroczono czas wykonania skryptu (timeout).");
        }
    });

#ifdef Q_OS_WIN
    // Uruchom PowerShell z bypass execution policy
    process->start("powershell", {"-ExecutionPolicy", "Bypass", "-File", scriptPath});
#else
    // Nadaj prawa wykonywania i uruchom bash
    QFile::setPermissions(scriptPath, QFile::permissions(scriptPath) | QFileDevice::ExeUser);
    process->start("bash", {scriptPath});
#endif
}