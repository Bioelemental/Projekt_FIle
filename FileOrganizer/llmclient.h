#ifndef LLMCLIENT_H
#define LLMCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>


/**
 * @brief Klasa do komunikacji z lokalnym modelem LLM przez REST API.
 *
 * @details Wysy³a prompt (system + user) i emituje sygna³y z wynikiem.
 */
class LlmClient : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor
     * @param parent rodzic QObject
     */
    explicit LlmClient(QObject *parent = nullptr);

    /**
     * @brief Wygeneruj skrypt na podstawie promptu.
     * @param userPrompt polecenie od u¿ytkownika (polski/angielski)
     * @param folderPath œcie¿ka do folderu docelowego
     * @param os identyfikator systemu ("Windows" lub "Linux")
     *
     * Wynik zostanie dostarczony asynchronicznie:
     * - @c scriptReady(zawiera skrypt) lub
     * - @c errorOccurred(z komunikatem b³êdu)
     */
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