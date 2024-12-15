#pragma once

#include <iostream>
#include <memory>
#include <random>
#include <vector>
#include <string>
#include <map>
#include "../GameAnalysis/GameAnalysis.h"
#include "../GameBot/GameBot.h"


static std::vector<std::pair<std::string, std::vector<std::string>>> gameParameters{
    
    // ��������� ���� (�������� � ����� ��������) 
    // �������� ������������ ��� �������� �� �������� ���������� ���� � ��������� ����������� 
    // ���� � ������ ������������ ��� ��������� �������� ������� ��� ��������������� ����

    { "mushroom_glade_5x6x6",
        {
            "D:\\GameData\\mg_5x6x6\\1\\1_mg_5x6x6_firstPlayerTree30_35.bin"
        }
    },
    { "mushroom_glade_5x6x6",
        {
            "D:\\GameData\\mg_5x6x6\\1\\1_mg_5x6x6_equalTree0_20.bin"
        }
    },
    { "mushroom_glade_5x6x6",
        {
            "D:\\GameData\\mg_5x6x6\\1\\1_mg_5x6x6_secondPlayerTree25_30.bin"
        }
    },
    { "mushroom_glade_5x6x6",
        {
            "D:\\GameData\\mg_5x6x6\\1\\1_mg_5x6x6_secondPlayerTree30_35.bin"
        }
    },
};

class PlayingGame {
private:
    
    // ������� �������
    int currentLevel; 
    // ������� ��� ����
    std::string gameName{""};  
    // ��� ����� ��� �������� ������ ���������
    std::string stateTreeFile{""}; 
    // ��������� �� ����
    std::unique_ptr<GameBot> bot{nullptr}; 

    std::unique_ptr<StateTree> loadStateTree(const std::string& filename) {

        // �� ���������� �������� ����� ����������� 
        // ��������������� ������ ������� ����

        auto tree = std::make_unique<StateTree>();
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            throw std::runtime_error("Failed to load StateTree from file: " 
                + filename);
        }
        tree->loadFromBinary(file);
        file.close();

        return std::move(tree);
    }

    int selectRandomKey(const std::unique_ptr<StateTree>& infoState) {

        // � ����������� ������ ������� ����� ����� 
        // �������� ���������� ���� �� ��������� ��������� �������� ����

        if (!infoState || infoState->states.empty()) {
            throw std::runtime_error("StateTree is empty or null");
        }

        auto it = infoState->states.begin();
        std::advance(it, rand() % infoState->states.size());
        return it->first;
    }

    bool playGame(int initialKey) {

        // ������������� �������� ��������
        // ������� ������������ ��������� ����� 
        // ��������� ��������� �������� (initialKey)
        // ����� ���� ���� ����� ������� � �����

        auto game = open_spiel::LoadGame(gameName);
        auto state = game->NewInitialState();

        // �������� ��������� ���������
        state->ApplyAction(initialKey);

        while (!state->IsTerminal()) {
            auto player = state->CurrentPlayer();
            std::cout << "player " << player << std::endl;
            std::cout << "State's history: " 
                << state->InformationStateString(player) << std::endl;

            std::cout << "State: " << std::endl 
                << state->ToString() << std::endl;

            if (player == 0) {
                // ��� ������
                std::vector<open_spiel::Action> actions = 
                    state->LegalActions(player);
                std::cout << "Legal Actions: [";
                for (auto act : actions) {
                    std::cout << act << ", ";
                }
                std::cout << "]\n\n";

                int action;
                std::cout << "choose your action: ";
                std::cin >> action;

                state->ApplyAction(action);
                bot->setCurrentState(action);
            }
            else {
                // ��� ����
                int botAction = bot->makeMove();
                state->ApplyAction(botAction);
                bot->setCurrentState(botAction);
            }
        }

        // ��������� ����
        std::cout << "State: " << std::endl << state->ToString() << std::endl;
        auto results = state->Returns();
        for (auto award : results) {
            std::cout << award << " ";
        }

        std::cout << std::endl;
        if (results[0] == 1 && results[1] == -1) {
            std::cout << "You Win!" << std::endl;
            return true;
        }
        else if (results[0] == -1 && results[1] == 1) {
            std::cout << "You Lose!" << std::endl;
        }
        else {
            std::cout << "Friendship Won!" << std::endl;
        }
        return results[0] == 0 || results[0] == 1;
    }



    bool nextLevel() {

        // ���������� gameParameters 
        // ��� �������� �� ����� ������� 
        // � ������ ����� ������� �������� 
        // �������� ���� ��� ���������� � �������� � ����, 
        // �� �������� ����� �������� ������ ������� ����

        if (currentLevel >= gameParameters.size()) {
            return false;
        }

        gameName = gameParameters[currentLevel].first;
        const auto& files = gameParameters[currentLevel].second;

        // �������� ��������� ���� �� ���������
        stateTreeFile = files[rand() % files.size()];
        ++currentLevel;
        return true;
    }
public:
    PlayingGame() : 
        currentLevel{ 0 }, 
        gameName{ " " }, 
        stateTreeFile{ " " }, 
        bot{ nullptr } {};

    void run() {

        // ������� �������������� ���� 
        // � ������ ���������� ������� ��������� ����, 
        // � ����������� � ������� �� ����� ��������������� ��� ���� ���� 
        // � ������ �������� ������������� ���� ���� 
        // � �� ����� ��� �������� ����������� ���������� �������
        // � �������� ��������� ���������� ���������� ������� �������, 
        // ������� ��������� ����� ��������� ���� ����

        // ����� ��������� ����
        std::cout << "Choose bot difficulty (1-4): ";

        int difficulty;
        std::cin >> difficulty;

        bool isNextLevel = true;

        int completedLevels = 0;


        while (true) {
            if (isNextLevel) { // ������� �� ����� �������
                if (!nextLevel()) {
                    std::cout << "All levels completed!" << std::endl;
                    break;
                }

                std::cout << "Level " << currentLevel << std::endl;
            }

            try {
                //�������� ���� ��� ������� ����
                auto infoState = loadStateTree(stateTreeFile);
                int initialKey = selectRandomKey(infoState);



                infoState = std::move(infoState->states[initialKey]);

                // ������� ���� ������� ������ � ����������� ������� �������
                switch (difficulty) {
                case 1:
                    bot = std::make_unique<EasyBot>(std::move(infoState));
                    break;
                case 2:
                    bot = std::make_unique<MediumBot>(std::move(infoState));
                    break;
                case 3:
                    bot = std::make_unique<HardBot>(std::move(infoState));
                    break;
                case 4:
                    bot = std::make_unique<ExpertBot>(std::move(infoState));
                    break;
                default:
                    throw std::runtime_error("Invalid difficulty level");
                }

                // ������� �������
                isNextLevel = playGame(initialKey);

                if (isNextLevel) {
                    ++completedLevels;
                }
            }
            catch (const std::exception& ex) {
                std::cerr << "Error: " << ex.what() << std::endl;
                break;
            }

            std::cout << "Do you want to exit? (yes/no): ";
            std::string choice;
            std::cin >> choice;
            if (choice == "yes" || choice == "y") {
                break;
            }
        }

        std::cout << "You completed " << completedLevels << " levels!" << std::endl;
    }
};
