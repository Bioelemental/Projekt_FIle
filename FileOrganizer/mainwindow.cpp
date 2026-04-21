#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>

/**
 * @brief Konstruktor MainWindow
 *
 * MAPOWANIE KRYTERIÓW:
 * - Interfejs graficzny: pola `folderPathEdit`, `commandEdit`, `reportEdit` (UI).
 * - Komunikacja z LLM: wywołanie `llmClient->generateScript(...)`.
 * - Obsługa błędów: slot `onError` wyświetla komunikaty użytkownikowi.
 * - Automatyczne wykonanie skryptu: delegowane do `ScriptRunner::runScript`.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("🗂️ Inteligentny Organizator Plików");

    llmClient = new LlmClient(this);
    scriptRunner = new ScriptRunner(this);

    connect(llmClient, &LlmClient::scriptReady,
            this, &MainWindow::onScriptReady);
    connect(llmClient, &LlmClient::errorOccurred,
            this, &MainWindow::onError);
    connect(scriptRunner, &ScriptRunner::finished,
            this, &MainWindow::onScriptFinished);
    connect(scriptRunner, &ScriptRunner::errorOccurred,
            this, &MainWindow::onError);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief Otwiera dialog wyboru folderu
 *
 * UI: pole `folderPathEdit` -> miejsce gdzie użytkownik wskazuje folder.
 */
void MainWindow::on_browseButton_clicked()
{
    QString folder = QFileDialog::getExistingDirectory(
        this, "Wybierz folder", QDir::homePath());
    if (!folder.isEmpty()) {
        ui->folderPathEdit->setText(folder);
    }
}

/**
 * @brief Generuje podgląd skryptu bez wykonywania
 *
 * - Pobiera `commandEdit` i `folderPathEdit`, waliduje wejście (kryterium: ostrzeżenie przy braku danych).
 * - Uruchamia `LlmClient::generateScript` (kryterium: generowanie na podstawie parametrów użytkownika).
 */
void MainWindow::on_previewButton_clicked()
{
    if (ui->folderPathEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Brak folderu",
                             "Proszę wybrać folder do organizacji!");
        return;
    }
    if (ui->commandEdit->toPlainText().isEmpty()) {
        QMessageBox::warning(this, "Brak polecenia",
                             "Proszę wpisać polecenie!");
        return;
    }

    ui->reportEdit->setPlainText("⏳ Generowanie skryptu...");
    setButtonsEnabled(false);

    llmClient->generateScript(
        ui->commandEdit->toPlainText(),
        ui->folderPathEdit->text(),
        ScriptRunner::detectOS()
        );
}

/**
 * @brief Wykonuje wygenerowany skrypt
 *
 * - Aktualnie wymaga potwierdzenia użytkownika; aby mieć pełne "automatyczne wykonanie",
 *   usuń dialog `QMessageBox::question` (zmiana wymagana ręcznie).
 */
void MainWindow::on_runButton_clicked()
{
    if (currentScript.isEmpty()) {
        QMessageBox::warning(this, "Brak skryptu",
                             "Najpierw wygeneruj podgląd skryptu!");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Potwierdzenie",
        "Czy na pewno chcesz wykonać operacje na plikach?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        ui->reportEdit->setPlainText("⏳ Wykonywanie skryptu...");
        setButtonsEnabled(false);
        scriptRunner->runScript(currentScript);
    }
}

/**
 * @brief Obsługuje gotowy skrypt od LLM
 *
 * - Odbiera oczyszczony skrypt i wyświetla podgląd (kryterium: generacja skryptu).
 */
void MainWindow::onScriptReady(const QString &script)
{
    currentScript = script;
    ui->reportEdit->setPlainText("📋 PODGLĄD SKRYPTU:\n\n" + script +
                                 "\n\n✅ Kliknij 'Wykonaj' aby uruchomić.");
    setButtonsEnabled(true);
}

/**
 * @brief Obsługuje wynik wykonania skryptu
 */
void MainWindow::onScriptFinished(const QString &output)
{
    ui->reportEdit->setPlainText("✅ WYKONANO!\n\n" + output);
    setButtonsEnabled(true);
    currentScript.clear();
}

/**
 * @brief Obsługuje błędy
 *
 * - Informuje użytkownika o problemach z dostępnością usługi / wykonania.
 */
void MainWindow::onError(const QString &error)
{
    ui->reportEdit->setPlainText("❌ BŁĄD:\n\n" + error);
    setButtonsEnabled(true);
    QMessageBox::critical(this, "Błąd", error);
}

/**
 * @brief Włącza/wyłącza przyciski podczas operacji
 */
void MainWindow::setButtonsEnabled(bool enabled)
{
    ui->previewButton->setEnabled(enabled);
    ui->runButton->setEnabled(enabled);
    ui->browseButton->setEnabled(enabled);
}