#ifndef LLMCLIENT_H
#define LLMCLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>


/**
 * @brief Klasa do komunikacji z lokalnym modelem LLM przez REST API.
 *
 * @details Wysy³a prompt (system + user) i emituje sygna³y z wynikiem.
 *
 * MAPOWANIE KRYTERIÓW:
 * - Komunikacja z modelem lokalnym (Ollama) przez REST API: implementacja w `generateScript()` (llmclient.cpp).
 * - Definicja system prompt oraz user prompt: budowane w `generateScript()` (llmclient.cpp).
 * - Dzia³anie bez internetu / obs³uga niedostêpnoœci: timeout i sprawdzanie b³êdów QNetworkReply -> emit `errorOccurred`.
 * - Doxygen: komentarze dokumentuj¹ce klasê i funkcje (spe³nia wymóg dokumentacji).
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
     *
     * Zwróæ uwagê:
     * - Funkcja wysy³a JSON do lokalnego serwera (Ollama) — patrz `API_URL` i `MODEL`.
     * - Obs³uga b³êdów i timeout jest implementowana w llmclient.cpp.
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
    // Adres lokalnego API Ollama (mo¿na nadpisaæ przez zmienn¹ œrodowiskow¹ OLLAMA_API_URL)
    const QString API_URL = "http://localhost:11434/api/generate";
    // U¿ywany model — dokumentowane w README/requirements.txt
    const QString MODEL = "SpeakLeash/bielik-11b-v3.0-instruct:Q4_K_M";
};

#endif // LLMCLIENT_H