#include "GameProcess.h"

///////////////////////GetRegisterGames/////////////////////////////

void GameProcess::GetRegisteredGames() {

	// Вывести список зарегестрированных игр

	std::vector<std::string> names = open_spiel::RegisteredGames();
	std::cout << "Registered games:" << std::endl;
	for (const std::string& name : names) {
		std::cout << name << std::endl;
	}
	std::cout << std::endl;
}


////////////////////ConductRandomGame///////////////////////////////

void GameProcess::ConductRandomGame(std::string gameName, open_spiel::GameParameters params) {

	// Внешняя функция к рандомному прохождению игры

	//open_spiel::GameParameters params;
	//params["max_game_length"] = open_spiel::GameParameter(9);

	std::cout << gameName << std::endl;
	std::cout << open_spiel::GameParametersToString(params) << std::endl;
	auto game = open_spiel::LoadGame(gameName, params);
	StartRandomGame(game);
}

void GameProcess::ConductRandomGame(std::string gameName) {

	// Внешняя функция к рандомному прохождению игры

	std::cout << gameName << std::endl;
	auto game = open_spiel::LoadGame(gameName);
	StartRandomGame(game);
}

void GameProcess::StartRandomGame(std::shared_ptr<const open_spiel::Game> game) {

	// Рандомное прохождение игры

	std::mt19937 rng(time(0));

	auto state = game->NewInitialState();
	while (!state->IsTerminal()) {

		if (state->IsChanceNode()) {
			std::vector<std::pair<open_spiel::Action, double>> outcomes =
				state->ChanceOutcomes();
			std::cout << "Board count: " << outcomes.size() << std::endl;
			open_spiel::Action action = open_spiel::SampleAction(outcomes, rng).first;
			state->ApplyAction(action); //////////////////////////////////////
			std::cout << "Board:\n";
			std::cout << state->ToString() << std::endl;
		}
		else {
			auto player = state->CurrentPlayer();
			std::cout << "player " << player << std::endl;
			std::cout << "State's history: " << state->InformationStateString(player) << std::endl;

			std::cout << "State: " << std::endl << state->ToString() << std::endl;

			std::vector<open_spiel::Action> actions = state->LegalActions(player);
			std::cout << "Legal Actions: [";
			for (auto act : actions) {
				std::cout << act << ", ";
			}
			std::cout << "]\n";

			absl::uniform_int_distribution<> dis(0, actions.size() - 1);
			auto action = actions[dis(rng)];
			std::cout << "chose action: " << state->ActionToString(player, action)
				<< std::endl;
			state->ApplyAction(action);
			std::cout << std::endl;
		}

	}

	std::cout << "State: " << std::endl << state->ToString() << std::endl;
	for (auto award : state->Returns()) {
		std::cout << award << " ";
	}
}




////////////////////////ConductGame//////////////////////////////////

void GameProcess::ConductGame(std::string gameName, open_spiel::GameParameters params) {
	
	// Внешняя функция к прохождению игры

	//open_spiel::GameParameters params;
	//params["max_game_length"] = open_spiel::GameParameter(9);

	std::cout << gameName << std::endl;
	std::cout << open_spiel::GameParametersToString(params) << std::endl;
	auto game = open_spiel::LoadGame(gameName, params);
	StartGame(game);
}

void GameProcess::ConductGame(std::string gameName) {

	// Внешняя функция к прохождению игры

	std::cout << gameName << std::endl;
	auto game = open_spiel::LoadGame(gameName);
	StartGame(game);
}

void GameProcess::StartGame(std::shared_ptr<const open_spiel::Game> game) {

	// Прохождение игры

	std::mt19937 rng(time(0));

	auto state = game->NewInitialState();
	while (!state->IsTerminal()) { // Текущее состояние не является конечным

		if (state->IsChanceNode()) { // Текущее состояние - случайный узле

			// Возвращается множество возможных действий случайного узла
			// и случайным образом выбирается одно из них, 
			// которое переводит игру в следующее состояние 
			std::vector<std::pair<open_spiel::Action, double>> outcomes =
				state->ChanceOutcomes();
			std::cout << "Board count: " << outcomes.size() << std::endl;
			open_spiel::Action action = 
				open_spiel::SampleAction(outcomes, rng).first;
			state->ApplyAction(action); 
			std::cout << "Board:\n";
			std::cout << state->ToString() << std::endl;
		}
		else { // Текущее состояние - ход одного из двух игроков
			
			// Текущий игрок
			auto player = state->CurrentPlayer(); 
			std::cout << "player " << player << std::endl; 
			
			// История дейсвтий игроков
			std::cout << "State's history: " 
				<< state->InformationStateString(player) 
				<< std::endl;

			// Игровое поле текущего состояния
			std::cout << "State: " << std::endl 
				<< state->ToString() << std::endl;

			// Доступные действия для следующих ходов
			std::vector<open_spiel::Action> actions = 
				state->LegalActions(player);
			std::cout << "Legal Actions: [";
			for (auto act : actions) {
				std::cout << act << ", ";
			}
			std::cout << "]\n\n";

			// Выбор и запись дейсвтия в текущее состояние
			std::cout << "choose your action: ";

			int action;
			std::cin >> action;
			
			std::cout << "chose action: " 
				<< state->ActionToString(player, action)
				<< std::endl;
			state->ApplyAction(action);
			std::cout << std::endl;
		}

	}

	// Вывод результатов игры
	std::cout << "State: " << std::endl << state->ToString() << std::endl;
	for (auto award : state->Returns()) {
		std::cout << award << " ";
	}
}



