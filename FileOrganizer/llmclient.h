#ifndef LLMCLIENT_H
#define LLMCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

/**
 * @brief Klasa do komunikacji z lokalnym modelem LLM przez REST API
 */
class LlmClient : public QObject
{
    Q_OBJECT

public:
    explicit LlmClient(QObject *parent = nullptr);
    void generateScript(const QString &userPrompt,
                        const QString &folderPath,
                        const QString &os);

signals:
    void scriptReady(const QString &script);
    void errorOccurred(const QString &error);

private:
    QNetworkAccessManager *manager;
    const QString API_URL = "http://localhost:11434/api/generate";
    const QString MODEL = "SpeakLeash/bielik-11b-v3.0-instruct:Q4_K_M";
};

#endif // LLMCLIENT_H