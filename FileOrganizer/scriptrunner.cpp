#include "scriptrunner.h"
#include <QFile>
#include <QDir>
#include <QTemporaryFile>

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

/**
 * @brief Uruchamia wygenerowany skrypt systemowy
 */
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

    tmpFile.write(scriptContent.toUtf8());
    tmpFile.close();
    QString scriptPath = tmpFile.fileName();

    connect(process, &QProcess::finished, this,
            [this, scriptPath](int exitCode, QProcess::ExitStatus) {
                QString output = process->readAllStandardOutput();
                QString errors = process->readAllStandardError();

                QFile::remove(scriptPath);

                if (!errors.isEmpty()) {
                    emit errorOccurred(errors);
                } else {
                    emit finished(output);
                }
            });

#ifdef Q_OS_WIN
    process->start("powershell",
                   {"-ExecutionPolicy", "Bypass", "-File", scriptPath});
#else
    process->start("bash", {scriptPath});
#endif
}