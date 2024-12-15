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
    // ������� ��������� ���� � ������ �������
    std::unique_ptr<StateTree> currentState; 
    // ������� ��������� ����
    int difficultyLevel; 

public:
    GameBot(std::unique_ptr<StateTree> state, int difficulty)
        : currentState(std::move(state)), difficultyLevel(difficulty) {}

    virtual ~GameBot() = default;

    // ����� ��������, ��������� ���������� �� ������ �������
    virtual int makeMove() = 0; 

    // ������� ������ ������� � ����� ��������� 
    void setCurrentState(int key); 
};

// ��� � ������ ������� ���������
class EasyBot : public GameBot {
public:
    EasyBot(std::unique_ptr<StateTree> state) 
        : GameBot(std::move(state), 1) {}

    int makeMove() override;
};

// ��� � ������� ���� ��������
class MediumBot : public GameBot {
public:
    MediumBot(std::unique_ptr<StateTree> state) 
        : GameBot(std::move(state), 2) {}

    int makeMove() override;
};

// ��� � ������� ���� ��������
class HardBot : public GameBot {
public:
    HardBot(std::unique_ptr<StateTree> state) 
        : GameBot(std::move(state), 3) {}

    int makeMove() override;
};

// ������� ���
class ExpertBot : public GameBot {
public:
    ExpertBot(std::unique_ptr<StateTree> state) 
        : GameBot(std::move(state), 4) {}

    int makeMove() override;
};

