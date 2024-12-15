#pragma once
#include <windows.h>  // Для работы с CreateFile, CreateFileMapping, MapViewOfFile, UnmapViewOfFile
#include <map>        // Для std::map
#include <memory>     // Для std::unique_ptr
#include <string>     // Для std::string
#include <stdexcept>  // Для std::runtime_error
#include <cstring>    // Для memcpy
#include <iostream>   // Для тестового вывода (опционально)

#include <chrono>

#include <math.h>
#include <vector>
#include <fstream>

#include <iterator>
#include <sstream>

#include "open_spiel/spiel.h"
#include "open_spiel/spiel_utils.h"
//


/////////////////////StateOutcomesTree///////////////////////
// Вычисление дерева исходов для всех состояний

struct StateTree {
	// Количесвто победных исходов для первого игрока в поддереве
	int winFirstPlayerSum{ 0 };  
	// Количество победных исходов второго игрока в поддереве
	int winSecondPlayerSum{ 0 };
	// Количество исходов-ничьей в поддереве
	int equalResultsSum{ 0 };  

	// Вся поддеревья текущего состояния
	std::map<int, std::unique_ptr<StateTree>> states;  


	void saveToBinary(std::ofstream& out) const {
		// Сохраняем базовые поля
		out.write(reinterpret_cast<const char*>(&winFirstPlayerSum), sizeof(winFirstPlayerSum));
		out.write(reinterpret_cast<const char*>(&winSecondPlayerSum), sizeof(winSecondPlayerSum));
		out.write(reinterpret_cast<const char*>(&equalResultsSum), sizeof(equalResultsSum));

		// Сохраняем количество узлов
		size_t numStates = states.size();
		out.write(reinterpret_cast<const char*>(&numStates), sizeof(numStates));

		// Сохраняем ключи и состояния
		std::vector<int> keys;
		keys.reserve(numStates);
		for (const auto& [key, child] : states) {
			keys.push_back(key);
		}
		out.write(reinterpret_cast<const char*>(keys.data()), keys.size() * sizeof(int));

		for (const auto& [key, child] : states) {
			child->saveToBinary(out);
		}
	}

	void loadFromBinary(std::ifstream& in) {
		// Загружаем базовые поля
		in.read(reinterpret_cast<char*>(&winFirstPlayerSum), sizeof(winFirstPlayerSum));
		in.read(reinterpret_cast<char*>(&winSecondPlayerSum), sizeof(winSecondPlayerSum));
		in.read(reinterpret_cast<char*>(&equalResultsSum), sizeof(equalResultsSum));

		// Загружаем количество узлов
		size_t numStates;
		in.read(reinterpret_cast<char*>(&numStates), sizeof(numStates));

		// Загружаем ключи
		std::vector<int> keys(numStates);
		in.read(reinterpret_cast<char*>(keys.data()), keys.size() * sizeof(int));

		// Загружаем состояния
		for (size_t i = 0; i < numStates; ++i) {
			auto child = std::make_unique<StateTree>();
			child->loadFromBinary(in);
			states[keys[i]] = std::move(child);
		}
	}
};



class StateOutcomesTree {
private:
	std::unique_ptr<StateTree> infoStateTree;  // Дерево исходов для всех состояний
	std::unique_ptr<StateTree> InitialStateTree(std::unique_ptr<open_spiel::State>);
	void SerializeStateTree(
		std::unique_ptr<StateTree>,
		std::string,
		std::string, std::string,
		std::string, std::string);
public:
	void FindAndSerializeStateTree(
		std::unique_ptr<open_spiel::State>,
		std::string, std::string,
		std::string, std::string,
		std::string, int);
	StateOutcomesTree() { infoStateTree = std::make_unique<StateTree>(); }
	void FindStateTree(std::unique_ptr<open_spiel::State>);
	std::unique_ptr<StateTree> GetTree();
	void SetTree(std::unique_ptr<StateTree>);
	void Serialize(std::string);
	void Deserialize(std::string);

};


/////////////////////GameAnalysis///////////////////////
// Поиск состояний по дереву исходов

class GameAnalysis {
public:
	virtual std::unique_ptr<StateTree> StartedStateSearch(std::unique_ptr<StateTree>, int) = 0;
	std::string GetStartedStateHistory() const;
	std::unique_ptr<int[]> GetStartedStateOutcomesNum();

protected:
	int depthLimit;  // Глубина поиска по дереву исходов
	double minDiff;  // Характеристика искомого узла дерева исходов, 
	// минимальная разница между количеством разных исходов
	std::unique_ptr<StateTree> optimalStartedState; // Искомый узел дерева исходов
};

class TreeAnalysis : public GameAnalysis {
public:
	std::unique_ptr<StateTree> StartedStateSearch(std::unique_ptr<StateTree>, int) override;
};

// Поиск состояний для победы первого игрока
class FirstPlayerAnalysis : public GameAnalysis {
public:
	std::unique_ptr<StateTree> StartedStateSearch(std::unique_ptr<StateTree>, int) override;

private:
	std::unique_ptr<StateTree> FirstPlayerStartedStateSearch(std::unique_ptr<StateTree>, int);
};

// Поиск состояний для победы второго игрока
class SecondPlayerAnalysis : public GameAnalysis {
public:
	std::unique_ptr<StateTree> StartedStateSearch(std::unique_ptr<StateTree>, int) override;

private:
	std::unique_ptr<StateTree> SecondPlayerStartedStateSearch(std::unique_ptr<StateTree>, int);
};

// Поиск состояний для ничьей
class EqualResultAnalysis : public GameAnalysis {
public:
	std::unique_ptr<StateTree> StartedStateSearch(std::unique_ptr<StateTree>, int) override;

private:
	std::unique_ptr<StateTree> EqualResultStartedStateSearch(std::unique_ptr<StateTree>, int);
};


/////////////////////ChanceNodeGameAnalysis///////////////////////

class ChanceNodeGameAnalysis {
private:
	int winFirstPlayer;
	int winSecondPlayer;
	int equalRezult;
	int chanceNodeNum;

	void StrategyNum(std::unique_ptr<open_spiel::State>);

public:
	std::vector<int> GetStrategyNum(std::unique_ptr<open_spiel::State>);
	std::vector<std::vector<int>> GetSplitStrategyNum(std::unique_ptr<open_spiel::State>, int);
};