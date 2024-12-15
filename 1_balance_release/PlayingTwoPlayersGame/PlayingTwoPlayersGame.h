#pragma once

#include <iostream>
#include <memory>
#include <random>
#include <vector>
#include <string>
#include <map>
#include "../GameAnalysis/GameAnalysis.h"


static std::vector<std::pair<std::string, std::vector<std::string>>> twoPlayersGameParameters{
    
    // Параметры игры (название и файлы деревьев) 
    // Название используется для создания по названию конкретной игры с заданными параметрами 
    // Пути к файлам используются для подгрузки деревьев исходов для соответствующей игры

    { "mushroom_glade_3x4x4",
        {
            "D:\\GameData\\mg_3x4x4\\1_mg_3x4x4_equalTree0_20.bin"
        }
    },
    { "mushroom_glade_3x4x6",
        {
            "D:\\GameData\\mg_3x4x6\\1_mg_3x4x6_equalTree0_20.bin"
        }
    },
    { "mushroom_glade_3x6x6",
        {
            "D:\\GameData\\mg_3x6x6\\1_mg_3x6x6_equalTree0_20.bin"
        }
    },
    { "mushroom_glade_5x4x6",
        {
            "D:\\GameData\\mg_5x4x6\\1\\1_mg_5x4x6_equalTree0_20.bin",
            "D:\\GameData\\mg_5x4x6\\2\\2_mg_5x4x6_equalTree0_20.bin",
        }
    },
    { "mushroom_glade_5x6x6",
        {
            "D:\\GameData\\mg_5x6x6\\1\\1_mg_5x6x6_equalTree0_20.bin",
            "D:\\GameData\\mg_5x6x6\\2\\2_mg_5x6x6_equalTree0_20.bin",
            "D:\\GameData\\mg_5x6x6\\3\\3_mg_5x6x6_equalTree0_20.bin",
            "D:\\GameData\\mg_5x6x6\\4\\4_mg_5x6x6_equalTree0_20.bin",
            "D:\\GameData\\mg_5x6x6\\5\\5_mg_5x6x6_equalTree0_20.bin",
            "D:\\GameData\\mg_5x6x6\\10\\10_mg_5x6x6_equalTree0_20.bin",
            "D:\\GameData\\mg_5x6x6\\15\\15_mg_5x6x6_equalTree0_20.bin",
            "D:\\GameData\\mg_5x6x6\\25\\25_mg_5x6x6_equalTree0_20.bin",
        }
    },
};

class PlayingTwoPlayersGame {
private:
    
    int currentLevel{0}; // Текущий уровень
    std::string gameName{""};  // Текущее имя игры
    std::string stateTreeFile{""}; // Имя файла для загрузки дерева состояний



    std::unique_ptr<StateTree> loadStateTree(const std::string& filename) {
        
        // По выбранному названию файла загружается 
        // соответственное дерево исходов игры
        
        auto tree = std::make_unique<StateTree>();
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to load StateTree from file: " + filename);
        }
        tree->loadFromBinary(file);
        file.close();

        return std::move(tree);
    }

    int selectRandomKey(const std::unique_ptr<StateTree>& infoState) {

        // В загруженном дереве исходов перед игрой 
        // рандомно выбирается одно из начальных состояний игрового поля

        if (!infoState || infoState->states.empty()) {
            throw std::runtime_error("StateTree is empty or null");
        }

        auto it = infoState->states.begin();
        std::advance(it, rand() % infoState->states.size());
        return it->first;
    }

    int playGame(int initialKey) {

        // Осуществление игрового процесса
        // Сначала записывается выбранное ранее 
        // начальное состояние игрового (initialKey)
        // Далее идет игра между двумя иггроками

        auto game = open_spiel::LoadGame(gameName);
        auto state = game->NewInitialState();

        // Выбираем начальное состояние
        state->ApplyAction(initialKey);

        while (!state->IsTerminal()) {
            auto player = state->CurrentPlayer();
            std::cout << "player " << player << std::endl;
            std::cout << "State's history: " << state->InformationStateString(player) << std::endl;

            std::cout << "State: " << std::endl << state->ToString() << std::endl;

            // Ход игрока
            std::vector<open_spiel::Action> actions = state->LegalActions(player);
            std::cout << "Legal Actions: [";
            for (auto act : actions) {
                std::cout << act << ", ";
            }
            std::cout << "]\n\n";

            int action;
            std::cout << "choose your action: ";
            std::cin >> action;

            state->ApplyAction(action);
        }

        // Завершаем игру
        std::cout << "State: " << std::endl << state->ToString() << std::endl;
        auto results = state->Returns();
        for (auto award : results) {
            std::cout << award << " ";
        }

        std::cout << std::endl;
        if (results[0] == 1 && results[1] == -1) {
            std::cout << "Player 1 Win!" << std::endl;
            return true;
        }
        else if (results[0] == -1 && results[1] == 1) {
            std::cout << "Player 2 Win!" << std::endl;
        }
        else {
            std::cout << "Friendship Won!" << std::endl;
        }
        return results[0];
    }



    bool nextLevel() {

        // Использует twoPlayersGameParameters 
        // для перехода на новый уровень 
        // С каждым новым уровнем меняется 
        // название игры для дальнейшей её загрузки и файл, 
        // из которого будет записано дерево исходов игры

        if (currentLevel >= twoPlayersGameParameters.size()) {
            return false;
        }

        gameName = twoPlayersGameParameters[currentLevel].first;
        const auto& files = twoPlayersGameParameters[currentLevel].second;

        // Выбираем случайный файл из доступных
        stateTreeFile = files[rand() % files.size()];
        ++currentLevel;
        return true;
    }
public:
    PlayingTwoPlayersGame() : currentLevel{ 0 }, gameName{ " " }, stateTreeFile{ " " } {};

    void run() {

        // Процесс многоуровневой игры 
        // В каждой итерации проигрывается одна игра 
        // и по итогу игрокам предлагается повысить уровень
        // В процессе игры у участников считаются победные очки, 
        // которые выводятся после окончания всей игры

        bool isNextLevel = true;

        int player1Points = 0, player2Points = 0;


        while (true) {
            if (isNextLevel) {
                if (!nextLevel()) {
                    std::cout << "All levels completed!" << std::endl;
                    break;
                }

                std::cout << "Level " << currentLevel << std::endl;
            }

            try {
                // Загрузка дерева состояний по выбранному файлу
                auto infoState = loadStateTree(stateTreeFile);
                int initialKey = selectRandomKey(infoState);
                
                // Игровой процесс
                int points = playGame(initialKey);

                if (points > 0) {
                    player1Points++;
                } 
                else if (points < 0) {
                    player2Points++;
                }
                else {
                    player1Points++;
                    player2Points++;
                }
            }
            catch (const std::exception& ex) {
                std::cerr << "Error: " << ex.what() << std::endl;
                break;
            }

            // Выбор перехода на следующий уровень 
            // или реванша на текущем уровне
            std::cout << "Do you want a rematch? (yes/no): ";
            std::string choice;
            std::cin >> choice;
            if (choice == "yes" || choice == "y") {
                isNextLevel = false;
            }
            else {
                isNextLevel = true;
            }

            std::cout << "Do you want to exit? (yes/no): ";
            std::cin >> choice;
            if (choice == "yes" || choice == "y") {
                break;
            }
        }

        std::cout << "Total points earned: Player 1 - " << player1Points << "; Player 2 - " << player2Points << std::endl;
    }
};
