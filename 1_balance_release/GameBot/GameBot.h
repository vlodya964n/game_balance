#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <vector>
#include <algorithm>
#include <random>
#include <stdexcept>
#include "../GameAnalysis/GameAnalysis.h"


class GameBot {
protected:
    // Текущее состояние игры в дереве исходов
    std::unique_ptr<StateTree> currentState; 
    // Уровень сложности бота
    int difficultyLevel; 

public:
    GameBot(std::unique_ptr<StateTree> state, int difficulty)
        : currentState(std::move(state)), difficultyLevel(difficulty) {}

    virtual ~GameBot() = default;

    // Выбор действия, используя информацию из дерева исходов
    virtual int makeMove() = 0; 

    // Перевод дерева исходов в новое состояние 
    void setCurrentState(int key); 
};

// Бот с легким уровнем сложности
class EasyBot : public GameBot {
public:
    EasyBot(std::unique_ptr<StateTree> state) 
        : GameBot(std::move(state), 1) {}

    int makeMove() override;
};

// Бот с уровнем ниже среднего
class MediumBot : public GameBot {
public:
    MediumBot(std::unique_ptr<StateTree> state) 
        : GameBot(std::move(state), 2) {}

    int makeMove() override;
};

// Бот с уровнем выше среднего
class HardBot : public GameBot {
public:
    HardBot(std::unique_ptr<StateTree> state) 
        : GameBot(std::move(state), 3) {}

    int makeMove() override;
};

// Сложный бот
class ExpertBot : public GameBot {
public:
    ExpertBot(std::unique_ptr<StateTree> state) 
        : GameBot(std::move(state), 4) {}

    int makeMove() override;
};

