// 1_balance_release.cpp: исходный файл для целевого объекта.
//
#include <iostream>
#include <vector>
#include <memory>
#include <clocale>
#include <fstream>
#include <chrono>
#include <string>

using namespace std::string_literals;

#include "open_spiel/abseil-cpp/absl/random/uniform_int_distribution.h"
#include "open_spiel/spiel.h"
#include "open_spiel/spiel_utils.h"

#include "open_spiel/games/tic_tac_toe/tic_tac_toe.cc"


#include "1_balance_release.h"
#include "GameProcess/GameProcess.h"
#include "GameAnalysis/GameAnalysis.h"
#include "PlayingGame/PlayingGame.h"
#include "PlayingTwoPlayersGame/PlayingTwoPlayersGame.h"

void test() {
	auto game = open_spiel::LoadGame("mushroom_glade_4x6");
	auto state = game->NewInitialState();

	std::ofstream out;
	out.open("comment.txt", std::ios::app);
	out << "Start counting outcomes...\n";
	out.close();
	std::cout << "Start counting outcomes...\n";
	// Определяем начальную точку времени
	auto start = std::chrono::high_resolution_clock::now();

	std::unique_ptr<StateOutcomesTree> outcomesTree = std::make_unique<StateOutcomesTree>();
	outcomesTree->FindStateTree(std::move(state));

	// Определяем конечную точку времени
	auto end = std::chrono::high_resolution_clock::now();
	// Вычисляем продолжительность
	std::chrono::duration<double> duration = end - start;
	out.open("comment.txt", std::ios::app);
	out << "Outcomes counted. Time = " << duration.count() << " s.\n";
	
	std::cout << "Outcomes counted. Time = " << duration.count() << " s.\n";

	
	out << "Outcomes serialize...\n";
	out.close();
	std::cout << "Outcomes serialize...\n";
	start = std::chrono::high_resolution_clock::now();

	outcomesTree->Serialize("");

	end = std::chrono::high_resolution_clock::now();
	// Вычисляем продолжительность
	duration = end - start;
	out.open("comment.txt", std::ios::app);
	out << "Outcomes serialized. Time = " << duration.count() << " s.\n";
	out.close();
	std::cout << "Outcomes serialized. Time = " << duration.count() << " s.\n";

	out.close();
}

//void printTree(std::unique_ptr<StateTree> infoState) {
//	std::cout << "[" << infoState->stateHistory << "]" << "\n";
//	for (auto state_ : infoState->states) {
//		printTree(state_.second);
//	}
//}

void test2() {

	std::unique_ptr<StateOutcomesTree> outcomesTree = std::make_unique<StateOutcomesTree>();

	std::ofstream out;
	out.open("comment.txt", std::ios::app);

	out << "Outcomes deserialize...\n";
	out.close();
	std::cout << "Outcomes deserialize...\n";
	auto start = std::chrono::high_resolution_clock::now();

	outcomesTree->Deserialize("mg4x6_outcomes_tree111_optim.bin");

	// Определяем конечную точку времени
	auto end = std::chrono::high_resolution_clock::now();
	// Вычисляем продолжительность
	std::chrono::duration<double> duration = end - start;
	out.open("comment.txt", std::ios::app);
	out << "Outcomes deserialized. Time = " << duration.count() << " s.\n";
	std::cout << "Outcomes deserialized. Time = " << duration.count() << " s.\n";

	out << "Outcomes serialize...\n";
	out.close();
	std::cout << "Outcomes serialize...\n";
	start = std::chrono::high_resolution_clock::now();

	outcomesTree->Serialize("mg4x6_outcomes_tree333_optim.bin");
	
	// Определяем конечную точку времени
	end = std::chrono::high_resolution_clock::now();
	// Вычисляем продолжительность
	duration = end - start;
	out.open("comment.txt", std::ios::app);
	out << "Outcomes serialized. Time = " << duration.count() << " s.\n";
	std::cout << "Outcomes serialized. Time = " << duration.count() << " s.\n";

	out.close();

	//printTree(outcomesTree->GetTree());

	//std::cout << "Outcomes search...\n";
	//std::shared_ptr<GameAnalysis> firstPlayerAnalysis = std::make_shared<FirstPlayerAnalysis>();
	//outcomesTree->SetTree(
	//	firstPlayerAnalysis->
	//	StartedStateSearch(std::move(outcomesTree->GetTree()), 2)
	//);
	//std::cout << "Outcomes searched\n";

	/*std::ofstream out;
	out.open("test_mg_outcomes.txt", std::ios::app);
	std::cout << "Open file for writting...\n";
	out << "Strategy_name;Step_num;State_history;" <<
		"First_player_wins_num;Second_player_wins_num;Equal_results_num;All_variants_num;" <<
		"First_player_wins_percent;Second_player_wins_percent;Equal_results_percent;\n";

	auto results = firstPlayerAnalysis->GetStartedStateOutcomesNum();
	auto history = "[" + firstPlayerAnalysis->GetStartedStateHistory() + "]";

	int winX = results[0], winO = results[1], equalRezults = results[2];
	double allWins = winX + winO + equalRezults;

	out << "General_strategy" << ";" << 1 << ";" << history << ";" <<
		winX << ";" << winO << ";" << equalRezults << ";" << allWins << ";" <<
		winX / allWins << ";" << winO / allWins << ";" << equalRezults / allWins << ";" << "\n";

	out.close();
	std::cout << "File has been written" << std::endl;*/
}

