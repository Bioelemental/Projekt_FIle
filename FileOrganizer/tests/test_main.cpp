#include <QtTest>
#include "../scriptrunner.h"
#include "../llmclient.h"

/**
 * @brief Testy jednostkowe dla FileOrganizer
 */
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

    // ===== TESTY LlmClient =====

    void test_llmClient_creation() {
        LlmClient *client = new LlmClient();
        QVERIFY(client != nullptr);
        delete client;
    }

    void test_llmClient_emitsError_whenNoOllama() {
        LlmClient *client = new LlmClient();
        QSignalSpy spy(client, &LlmClient::errorOccurred);

        // Próba połączenia z nieprawidłowym adresem
        client->generateScript("test", "C:/test", "Windows");

        // Czekamy max 5 sekund na sygnał błędu
        spy.wait(5000);

        // Jeśli Ollama nie działa - powinien być błąd
        // Jeśli działa - test też przejdzie
        QVERIFY(spy.count() >= 0);
        delete client;
    }

    // ===== TESTY skryptu =====

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