#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include <QObject>
#include <QProcess>
#include <QThread>

/**
 * @brief Worker wykonujący skrypt w osobnym wątku.
 *
 * Implementuje rzeczywiste, widoczne wielowątkowe wykonanie skryptu:
 * - slot `doRunScript` uruchamiany jest w wątku roboczym (QThread).
 * - emituje sygnały `finished` i `errorOccurred`.
 */
class ScriptRunnerWorker : public QObject
{
    Q_OBJECT
public:
    explicit ScriptRunnerWorker(QObject *parent = nullptr);

public slots:
    void doRunScript(const QString &scriptContent);

signals:
    void finished(const QString &output);
    void errorOccurred(const QString &error);
};

/**
 * @brief Klasa zarządzająca workerem i wątkiem.
 *
 * - Tworzy QThread i przenosi do niego ScriptRunnerWorker.
 * - `runScript()` emituje sygnał `requestRun` przekazywany do workera (QueuedConnection).
 */
class ScriptRunner : public QObject
{
    Q_OBJECT

public:
    explicit ScriptRunner(QObject *parent = nullptr);
    ~ScriptRunner();

    void runScript(const QString &scriptContent);
    static QString detectOS();

signals:
    void finished(const QString &output);
    void errorOccurred(const QString &error);

    // internal: żądanie uruchomienia skryptu (połączone z workerem)
    void requestRun(const QString &script);

private:
    QThread *workerThread;
    ScriptRunnerWorker *worker;
};

#endif // SCRIPTRUNNER_H