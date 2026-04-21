#include "llmclient.h"
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QTimer>
#include <QCoreApplication>
#include <QProcess>

/**
 * @brief Konstruktor LlmClient
 */
LlmClient::LlmClient(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
}


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
                       "osobnego Get-ChildItem dla każdego rozszerzenia osobno. "
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

    // Pozwól nadpisać URL przez zmienną środowiskową
    QString apiUrl = QString::fromUtf8(qgetenv("OLLAMA_API_URL"));
    if (apiUrl.isEmpty()) apiUrl = API_URL;

    QUrl url(apiUrl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QByteArray data = QJsonDocument(json).toJson();
    QNetworkReply *reply = manager->post(request, data);

    // Timeout dla odpowiedzi sieciowej
    QTimer *netTimer = new QTimer(reply);
    netTimer->setSingleShot(true);
    netTimer->start(8000);
    connect(netTimer, &QTimer::timeout, reply, [reply, this]() {
        if (!reply->isFinished()) {
            reply->abort();
            emit errorOccurred("Timeout: brak odpowiedzi od serwera LLM (Ollama).");
        }
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        // Jeśli miał miejsce błąd sieciowy -> zgłoś i posprzątaj
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(QString("Błąd połączenia z Ollama: %1")
                               .arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        // Spróbuj sparsować JSON, a jeśli nie - użyj surowego tekstu
        QByteArray raw = reply->readAll();
        QString script;
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(raw, &parseError);
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject obj = doc.object();
            // Różne wersje Ollama mogą zwrócić pole "response" lub "text"
            if (obj.contains("response")) {
                script = obj.value("response").toString();
            } else if (obj.contains("text")) {
                script = obj.value("text").toString();
            } else {
                // fallback: całe body jako tekst
                script = QString::fromUtf8(raw);
            }
        } else {
            script = QString::fromUtf8(raw);
        }

        // Usuwanie bloków <think>...</think> (wielowierszowo)
        QRegularExpression thinkRe("<think>[\\s\\S]*?</think>",
                                  QRegularExpression::CaseInsensitiveOption);
        script.remove(thinkRe);

        // Usuń bloki markdown ```...``` wraz z opcją języka 
        QRegularExpression fenceRe("(?s)```[a-zA-Z0-9_-]*\\s*.*?```",
                                   QRegularExpression::CaseInsensitiveOption);
        script.remove(fenceRe);

        // Usuń pojedyncze otwierające znaczniki ```powershell lub ```bash
        QRegularExpression fenceOpenRe("(?i)```(?:powershell|bash)?\\s*");
        script.remove(fenceOpenRe);

        // Usuń linie opisowe — angielskie i polskie początki
        QRegularExpression nonCodeLineRe(
            "^(\\s*(#|//).*|\\s*(First|Next|Now|This|The|Note|Please|Uwaga|Najpierw|Następnie|Teraz|Proszę).*)$",
            QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption);
        // Dzielimy na linie i filtrujemy, aby zachować kolejność i puste linie usunąć
        QStringList lines = script.split(QRegularExpression("\\r?\\n"));
        QStringList cleanLines;
        for (const QString &line : lines) {
            QString trimmed = line.trimmed();
            if (trimmed.isEmpty()) continue;
            if (trimmed.startsWith('#') || trimmed.startsWith("//")) continue;
            if (nonCodeLineRe.match(line).hasMatch()) continue;
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

        if (script.isEmpty()) {
            emit errorOccurred("Otrzymano pusty skrypt od modelu LLM.");
        } else {
            emit scriptReady(script);
        }

        reply->deleteLater();
    });
}

#include "llmclient.moc"