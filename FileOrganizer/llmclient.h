#ifndef LLMCLIENT_H
#define LLMCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>


class LlmClient : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor
     * @param parent rodzic QObject
     */
    explicit LlmClient(QObject *parent = nullptr);

  
    void generateScript(const QString &userPrompt,
                        const QString &folderPath,
                        const QString &os);

signals:
    /** Emitted when a cleaned script is ready. */
    void scriptReady(const QString &script);

    /** Emitted on error with human-readable message. */
    void errorOccurred(const QString &error);

private:
    QNetworkAccessManager *manager;
    const QString API_URL = "http://localhost:11434/api/generate";
    const QString MODEL = "SpeakLeash/bielik-11b-v3.0-instruct:Q4_K_M";
};

#endif // LLMCLIENT_H