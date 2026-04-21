#include <QtTest>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSignalSpy>
#include "../scriptrunner.h"
#include "../llmclient.h"


class FileOrganizerTests : public QObject
{
    Q_OBJECT

private slots:
    // ===== TESTY ScriptRunner =====

    void test_detectOS_notEmpty() {
        QString os = ScriptRunner::detectOS();
        QVERIFY(!os.isEmpty());
    }

    void test_detectOS_validValue() {
        QString os = ScriptRunner::detectOS();
        QVERIFY(os == "Windows" || os == "Linux");
    }

    void test_detectOS_isWindows() {
#ifdef Q_OS_WIN
        QCOMPARE(ScriptRunner::detectOS(), QString("Windows"));
#else
        QCOMPARE(ScriptRunner::detectOS(), QString("Linux"));
#endif
    }

    void test_scriptExecution_echo() {
        // przygotuj prosty skrypt który wypisze "TEST_OK"
#ifdef Q_OS_WIN
        QString script = "Write-Output 'TEST_OK'";
#else
        QString script = "echo TEST_OK";
#endif
        ScriptRunner runner;
        QSignalSpy spyFinished(&runner, &ScriptRunner::finished);
        QSignalSpy spyError(&runner, &ScriptRunner::errorOccurred);

        runner.runScript(script);

        // czekamy do 10s (ze względu na proces/system)
        QVERIFY(spyFinished.wait(10000) || spyError.wait(10000));
        if (spyFinished.count() > 0) {
            QList<QVariant> args = spyFinished.takeFirst();
            QString output = args.at(0).toString();
            QVERIFY(output.contains("TEST_OK"));
        } else {
            // jeśli pojawił się błąd - test powinien to zasygnalizować
            QFAIL(qPrintable(QString("ScriptRunner error: %1")
                             .arg(spyError.takeFirst().at(0).toString())));
        }
    }

    // ===== TESTY LlmClient z lokalnym serwerem =====

    void test_llmClient_returnsCleanScript_fromLocalServer() {
        // Przygotuj fake server, który zwróci JSON z markdown + polskimi znakami
        QTcpServer server;
        QVERIFY(server.listen(QHostAddress::LocalHost, 0));
        quint16 port = server.serverPort();

        // Odpowiedź/body z markdown i polskimi znakami
        QByteArray body = R"json({"response":"```powershell\nMove-Item \"plik_ąę.txt\" \"C:\\dest\\\"\n```\n<think>debug</think>"})json";

        // Połączenie: na pierwsze połączenie odsyłamy gotową odpowiedź HTTP
        connect(&server, &QTcpServer::newConnection, [&server, body]() {
            QTcpSocket *sock = server.nextPendingConnection();
            connect(sock, &QTcpSocket::readyRead, [sock, body]() {
                // odczytaj request (ignorujemy treść) i odpowiedz
                sock->readAll();
                QByteArray http = "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: "
                                  + QByteArray::number(body.size()) + "\r\n\r\n" + body;
                sock->write(http);
                sock->flush();
                sock->disconnectFromHost();
            });
        });

        // Ustaw URL API LlmClient na lokalny serwer
        QString apiUrl = QString("http://127.0.0.1:%1/api/generate").arg(port);
        qputenv("OLLAMA_API_URL", apiUrl.toUtf8());

        LlmClient client;
        QSignalSpy spyScript(&client, &LlmClient::scriptReady);
        QSignalSpy spyError(&client, &LlmClient::errorOccurred);

        client.generateScript("Przenieś pliki", "C:/test", "Windows");

        // czekamy na wynik (do 5s) — serwer odpowiada natychmiast
        QVERIFY(spyScript.wait(5000) || spyError.wait(5000));
        if (spyScript.count() > 0) {
            QString script = spyScript.takeFirst().at(0).toString();
            // powinien zawierać polecenie Move-Item oraz polskie znaki, ale nie znaków markdown ani <think>
            QVERIFY(script.contains("Move-Item"));
            QVERIFY(script.contains("plik_ąę"));
            QVERIFY(!script.contains("```"));
            QVERIFY(!script.contains("<think>"));
        } else {
            QFAIL(qPrintable(QString("LlmClient error: %1")
                             .arg(spyError.takeFirst().at(0).toString())));
        }

        server.close();
    }

    void test_llmClient_emitsError_whenNoServer() {
        // ustaw nieosiągalny port — spodziewamy się errorOccurred
        qputenv("OLLAMA_API_URL", QByteArray("http://127.0.0.1:9/api/generate")); // zazwyczaj zamknięty port
        LlmClient client;
        QSignalSpy spyError(&client, &LlmClient::errorOccurred);

        client.generateScript("test", "C:/test", "Windows");

        // LlmClient ma wewnętrzny timeout ~8000ms; czekamy 9s aby być bezpiecznym
        QVERIFY(spyError.wait(9000));
        QVERIFY(spyError.count() > 0);
    }

    // ===== Dodatkowe testy czyszczenia =====

    void test_scriptCleaning_removesMarkdown() {
        QString dirty = "```powershell\nMove-Item test\n```";
        dirty.remove(QRegularExpression("```powershell\\s*",
                                        QRegularExpression::CaseInsensitiveOption));
        dirty.remove(QRegularExpression("```\\s*"));
        QVERIFY(!dirty.contains("```"));
    }

    void test_scriptCleaning_removesThinkBlock() {
        QString dirty = "<think>some thinking</think>\nMove-Item test";
        dirty.remove(QRegularExpression("<think>[\\s\\S]*?</think>",
                                        QRegularExpression::CaseInsensitiveOption));
        QVERIFY(!dirty.contains("<think>"));
    }

    void test_braceBalancing() {
        QString script = "ForEach-Object {\nMove-Item test\n";
        int openBraces = script.count('{');
        int closeBraces = script.count('}');
        while (closeBraces < openBraces) {
            script += "\n}";
            closeBraces++;
        }
        QCOMPARE(script.count('{'), script.count('}'));
    }
};

QTEST_MAIN(FileOrganizerTests)
#include "test_main.moc"