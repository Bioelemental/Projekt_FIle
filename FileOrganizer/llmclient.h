#ifndef LLMCLIENT_H
#define LLMCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>


/**
 * @ Klasa do komunikacji z lokalnym modelem LLM przez REST API.
 * - Komunikacja z modelem lokalnym (Ollama) przez REST API: implementacja w `generateScript()` (llmclient.cpp).
 * 
 */
class LlmClient : public QObject
{
    Q_OBJECT
public:
    /**
     * Konstruktor
     * parent rodzic QObject
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
    // Adres lokalnego API Ollama
    const QString API_URL = "http://localhost:11434/api/generate";
    // U¿ywany model 
    const QString MODEL = "SpeakLeash/bielik-11b-v3.0-instruct:Q4_K_M";
};

#endif // LLMCLIENT_H