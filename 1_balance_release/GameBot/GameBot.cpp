#include "GameBot.h"

void GameBot::setCurrentState(int key) {

    // ѕередача дейсвти€ и перевод 
    // дерева исходов в новое состо€ние

    auto it = currentState->states.find(key);
    if (it == currentState->states.end()) {
        throw std::invalid_argument("Invalid state key!");
    }

    currentState = std::move(it->second);
}

int EasyBot::makeMove() {

    // ¬ыбор действи€ из доступных в дереве исходов
    // »щет дейсвтие с наимениьшим количеством побед

    auto it = std::min_element(currentState->states.begin(), 
        currentState->states.end(),
        [](const auto& a, const auto& b) {
            return a.second->winSecondPlayerSum 
                < b.second->winSecondPlayerSum;
        });
    return it != currentState->states.end() ? it->first : -1;
}

int MediumBot::makeMove() {

    // ¬ыбор действи€ из доступных в дереве исходов
    // ¬ыбирает случайное дейсвти€ из половины 
    // с меньшим количеством победных исходов

    std::vector<std::pair<int, StateTree*>> candidates;
    for (const auto& [key, state] : currentState->states) {
        candidates.emplace_back(key, state.get());
    }

    if (candidates.size() == 1) {
        return candidates[0].first;
    }

    std::sort(candidates.begin(), candidates.end(),
        [](const auto& a, const auto& b) {
            return a.second->winSecondPlayerSum 
                < b.second->winSecondPlayerSum;
        });

    size_t halfSize = candidates.size() / 2;
    if (halfSize == 0) return -1;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> 
        dist(0, halfSize - 1);
    return candidates[dist(gen)].first;
}

int HardBot::makeMove() {

    // ¬ыбор действи€ из доступных в дереве исходов
    // ¬ыбирает случайное дейсвти€ из половины 
    // с большим количеством победных исходов

    std::vector<std::pair<int, StateTree*>> candidates;
    for (const auto& [key, state] : currentState->states) {
        candidates.emplace_back(key, state.get());
    }

    if (candidates.size() == 1) {
        return candidates[0].first;
    }

    std::sort(candidates.begin(), candidates.end(),
        [](const auto& a, const auto& b) {
            return a.second->winSecondPlayerSum 
                < b.second->winSecondPlayerSum;
        });

    size_t halfSize = candidates.size() / 2;
    if (halfSize == 0) return -1;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> 
        dist(halfSize, candidates.size() - 1);
    return candidates[dist(gen)].first;
}

int ExpertBot::makeMove() {

    // ¬ыбор действи€ из доступных в дереве исходов
    // »щет дейсвтие с наибольшим количеством побед

    auto it = std::max_element(currentState->states.begin(), 
        currentState->states.end(),
        [](const auto& a, const auto& b) {
            return a.second->winSecondPlayerSum 
                < b.second->winSecondPlayerSum;
        });
    return it != currentState->states.end() ? it->first : -1;
}

