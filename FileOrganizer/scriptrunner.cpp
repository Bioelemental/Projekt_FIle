#include "scriptrunner.h"
#include <QFile>
#include <QDir>
#include <QTemporaryFile>
#include <QTimer>
#include <QTextStream>

/**
 * @brief Konstruktor workera
 */
ScriptRunnerWorker::ScriptRunnerWorker(QObject *parent) : QObject(parent)
{
}

/**
 * @brief Wykonaj skrypt w wątku workera.
 *
 * - Tworzy plik tymczasowy, uruchamia QProcess w tym samym wątku (bez blokowania UI).
 * - Obsługuje timeout i zwraca wynik przez sygnały.
 */
void ScriptRunnerWorker::doRunScript(const QString &scriptContent)
{
    QString os;
#ifdef Q_OS_WIN
    os = "Windows";
#else
    os = "Linux";
#endif
    QString suffix = (os == "Windows") ? ".ps1" : ".sh";

    QTemporaryFile tmpFile;
    tmpFile.setFileTemplate(QDir::tempPath() + "/organizer_XXXXXX" + suffix);
    tmpFile.setAutoRemove(false);

    if (!tmpFile.open()) {
        emit errorOccurred("Nie można utworzyć pliku tymczasowego!");
        return;
    }

#ifdef Q_OS_WIN
    tmpFile.write("\xEF\xBB\xBF");
#endif
    tmpFile.write(scriptContent.toUtf8());
    tmpFile.close();
    QString scriptPath = tmpFile.fileName();

    // Utwórz QProcess jako dziecko workera — będzie działał w tym wątku
    QProcess *process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);

    // Po zakończeniu procesu: odczytaj output / errors i usuń plik tymczasowy
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, process, scriptPath](int /*exitCode*/, QProcess::ExitStatus) {
                QString output = process->readAllStandardOutput();
                QString errors = process->readAllStandardError();

                QFile::remove(scriptPath);

                if (!errors.isEmpty()) {
                    emit errorOccurred(errors.trimmed());
                } else {
                    emit finished(output.trimmed());
                }

                process->deleteLater();
            });

    // Timeout wykonania procesu (60s)
    QTimer *procTimer = new QTimer(process);
    procTimer->setSingleShot(true);
    procTimer->start(60000);
    connect(procTimer, &QTimer::timeout, process, [this, process, scriptPath]() {
        if (process->state() != QProcess::NotRunning) {
            process->kill();
            QFile::remove(scriptPath);
            emit errorOccurred("Przekroczono czas wykonania skryptu (timeout).");
        }
    });

#ifdef Q_OS_WIN
    process->start("powershell", {"-ExecutionPolicy", "Bypass", "-File", scriptPath});
#else
    QFile::setPermissions(scriptPath, QFile::permissions(scriptPath) | QFileDevice::ExeUser);
    process->start("bash", {scriptPath});
#endif
}

/**
 * @brief Konstruktor ScriptRunner — tworzy wątek i workera
 */
ScriptRunner::ScriptRunner(QObject *parent) : QObject(parent)
{
    workerThread = new QThread(this);
    worker = new ScriptRunnerWorker();
    worker->moveToThread(workerThread);

    // Przekierowanie żądań do workera (kolejkowane połączenie)
    connect(this, &ScriptRunner::requestRun,
            worker, &ScriptRunnerWorker::doRunScript,
            Qt::QueuedConnection);

    // Forward sygnałów z workera do użytkowników ScriptRunner
    connect(worker, &ScriptRunnerWorker::finished,
            this, &ScriptRunner::finished);
    connect(worker, &ScriptRunnerWorker::errorOccurred,
            this, &ScriptRunner::errorOccurred);

    // Upewnij się, że worker zostanie posprzątany razem z wątkiem
    connect(workerThread, &QThread::finished, worker, &QObject::deleteLater);

    workerThread->start();
}

/**
 * @brief Destruktor — zamyka wątek i czeka
 */
ScriptRunner::~ScriptRunner()
{
    if (workerThread) {
        workerThread->quit();
        workerThread->wait();
        // worker zostanie usunięty przez powiązanie z finished
        delete workerThread;
        workerThread = nullptr;
        worker = nullptr;
    }
}

/**
 * @brief Publiczne API — żądanie uruchomienia skryptu (nie blokuje)
 */
void ScriptRunner::runScript(const QString &scriptContent)
{
    // Emitujemy sygnał, worker wykona pracę w swoim wątku (QueuedConnection).
    emit requestRun(scriptContent);
}

/**
 * @brief Wykrywa system operacyjny (bez zmian)
 */
QString ScriptRunner::detectOS()
{
#ifdef Q_OS_WIN
    return "Windows";
#else
    return "Linux";
#endif
}