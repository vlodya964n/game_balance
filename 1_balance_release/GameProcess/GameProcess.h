#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <clocale>
#include <fstream>

#include "open_spiel/abseil-cpp/absl/random/uniform_int_distribution.h"
#include "open_spiel/spiel.h"
#include "open_spiel/spiel_utils.h"

#include "..\GameAnalysis\GameAnalysis.h"

class GameProcess
{
public:
	void ConductRandomGame(std::string);
	void ConductRandomGame(std::string, open_spiel::GameParameters);

	void ConductGame(std::string);
	void ConductGame(std::string, open_spiel::GameParameters);
	
	void GetRegisteredGames();

	void DoGameAnalysis(std::string, std::string);
	void DoGameAnalysis(std::string, open_spiel::GameParameters, std::string);

	void DoChanceNodeGameAnalysis(std::string, std::string);
	void DoChanceNodeGameAnalysis(std::string, open_spiel::GameParameters, std::string);

private:
	void StartGame(std::shared_ptr<const open_spiel::Game>);

	void StartRandomGame(std::shared_ptr<const open_spiel::Game>);
	
	void StartGameAnalysis(std::shared_ptr<const open_spiel::Game>, std::string);
	std::unique_ptr<GameAnalysis> FixResult(std::unique_ptr<GameAnalysis>, std::ofstream&, std::string, int);

	void StartChanceNodeAnalysis(std::shared_ptr<const open_spiel::Game>, std::string);
	void FixChanceNodeResult(std::vector<int>, std::ofstream&, std::string);
};
