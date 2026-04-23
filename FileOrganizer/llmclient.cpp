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
    // ----------------------------------------------------------
    // Definicja system prompt (kryterium: Definicja system prompt)
    // ----------------------------------------------------------
    QString systemPrompt;
    if (os == "Windows") {
        systemPrompt = "Jestes generatorem skryptow PowerShell. "
                       "Generuj TYLKO czysty kod PowerShell. "
                       "Uzywaj TYLKO Move-Item do przenoszenia plikow. "
                       "NIGDY nie uzywaj Remove-Item. "
                       "Aby przeniesc plik DO folderu uzyj: Move-Item -Path $file -Destination $folderPath gdzie $folderPath to FOLDER nie pelna sciezka pliku. "
                       "Aby filtrowac wiele rozszerzen ZAWSZE uzywaj osobnego Get-ChildItem dla kazdego rozszerzenia osobno. "
                       "Nie usuwaj rozszerzen z plikow"
                       "Sprawdzaj czy folder istnieje przed przeniesieniem plikow jezeli nie to stworz go w $folderPath"
                       "Aby utworzyc folder: New-Item -ItemType Directory -Force -Path $folder. "
                       "Po kazdym przeniesieniu: Write-Host 'MOVED: plik -> cel'. "
                       "Nie dodawaj opisow ani komentarzy.";
                       "ZAKAZANE: -Filter '*.jpg, *.png' (przecinek w filtrze jest BLEDNY)";


    } else {
        systemPrompt = "Jestes generatorem skryptow Bash. "
                       "Generuj TYLKO czysty kod Bash bez opisow, bez komentarzy. "
                       "Uzywaj TYLKO mv do przenoszenia plikow. "
                       "NIGDY nie uzywaj rm -rf. "
                       "Aby utworzyc folder uzyj: mkdir -p. "
                       "Po kazdym przeniesieniu uzyj: echo 'MOVED: plik -> cel'. "
                       "NIE pisz slowa MOVED bez echo. "
                       "Nie dodawaj zadnych opisow ani komentarzy poza kodem.";
                       "- Dla folderu A-Z uzyj: for letter in {A..Z}; do mkdir -p \"$sourcePath/$letter\"; done\n";
    }

    QString fullPrompt = QString("Folder: %1\nPolecenie: %2")
                             .arg(folderPath, userPrompt);

    QJsonObject json;
    json["model"] = MODEL;
    json["system"] = systemPrompt;
    json["prompt"] = fullPrompt;
    json["stream"] = false;
    json["options"] = QJsonObject{{"num_predict", 1024}};

    QString apiUrl = QString::fromUtf8(qgetenv("OLLAMA_API_URL"));
    if (apiUrl.isEmpty()) apiUrl = API_URL;

    QUrl url(apiUrl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=utf-8");

    QByteArray data = QJsonDocument(json).toJson();
    QNetworkReply *reply = manager->post(request, data);

    /*QTimer *netTimer = new QTimer(reply);
    netTimer->setSingleShot(true);
    netTimer->start(180000);
    connect(netTimer, &QTimer::timeout, reply, [reply, this]() {
        if (!reply->isFinished()) {
            reply->abort();
            emit errorOccurred("Timeout: brak odpowiedzi od serwera LLM (Ollama) po 180 sekundach.");
        }
    });*/

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(QString("Blad polaczenia z Ollama: %1")
                               .arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        // ----------------------------------------------------------
        // Parsowanie odpowiedzi i czyszczenie skryptu
        // ----------------------------------------------------------
        QByteArray raw = reply->readAll();
        QString script;
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(raw, &parseError);
        
        if (!doc.isNull() && doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains("response")) {
                script = obj.value("response").toString();
            } else if (obj.contains("text")) {
                script = obj.value("text").toString();
            } else {
                script = QString::fromUtf8(raw);
            }
        } else {
            script = QString::fromUtf8(raw);
        }

        // ZMIANA: Bezpieczniejsze czyszczenie
        // 1. Usuń bloki <think>...</think>
        script.remove(QRegularExpression("<think>[\\s\\S]*?</think>",
                                  QRegularExpression::CaseInsensitiveOption));

        // 2. Usuń bloki markdown ```powershell...``` lub ```bash...```
        // Wzorzec: ``` opcjonalny język (powershell/bash) dowolny tekst ```
        script.remove(QRegularExpression("```(?:powershell|bash)?\\s*\\n",
                                        QRegularExpression::CaseInsensitiveOption));
        script.remove(QRegularExpression("\\n?```\\s*$",
                                        QRegularExpression::CaseInsensitiveOption));
        script.remove(QRegularExpression("^```\\s*",
                                        QRegularExpression::CaseInsensitiveOption));

        // 3. Usuń tylko linie CAŁKOWICIE opisowe (nie dotykaj kodu)
        QStringList lines = script.split(QRegularExpression("\\r?\\n"));
        QStringList cleanLines;
        
        for (const QString &line : lines) {
            QString trimmed = line.trimmed();
            
            // Pomiń puste linie
            if (trimmed.isEmpty()) {
                continue;
            }
            
            // Pomiń tylko linie komentarzy
            if (trimmed.startsWith('#') && !trimmed.contains("MOVED:")) {
                continue;
            }
            
            // Pomiń linie zaczynające się od opisowych słów (TYLKO jeśli nie ma znaku $ lub Move-Item)
            bool isDescriptiveLine = false;
            QStringList descriptiveStarts = {"First", "Next", "Now", "This", "The", "Note", "Please", 
                                              "Uwaga", "Najpierw", "Nastepnie", "Teraz", "Prosze",
                                              "Here", "To", "You", "We"};
            
            for (const QString &desc : descriptiveStarts) {
                if (trimmed.startsWith(desc, Qt::CaseInsensitive) && 
                    !trimmed.contains("$") && 
                    !trimmed.contains("Move-Item") &&
                    !trimmed.contains("Get-ChildItem") &&
                    !trimmed.contains("New-Item") &&
                    !trimmed.contains("mkdir") &&
                    !trimmed.contains("mv") &&
                    !trimmed.contains("echo")) {
                    isDescriptiveLine = true;
                    break;
                }
            }
            
            if (isDescriptiveLine) {
                continue;
            }
            
            // Dodaj linię do oczyszczonego skryptu
            cleanLines.append(line); // Zachowaj oryginalne wcięcia!
        }
        
        script = cleanLines.join("\n").trimmed();

        // 4. Napraw brakujące nawiasy klamrowe
        int openBraces = script.count('{');
        int closeBraces = script.count('}');
        while (closeBraces < openBraces) {
            script += "\n}";
            closeBraces++;
        }

        // 5. Sprawdź czy skrypt nie jest pusty
        if (script.isEmpty() || script.length() < 10) {
            emit errorOccurred(QString("Otrzymano pusty lub zbyt krotki skrypt od modelu LLM.\n\nSurowa odpowiedz:\n%1")
                              .arg(QString::fromUtf8(raw).left(500)));
        } else {
            emit scriptReady(script);
        }

        reply->deleteLater();
    });
}

#include "llmclient.moc"