//void printTree(std::shared_ptr<StateTree> infoState, std::ofstream& out) {
//	out << "[" << infoState->stateHistory << "]" << "\n";
//	for (auto state_ : infoState->states) {
//		printTree(state_.second, out);
//	}
//}

void test3() {
	auto game = open_spiel::LoadGame("mushroom_glade");
	auto state = game->NewInitialState();

	std::cout << "Start counting outcomes...\n";
	std::unique_ptr<StateOutcomesTree> outcomesTree = std::make_unique<StateOutcomesTree>();
	outcomesTree->FindStateTree(std::move(state));
	std::cout << "Outcomes counted\n";

	

	std::ofstream out;
	out.open("test_mg_outcomes.txt", std::ios::app);
	std::cout << "Open file for writting...\n";
	
	//printTree(outcomesTree->GetTree(), out);

	out.close();
	std::cout << "File has been written" << std::endl;
}

void printSizes() {
	int i = 720;
	std::string
		str = "0",
		str1 = "0, 0",
		str2 = "0, 0, 1",
		str3 = "0, 0, 1, 2",
		str4 = "0, 0, 1, 2, 3",
		str5 = "0, 0, 1, 2, 3, 4",
		dtr6 = " ";
	std::cout << sizeof(i) << "\n";
	std::cout << sizeof(str) << "\n";
	std::cout << sizeof(str1) << "\n";
	std::cout << sizeof(str2) << "\n";
	std::cout << sizeof(str3) << "\n";
	std::cout << sizeof(str4) << "\n";
	std::cout << sizeof(str5) << "\n";
	std::cout << sizeof(dtr6) << "\n";
}

void doSmth() {
	auto game = open_spiel::LoadGame("mushroom_glade_3x4x4");
	auto state = game->NewInitialState();
	std::unique_ptr<StateOutcomesTree> outcomesTree = std::make_unique<StateOutcomesTree>();
	outcomesTree->FindAndSerializeStateTree(
		std::move(state), 
		"mg_3x4x4", "4", "3x4", 
		"statistic_file.txt", "log_file.txt", 2);
}

int main()
{
	setlocale(LC_CTYPE, "rus");

	// Проигрыш игры с компьютером
	//std::unique_ptr<PlayingGame> goGame = std::make_unique<PlayingGame>();
	//goGame->run();

	// Проигрыш игры двух игроков
	//std::unique_ptr<PlayingTwoPlayersGame> goGame = std::make_unique<PlayingTwoPlayersGame>();
	//goGame->run();

	// Проигрыш начальной игры
	//std::unique_ptr<GameProcess> playGame = std::make_unique<GameProcess>();
	//playGame->ConductGame("mushroom_glade_5x6x6");


	//playGame->GetRegisteredGames();

	// Вычисление дерева исходов для игры
	//doSmth();
	//playGame->ConductGame("mushroom_glade_5x6x6");
	
	// Проигрыш крестики нолики
	/*open_spiel::GameParameters params;
	params["max_game_length"] = open_spiel::GameParameter(9);
	playGame->ConductGame("modified_tic_tac_toe");*/
	
	// Анализ крестики нолики
	//playGame->DoGameAnalysis("modified_tic_tac_toe", "outcomes2.txt");

	// Анализ Грибной поляны с параметрами
	/*open_spiel::GameParameters params;
	params["max_game_length"] = open_spiel::GameParameter(4);
	playGame->DoChanceNodeGameAnalysis("mushroom_glade", params, "mg_len4_outcomes.txt");*/

	
	// Анализ грибной поляны с параметрами
	/*open_spiel::GameParameters params;
	params["max_game_length"] = open_spiel::GameParameter(5);
	params["bonus_diff"] = open_spiel::GameParameter(5);*/
	//playGame->DoChanceNodeGameAnalysis("mushroom_glade_4x6", "mg_4x6_outcomes1.txt");
	//playGame->DoChanceNodeGameAnalysis("mushroom_glade", "mg_len6_cn_outcomes.txt");

	//playGame->DoGameAnalysis("mushroom_glade", "mg_len6_cn_outcomes1.txt");


	//playGame->DoChanceNodeGameAnalysis("mushroom_glade", "mg_len6_outcomes.txt");

	//test3_1("modified_tic_tac_toe", "outcomes1.txt", 1);



	return 0;
}
