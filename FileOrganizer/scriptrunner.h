#ifndef SCRIPTRUNNER_H
#define SCRIPTRUNNER_H

#include <QObject>
#include <QProcess>

/**
 * @brief Klasa do uruchamiania skryptów systemowych
 */
class ScriptRunner : public QObject
{
    Q_OBJECT

public:
    explicit ScriptRunner(QObject *parent = nullptr);
    void runScript(const QString &scriptContent);
    static QString detectOS();

signals:
    void finished(const QString &output);
    void errorOccurred(const QString &error);

private:
    QProcess *process;
};

#endif // SCRIPTRUNNER_H