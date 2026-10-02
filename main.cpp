#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <utility>
#include <iomanip>
#include <fstream>

// Для корректного вывода кириллицы в консоль Windows
#include <windows.h>

using namespace std;

int main() {
    // Устанавливаем UTF-8 для корректного отображения кириллицы в консоли
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // --- НАСТРОЙКИ ---
    bool twoRounds = true; 
    bool shuffleRounds = true; 
    // -----------------

    const string TEAMS_FILE = "teams.txt";
    const string MARKER = "Пишите сразу под этой линией"; // Маркер, после которого идут команды

    // 1. Проверяем, существует ли файл teams.txt. Если нет — создаём с шаблоном.
    ifstream checkFile(TEAMS_FILE);
    if (!checkFile.good()) {
        ofstream templateFile(TEAMS_FILE);
        if (templateFile.is_open()) {
            templateFile << "========   Пояснения   ========\n";
            templateFile << "Напишите список команд. По одной на каждой строчке.\n";
            templateFile << "Это должно выглядить примерно так:\n";
            templateFile << "Команда 1 (Город)\n";
            templateFile << "Команда 2 (Город)\n";
            templateFile << "Команда 3 (Город)\n";
            templateFile << "===== Пишите сразу под этой линией =====\n";
            //templateFile << "========================================\n";
            templateFile.close();
            cout << "Файл '" << TEAMS_FILE << "' не найден.\n";
            cout << "Я создал для вас шаблон. Заполните его командами и запустите программу снова.\n";
            cout << "Нажмите Enter для выхода...";
            cin.get();
            return 0;
        }
    }
    checkFile.close();

    // 2. Читаем файл и ищем маркер
    ifstream fin(TEAMS_FILE);
    if (!fin.is_open()) {
        cerr << "Ошибка открытия файла " << TEAMS_FILE << endl;
        return 1;
    }

    vector<string> teams;
    string line;
    bool markerFound = false;

    while (getline(fin, line)) {
        // Если маркер ещё не найден — ищем его
        if (!markerFound) {
            if (line.find(MARKER) != string::npos) {
                markerFound = true; // Нашли! Теперь всё, что ниже — команды.
            }
            continue; // Пропускаем всё, что до маркера (пояснения)
        }

        // Если маркер уже найден — обрабатываем строки как команды
        // Убираем возможные пробелы в начале и конце строки
        size_t start = line.find_first_not_of(" \t\r\n");
        size_t end = line.find_last_not_of(" \t\r\n");
        
        if (start != string::npos) {
            string teamName = line.substr(start, end - start + 1);
            if (!teamName.empty()) {
                teams.push_back(teamName);
            }
        }
    }
    fin.close();

    if (!markerFound) {
        cerr << "Ошибка: в файле '" << TEAMS_FILE << "' не найден маркер '" << MARKER << "'.\n";
        cerr << "Проверьте, что строка '===== Пишите сразу под этой линией =====' на месте.\n";
        return 1;
    }

    if (teams.size() < 2) {
        cout << "Недостаточно команд для составления расписания (найдено: " << teams.size() << ")." << endl;
        cout << "Добавьте минимум 2 команды в файл '" << TEAMS_FILE << "' ниже маркера." << endl;
        return 1;
    }

    cout << "Загружено команд: " << teams.size() << endl;

    // Если количество команд нечетное, добавляем "пустую" команду
    bool hasBye = false;
    if (teams.size() % 2 != 0) {
        teams.push_back("---"); 
        hasBye = true;
    }

    size_t n = teams.size();
    size_t numRounds = n - 1;
    size_t matchesPerRound = n / 2;

    // Вектор для хранения всех туров первого круга. 
    // Пара хранит: <Хозяин, Гость>
    vector<vector<pair<string, string>>> firstCircleRounds;

    // Счётчик домашних матчей для каждой команды (для балансировки первого круга)
    map<string, int> homeCount;
    for (const auto& t : teams) {
        if (t != "---") homeCount[t] = 0;
    }

    vector<string> rotatedTeams = teams;

    // --- ГЕНЕРАЦИЯ ПЕРВОГО КРУГА ---
    for (size_t round = 0; round < numRounds; ++round) {
        vector<pair<string, string>> currentRoundMatches;
        
        for (size_t i = 0; i < matchesPerRound; ++i) {
            string team1 = rotatedTeams[i];
            string team2 = rotatedTeams[n - 1 - i];

            // Пропускаем матчи с "пустышкой"
            if (team1 == "---" || team2 == "---") {
                continue;
            }

           // Назначаем хозяина: у кого меньше домашних матчей — тот дома
            if (homeCount[team1] <= homeCount[team2]) {
                currentRoundMatches.emplace_back(team1, team2);
                homeCount[team1]++;
            } else {
                currentRoundMatches.emplace_back(team2, team1);
                homeCount[team2]++;
            }
        }
        
        firstCircleRounds.push_back(currentRoundMatches);

        // Вращаем массив команд (кроме первого элемента)
        rotate(rotatedTeams.begin() + 1, rotatedTeams.begin() + 2, rotatedTeams.end());
    }

    // Общий вектор для всех туров
    vector<vector<pair<string, string>>> allRounds;

    // Добавляем первый круг
    allRounds = firstCircleRounds;

    // --- ГЕНЕРАЦИЯ ВТОРОГО КРУГА (ЕСЛИ НУЖЕН) ---
    if (twoRounds) {
        for (const auto& roundMatches : firstCircleRounds) {
            vector<pair<string, string>> secondCircleRound;
            // Для каждого матча меняем хозяина и гостя местами
            for (const auto& match : roundMatches) {
                // match.first был хозяином, match.second гостем.
                // Теперь match.second станет хозяином.
                secondCircleRound.emplace_back(match.second, match.first);
            }
            allRounds.push_back(secondCircleRound);
        }
    }

    // --- ПЕРЕМЕШИВАНИЕ ---
    random_device rd;
    mt19937 g(rd());

    if (shuffleRounds) {
        // Перемешиваем туры ТОЛЬКО внутри первого круга (индексы 0 .. numRounds-1)
        shuffle(allRounds.begin(), allRounds.begin() + numRounds, g);

        // Если есть второй круг, перемешиваем его отдельно (индексы numRounds .. end)
        if (twoRounds) {
            shuffle(allRounds.begin() + numRounds, allRounds.end(), g);
        }
    }

    // Перемешиваем порядок матчей внутри каждого тура (независимо от круга)
    for (auto& roundMatches : allRounds) {
        shuffle(roundMatches.begin(), roundMatches.end(), g);
    }

    // 6. Выводим данные на экран
    string modeStr = twoRounds ? "в два круга" : "в один круг";
    cout << "Расписание игр (" << modeStr << "):" << endl;
    cout << "===============================" << endl;
    
    int globalMatchNumber = 1;

    for (size_t i = 0; i < allRounds.size(); ++i) {
        // Определяем, к какому кругу относится текущий тур для красивого вывода
        string circleLabel = "";
        if (twoRounds) {
            if (i < numRounds) circleLabel = " [1-й круг]";
            else circleLabel = " [2-й круг]";
        }

        cout << "\nТур " << (i + 1) << circleLabel << ":" << endl;
        cout << "-------------------------------" << endl;
        
        for (const auto& match : allRounds[i]) {
            // match.first - хозяин, match.second - гость
            cout << "  Игра #" << globalMatchNumber << ": " 
                 << match.first << " (дома)  -  " << match.second << " (в гостях)" << endl;
            globalMatchNumber++;
        }
    }

    // 7. Выводим данные в текстовый файл
    ofstream fout("schedule.txt");
    if (fout.is_open()) {
        fout << "Расписание игр (" << modeStr << "):" << endl;
        fout << "===============================" << endl;
        
        int global_Match_Number = 1;

        for (size_t i = 0; i < allRounds.size(); ++i) {
             // Определяем, к какому кругу относится текущий тур для файла
            string circleLabel = "";
            if (twoRounds) {
                if (i < numRounds) circleLabel = " [1-й круг]";
                else circleLabel = " [2-й круг]";
            }

            fout << "\nТур " << (i + 1) << circleLabel << ":" << endl;
            fout << "-------------------------------" << endl;
            
            for (const auto& match : allRounds[i]) {
                // match.first - хозяин, match.second - гость
                fout << "  Игра #" << global_Match_Number << ": " 
                     << match.first << " (дома)  -  " << match.second << " (в гостях)" << endl;
                global_Match_Number++;
            }
        }
        fout.close();
        cout << "\nРасписание сохранено в файл schedule.txt" << endl;
    } else {
        cerr << "Ошибка открытия файла для записи!" << endl;
    }

    return 0;

}