///////////////////////////DoGameAnalysis/////////////////////////////////

void GameProcess::DoGameAnalysis(std::string gameName, open_spiel::GameParameters params, std::string fileName) {
	
	// Внешняя функция к вычислению исходов
	
	auto game = open_spiel::LoadGame(gameName, params);
	StartGameAnalysis(game, fileName);
}

void GameProcess::DoGameAnalysis(std::string gameName, std::string fileName) {
	
	// Внешняя функция к вычислению исходов
	
	auto game = open_spiel::LoadGame(gameName);
	StartGameAnalysis(game, fileName);
}

void GameProcess::StartGameAnalysis(std::shared_ptr<const open_spiel::Game> game, std::string fileName) {
	
	// Вычисление разных исходов игры 

	auto state = game->NewInitialState();

	std::cout << "Start counting outcomes...\n";
	std::unique_ptr<StateOutcomesTree> outcomesTree = std::make_unique<StateOutcomesTree>();
	outcomesTree->FindStateTree(std::move(state));

	std::cout << "Outcomes counted\n";

	std::unique_ptr<GameAnalysis> firstPlayerAnalysis = std::make_unique<FirstPlayerAnalysis>();
	
	/*firstPlayerAnalysis->
		StartedStateSearch(outcomesTree->GetTree(), 1);*/

	outcomesTree->SetTree(
		firstPlayerAnalysis->
			StartedStateSearch(std::move(outcomesTree->GetTree()), 1)
		);

	std::ofstream out;
	out.open(fileName, std::ios::app);
	std::cout << "Open file for writting...\n";
	out << "Strategy_name;Step_num;State_history;" <<
			"First_player_wins_num;Second_player_wins_num;Equal_results_num;All_variants_num;" <<
			"First_player_wins_percent;Second_player_wins_percent;Equal_results_percent;\n";

	firstPlayerAnalysis = FixResult(std::move(firstPlayerAnalysis), out, "General_strategy", 1);

	out.close();
	std::cout << "File has been written" << std::endl;
}

std::unique_ptr<GameAnalysis> GameProcess::FixResult(std::unique_ptr<GameAnalysis> analysis, std::ofstream& out, std::string outcomesName, int startStepsNum) {
	
	// Запись исходов игры в файл
	
	auto results = analysis->GetStartedStateOutcomesNum();
	auto history = "[" + analysis->GetStartedStateHistory() + "]";

	int winX = results[0], winO = results[1], equalRezults = results[2];
	double allWins = winX + winO + equalRezults;
	
	out << outcomesName << ";" << startStepsNum << ";" << history << ";" <<
		winX << ";" << winO << ";" << equalRezults << ";" << allWins << ";" <<
		winX / allWins << ";" << winO / allWins << ";" << equalRezults / allWins << ";" << "\n";

	return std::move(analysis);
}

///////////////////////////DoChanceNodeGameAnalysis/////////////////////////////////

void GameProcess::DoChanceNodeGameAnalysis(std::string gameName, open_spiel::GameParameters params, std::string fileName) {

	// Внешняя функция к вычислению исходов

	auto game = open_spiel::LoadGame(gameName, params);
	StartChanceNodeAnalysis(game, fileName);
}

void GameProcess::DoChanceNodeGameAnalysis(std::string gameName, std::string fileName) {

	// внешняя функция к вычислению исходов игры со случайным корнем

	auto game = open_spiel::LoadGame(gameName);
	StartChanceNodeAnalysis(game, fileName);
}

void GameProcess::StartChanceNodeAnalysis(std::shared_ptr<const open_spiel::Game> game, std::string fileName) {
	
	// Вычисление разных исходов игры со случайным корнем дерева состояний
	
	std::unique_ptr<ChanceNodeGameAnalysis> gameAnalysis = std::make_unique<ChanceNodeGameAnalysis>();

	std::cout << "Start counting outcomes...\n";

	auto state = game->NewInitialState();
	//auto results = gameAnalysis->GetStrategyNum(std::move(state));
	
	state = game->NewInitialState();
	auto splitResults = gameAnalysis->GetSplitStrategyNum(std::move(state), 60);

	std::cout << "Outcomes counted\n";

	std::ofstream out;
	out.open(fileName, std::ios::app);
	std::cout << "Open file for writting...\n";

	out << "Strategy_name;Chance_nodes_num;" <<
		"First_player_wins_num;Second_player_wins_num;Equal_results_num;All_variants_num;" <<
		"First_player_wins_percent;Second_player_wins_percent;Equal_results_percent;\n";

	std::vector<int> results{0, 0, 0, 0};
	for (auto elem : splitResults) {
		results[0] += elem[0];
		results[1] += elem[1];
		results[2] += elem[2];
	}
	results[3] = splitResults[splitResults.size() - 1][3];

	FixChanceNodeResult(results, out, "General_outcomes");

	for (auto splitResult : splitResults) {
		FixChanceNodeResult(splitResult, out, "Split_outcomes");
	}

	out.close();
	std::cout << "File has been written" << std::endl;
}

void GameProcess::FixChanceNodeResult(std::vector<int> results, std::ofstream& out, std::string outcomesName) {

	// Запись результатов исходов игры со случайным корнем в файл
	
	int winX = results[0], winO = results[1], equalRezults = results[2];
	double allWins = winX + winO + equalRezults;
	int chanceNum = results[3];

	out << outcomesName << ";" << chanceNum << ";" <<
		winX << ";" << winO << ";" << equalRezults << ";" << allWins << ";" <<
		winX / allWins << ";" << winO / allWins << ";" << equalRezults / allWins << ";" << "\n";
}
