# 🗂️ Inteligentny Organizator Plików

Aplikacja do automatycznego organizowania plików przy użyciu lokalnego modelu językowego.

## Opis
Program pozwala użytkownikowi wskazać folder i wydać polecenie w języku naturalnym,
np. "Przenieś pliki PDF do folderu Dokumenty". Aplikacja generuje skrypt PowerShell
(Windows) lub Bash (Linux) i wykonuje go automatycznie.

## Wymagania
- Qt 6.x
- Ollama (lokalny model językowy)
- Model: SpeakLeash/bielik-11b-v3.0-instruct:Q4_K_M
- Windows 10/11 lub Linux

## Instalacja i uruchomienie

### 1. Instalacja Ollama
Pobierz z: https://ollama.com/download/windows

### 2. Pobierz model Bielik
ollama run SpeakLeash/bielik-11b-v3.0-instruct:Q4_K_M

### 3. Uruchom Ollama w tle
ollama serve

### 4. Zbuduj projekt w Qt Creator
- Otwórz CMakeLists.txt w Qt Creator
- Wybierz kit: Desktop Qt 6.x MinGW 64-bit
- Kliknij Buduj (Ctrl+B)
- Uruchom (Ctrl+R)

## Uruchomienie testów
W Qt Creator: Edycja → FileOrganizerTests → Uruchamianie

## Jak używać
1. Kliknij "Wybierz folder" i wskaż folder do organizacji
2. Wpisz polecenie w języku naturalnym
3. Kliknij "Podgląd skryptu" aby zobaczyć wygenerowany skrypt
4. Kliknij "Wykonaj" aby uruchomić organizację plików

## Struktura projektu
- llmclient.h/cpp - komunikacja z Ollama REST API
- scriptrunner.h/cpp - uruchamianie skryptów systemowych
- mainwindow.h/cpp - główne okno aplikacji

## Autor
Sebastian Kawiak - Języki Programowania Obiektowego 2025/2026