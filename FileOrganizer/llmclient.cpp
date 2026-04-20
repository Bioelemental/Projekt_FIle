#include "llmclient.h"
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QRegularExpression>
/**
 * @brief Konstruktor LlmClient
 */
LlmClient::LlmClient(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
}

/**
 * @brief Wysyła zapytanie do Ollama i odbiera wygenerowany skrypt
 */
void LlmClient::generateScript(const QString &userPrompt,
                               const QString &folderPath,
                               const QString &os)
{
    QString systemPrompt;
    if (os == "Windows") {
        systemPrompt = "Jesteś generatorem skryptów PowerShell. "
                       "Generuj TYLKO czysty kod PowerShell. "
                       "Używaj TYLKO Move-Item do przenoszenia plików. "
                       "NIGDY nie używaj Remove-Item. "
                       "Aby przenieść plik DO folderu użyj: "
                       "Move-Item -Path $file -Destination $folderPath "
                       "gdzie $folderPath to FOLDER nie pełna ścieżka pliku. "
                       "Aby filtrować wiele rozszerzeń ZAWSZE używaj "
                       "osobnego Get-ChildItem dla każdego rozszerzenia osobno, "
                       "NIGDY nie używaj przecinka w -Filter. "
                       "Przykład dla jpg i jpeg: "
                       "Get-ChildItem -Filter *.jpg; Get-ChildItem -Filter *.jpeg. "
                       "Aby utworzyć folder: "
                       "New-Item -ItemType Directory -Force -Path $folder. "
                       "Po każdym przeniesieniu: Write-Host 'MOVED: plik -> cel'. "
                       "Nie dodawaj opisów ani komentarzy.";
    } else {
        systemPrompt = "Jesteś generatorem skryptów Bash. "
                       "Generuj TYLKO czysty kod Bash bez opisów, bez komentarzy. "
                       "Używaj TYLKO mv do przenoszenia plików. "
                       "NIGDY nie używaj rm -rf. "
                       "Aby utworzyć folder użyj: mkdir -p. "
                       "Po każdym przeniesieniu użyj: echo 'MOVED: plik -> cel'. "
                       "NIE pisz słowa MOVED bez echo. "
                       "Nie dodawaj żadnych opisów ani komentarzy poza kodem.";
    }

    QString fullPrompt = QString("Folder: %1\nPolecenie: %2")
                             .arg(folderPath, userPrompt);

    QJsonObject json;
    json["model"] = MODEL;
    json["system"] = systemPrompt;
    json["prompt"] = fullPrompt;
    json["stream"] = false;
    json["options"] = QJsonObject{{"num_predict", 1024}};

    QUrl url(API_URL);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QByteArray data = QJsonDocument(json).toJson();
    QNetworkReply *reply = manager->post(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("Błąd połączenia z Ollama: " +
                               reply->errorString());
            reply->deleteLater();
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        QString script = doc.object()["response"].toString();

        // Usuń blok <think>...</think> który Bielik dodaje
        script.remove(QRegularExpression("<think>[\\s\\S]*?</think>",
                                         QRegularExpression::CaseInsensitiveOption));
        // Usuń markdown
        script.remove(QRegularExpression("```powershell\\s*",
                                         QRegularExpression::CaseInsensitiveOption));
        script.remove(QRegularExpression("```bash\\s*",
                                         QRegularExpression::CaseInsensitiveOption));
        script.remove(QRegularExpression("```\\s*"));

        // Usuń linie które nie są kodem (komentarze opisowe po angielsku/polsku)
        QStringList lines = script.split("\n");
        QStringList cleanLines;
        for (const QString &line : lines) {
            QString trimmed = line.trimmed();
            if (trimmed.isEmpty()) continue;
            if (trimmed.startsWith("#")) continue;
            if (trimmed.startsWith("First")) continue;
            if (trimmed.startsWith("Next")) continue;
            if (trimmed.startsWith("Now")) continue;
            if (trimmed.startsWith("This")) continue;
            if (trimmed.startsWith("The")) continue;
            if (trimmed.startsWith("Note")) continue;
            if (trimmed.startsWith("~")) continue;
            cleanLines.append(line);
        }
        script = cleanLines.join("\n").trimmed();
        // Napraw brakujące nawiasy klamrowe
        int openBraces = script.count('{');
        int closeBraces = script.count('}');
        while (closeBraces < openBraces) {
            script += "\n}";
            closeBraces++;
        }
        emit scriptReady(script);
        reply->deleteLater();
    });
}