#pragma once
#include <windows.h>  // ��� ������ � CreateFile, CreateFileMapping, MapViewOfFile, UnmapViewOfFile
#include <map>        // ��� std::map
#include <memory>     // ��� std::unique_ptr
#include <string>     // ��� std::string
#include <stdexcept>  // ��� std::runtime_error
#include <cstring>    // ��� memcpy
#include <iostream>   // ��� ��������� ������ (�����������)

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
// ���������� ������ ������� ��� ���� ���������

struct StateTree {
	// ���������� �������� ������� ��� ������� ������ � ���������
	int winFirstPlayerSum{ 0 };  
	// ���������� �������� ������� ������� ������ � ���������
	int winSecondPlayerSum{ 0 };
	// ���������� �������-������ � ���������
	int equalResultsSum{ 0 };  

	// ��� ���������� �������� ���������
	std::map<int, std::unique_ptr<StateTree>> states;  


	void saveToBinary(std::ofstream& out) const {
		// ��������� ������� ����
		out.write(reinterpret_cast<const char*>(&winFirstPlayerSum), sizeof(winFirstPlayerSum));
		out.write(reinterpret_cast<const char*>(&winSecondPlayerSum), sizeof(winSecondPlayerSum));
		out.write(reinterpret_cast<const char*>(&equalResultsSum), sizeof(equalResultsSum));

		// ��������� ���������� �����
		size_t numStates = states.size();
		out.write(reinterpret_cast<const char*>(&numStates), sizeof(numStates));

		// ��������� ����� � ���������
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
		// ��������� ������� ����
		in.read(reinterpret_cast<char*>(&winFirstPlayerSum), sizeof(winFirstPlayerSum));
		in.read(reinterpret_cast<char*>(&winSecondPlayerSum), sizeof(winSecondPlayerSum));
		in.read(reinterpret_cast<char*>(&equalResultsSum), sizeof(equalResultsSum));

		// ��������� ���������� �����
		size_t numStates;
		in.read(reinterpret_cast<char*>(&numStates), sizeof(numStates));

		// ��������� �����
		std::vector<int> keys(numStates);
		in.read(reinterpret_cast<char*>(keys.data()), keys.size() * sizeof(int));

		// ��������� ���������
		for (size_t i = 0; i < numStates; ++i) {
			auto child = std::make_unique<StateTree>();
			child->loadFromBinary(in);
			states[keys[i]] = std::move(child);
		}
	}
};



class StateOutcomesTree {
private:
	std::unique_ptr<StateTree> infoStateTree;  // ������ ������� ��� ���� ���������
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
// ����� ��������� �� ������ �������

class GameAnalysis {
public:
	virtual std::unique_ptr<StateTree> StartedStateSearch(std::unique_ptr<StateTree>, int) = 0;
	std::string GetStartedStateHistory() const;
	std::unique_ptr<int[]> GetStartedStateOutcomesNum();

protected:
	int depthLimit;  // ������� ������ �� ������ �������
	double minDiff;  // �������������� �������� ���� ������ �������, 
	// ����������� ������� ����� ����������� ������ �������
	std::unique_ptr<StateTree> optimalStartedState; // ������� ���� ������ �������
};

class TreeAnalysis : public GameAnalysis {
public:
	std::unique_ptr<StateTree> StartedStateSearch(std::unique_ptr<StateTree>, int) override;
};

// ����� ��������� ��� ������ ������� ������
class FirstPlayerAnalysis : public GameAnalysis {
public:
	std::unique_ptr<StateTree> StartedStateSearch(std::unique_ptr<StateTree>, int) override;

private:
	std::unique_ptr<StateTree> FirstPlayerStartedStateSearch(std::unique_ptr<StateTree>, int);
};

// ����� ��������� ��� ������ ������� ������
class SecondPlayerAnalysis : public GameAnalysis {
public:
	std::unique_ptr<StateTree> StartedStateSearch(std::unique_ptr<StateTree>, int) override;

private:
	std::unique_ptr<StateTree> SecondPlayerStartedStateSearch(std::unique_ptr<StateTree>, int);
};

// ����� ��������� ��� ������
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