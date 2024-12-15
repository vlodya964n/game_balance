#include "GameAnalysis.h"

//using namespace nvx;

//////////////////////////////////////StateOutcomesTree/////////////////////////////////////////

std::unique_ptr<StateTree> StateOutcomesTree::InitialStateTree(
	std::unique_ptr<open_spiel::State> state_) {

	// Создание и заполнение дерева исходов

	if (state_->IsTerminal()) {
		// Добавление соотвествующего исхода в терминальном состоянии игры
		std::unique_ptr<StateTree> infoState = std::make_unique<StateTree>();
		auto result = state_->Returns()[0];
		if (result > 0) {
			// Победа первого игрока
			infoState->winFirstPlayerSum = 1;
			infoState->winSecondPlayerSum = 0;
			infoState->equalResultsSum = 0;
		}
		else if (result < 0) {
			// Победа второго игрока
			infoState->winFirstPlayerSum = 0;
			infoState->winSecondPlayerSum = 1;
			infoState->equalResultsSum = 0;
		}
		else {
			// Ничья
			infoState->winFirstPlayerSum = 0;
			infoState->winSecondPlayerSum = 0;
			infoState->equalResultsSum = 1;
		}
		return std::move(infoState);
	}

	// Создание узла для текущего состояния, обход всех его дочерних узлов, 
	// и сохранение количества потенциальных исходов из них
	std::unique_ptr<StateTree> infoState = std::make_unique<StateTree>();
	for (auto action_ : state_->LegalActions()) {
		infoState->states[action_] = 
			InitialStateTree(std::move(state_->Child(action_)));
		infoState->winFirstPlayerSum += 
			infoState->states[action_]->winFirstPlayerSum;
		infoState->winSecondPlayerSum += 
			infoState->states[action_]->winSecondPlayerSum;
		infoState->equalResultsSum += 
			infoState->states[action_]->equalResultsSum;
	}
	return std::move(infoState);
}

void StateOutcomesTree::FindStateTree(std::unique_ptr<open_spiel::State> state_) {

	// Внешняя функция к созданию и наполнению дерева исходов
	if (state_->IsChanceNode()) {

		infoStateTree = std::make_unique<StateTree>();///////////
	
		std::vector<std::pair<open_spiel::Action, double>> outcomes =
			state_->ChanceOutcomes();


		for (auto action_ : outcomes) {
			infoStateTree->states[action_.first] = InitialStateTree(std::move(state_->Child(action_.first)));
			infoStateTree->winFirstPlayerSum += infoStateTree->states[action_.first]->winFirstPlayerSum;
			infoStateTree->winSecondPlayerSum += infoStateTree->states[action_.first]->winSecondPlayerSum;
			infoStateTree->equalResultsSum += infoStateTree->states[action_.first]->equalResultsSum;
		}
	}
	else {
		infoStateTree = InitialStateTree(std::move(state_));
	}
	
}

std::unique_ptr<StateTree> StateOutcomesTree::GetTree() {

	// Перенести дерево исходов для дальнейшего использования

	return std::move(infoStateTree);
}

void StateOutcomesTree::SetTree(std::unique_ptr<StateTree> infoState) {

	// Вернуть дерево исходов в объект

	infoStateTree = std::move(infoState);
}

void StateOutcomesTree::Serialize(std::string fileName) {
	//infoStateTree->saveToBinaryMMap(fileName);

	std::cout << "Outcomes serialize...\n";
	auto start = std::chrono::high_resolution_clock::now();

	std::ofstream fout(fileName, std::ios::binary);
	//archive(&fout) << &infoStateTree;
	infoStateTree->saveToBinary(fout);
	fout.close();

	auto end = std::chrono::high_resolution_clock::now();
	// Вычисляем продолжительность
	std::chrono::duration<double> duration = end - start;
	std::cout << "Outcomes serialized. Time = " << duration.count() << " s.\n";
}

void StateOutcomesTree::Deserialize(std::string fileName) {
	//infoStateTree->loadFromBinaryMMap(fileName);

	std::cout << "Outcomes deserialize...\n";
	auto start = std::chrono::high_resolution_clock::now();
	
	std::ifstream fin(fileName, std::ios::binary);
	//archive(&fin) >> &infoStateTree;
	infoStateTree->loadFromBinary(fin);
	fin.close();

	auto end = std::chrono::high_resolution_clock::now();
	// Вычисляем продолжительность
	std::chrono::duration<double> duration = end - start;
	std::cout << "Outcomes deserialized. Time = " << duration.count() << " s.\n";
}

void StateOutcomesTree::FindAndSerializeStateTree(
	std::unique_ptr<open_spiel::State> state_, 
	std::string gameName, std::string gameLength, 
	std::string boardSize, std::string statisticFile,
	std::string logFile, int slice_step) {

	// Функция вычисляет деревья исходов для всех 
	// начальных состояний (outcomes) игры state_. 
	// Каждое дерево исходов относится к определенному 
	// подмножеству из subsets в зависимости от разницы 
	// процентных соотношений побед игроков (diff).
	// Все подмножества записываются в отдельные файлы, 
	// информация о количестве исходов в каждом подмножестве 
	// и об их соотношении записывается в файл statisticFile.
	// Есть вариант разделения каждого подмножетсва 
	// на slice_step равных частей, если в игре слишком много 
	// начальных состояний и сами подмножества оказываются 
	// слишком большими. Это нужно для более быстрой 
	// загрузки/выгрузки файлов подмножеств.

	// Проверка ялвяется ли состояние игры 
	// случайным(начальным) узлом
	if (state_->IsChanceNode()) {

		// Берутся все начальные состояния игры
		std::vector<std::pair<open_spiel::Action, double>> outcomes =
			state_->ChanceOutcomes();

		// Подмножества, к которым будут 
		// добавляться разные начальные состояния игры
		std::vector<std::unique_ptr<StateTree>> subsets;

		for (int i = 0; i < 17; i++) {
			subsets.push_back(std::make_unique<StateTree>());
		}

		// Названия файлов для каждого подмножества
		std::vector<std::string> fileNames{
			"_equalTree0_20.bin",
			"_firstPlayerTree0_5.bin",
			"_firstPlayerTree5_10.bin",
			"_firstPlayerTree10_15.bin",
			"_firstPlayerTree15_20.bin",
			"_firstPlayerTree20_25.bin",
			"_firstPlayerTree25_30.bin",
			"_firstPlayerTree30_35.bin",
			"_firstPlayerTree35_40.bin",
			"_secondPlayerTree0_5.bin",
			"_secondPlayerTree5_10.bin",
			"_secondPlayerTree10_15.bin",
			"_secondPlayerTree15_20.bin",
			"_secondPlayerTree20_25.bin",
			"_secondPlayerTree25_30.bin",
			"_secondPlayerTree30_35.bin",
			"_secondPlayerTree35_40.bin",
		};

		// Запись в файл информации о разных подмножествах 
		// для дальнейшей обработки и представлении в виде таблицы
		std::ofstream out;
		out.open(statisticFile, std::ios::app);
		std::cout << "Open log file for writting...\n";
		out << "File_name;Strategies_count;Game_length;Board-size;" 
			<< "First_player_wins_num;Second_player_wins_num;" 
			<< "Equal_results_num;All_variants_num;" 
			<< "First_player_wins_percent;Second_player_wins_percent;" 
			<< "Equal_results_percent;\n";
		out.close();

		auto start = std::chrono::high_resolution_clock::now();

		// Для разделения подмножеств на несколько частей
		int slice = outcomes.size() / slice_step, step = 1, number = 0;

		// Обрабатываются все начальные состояния игры
		for (auto action_ : outcomes) {

			if (step < slice) {
				step++;
				std::unique_ptr<StateTree> infoState = 
					InitialStateTree(std::move(state_->
						Child(action_.first)
					));
			
				double allRez = infoState->winFirstPlayerSum 
					+ infoState->winSecondPlayerSum 
					+ infoState->equalResultsSum;
				double equalResultsPercent = 
					infoState->equalResultsSum / allRez;


				double firstPlayerPercent = 
					infoState->winFirstPlayerSum / allRez;
				double secondPlayerPercent = 
					infoState->winSecondPlayerSum / allRez;
				
				// Разница между победами игроков 
				// в процентном соотношении.
				// Для оценки на сколько различаются 
				// шансы на победу у игроков
				double diff = firstPlayerPercent - secondPlayerPercent, 
					diff1 = -diff;

				// Дерево исходов каждого начального состояния 
				// относится к одному из подмножеств
				if (abs(diff) < 0.01 && equalResultsPercent < 0.2) {
					// Минимальная разница между количество побед 
					// и минимальный процент ничьей
					subsets[0]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[0]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[0]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[0]->states[action_.first] 
						= std::move(infoState);
				}

				else if (diff <= 0.05 && diff > 0.01) {
					// У перовго игрока больше шансов на 5%
					subsets[1]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[1]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[1]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[1]->states[action_.first] 
						= std::move(infoState);
				}
				else if (diff <= 0.1 && diff > 0.05) {
					// У перовго игрока больше шансов на 10%
					subsets[2]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[2]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[2]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[2]->states[action_.first] 
						= std::move(infoState);
				}
				else if (diff <= 0.15 && diff > 0.1) {
					// У перовго игрока больше шансов на 15%
					subsets[3]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[3]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[3]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[3]->states[action_.first] 
						= std::move(infoState);
				}
				else if (diff <= 0.2 && diff > 0.15) {
					// У перовго игрока больше шансов на 20%
					subsets[4]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[4]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[4]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[4]->states[action_.first] 
						= std::move(infoState);
				}
				else if (diff <= 0.25 && diff > 0.2) {
					// У перовго игрока больше шансов на 25%
					subsets[5]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[5]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[5]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[5]->states[action_.first] 
						= std::move(infoState);
				}
				else if (diff <= 0.3 && diff > 0.25) {
					// У перовго игрока больше шансов на 30%
					subsets[6]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[6]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[6]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[6]->states[action_.first] 
						= std::move(infoState);
				}
				else if (diff <= 0.35 && diff > 0.3) {
					// У перовго игрока больше шансов на 35%
					subsets[7]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[7]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[7]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[7]->states[action_.first] 
						= std::move(infoState);
				}
				else if (diff <= 0.4 && diff > 0.35) {
					// У перовго игрока больше шансов на 40%
					subsets[8]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[8]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[8]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[8]->states[action_.first] 
						= std::move(infoState);
				}

				else if (diff1 <= 0.05 && diff1 > 0.01) {
					// У второго игрока больше шансов на 5%
					subsets[9]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[9]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[9]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[9]->states[action_.first] 
						= std::move(infoState);
				}
				else if (diff1 <= 0.1 && diff1 > 0.05) {
					// У второго игрока больше шансов на 10%
					subsets[10]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[10]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[10]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[10]->states[action_.first] 
						= std::move(infoState);
				}
				else if (diff1 <= 0.15 && diff1 > 0.1) {
					// У второго игрока больше шансов на 15%
					subsets[11]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[11]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[11]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[11]->states[action_.first] 
						= std::move(infoState);
				}
				else if (diff1 <= 0.2 && diff1 > 0.15) {
					// У второго игрока больше шансов на 20%
					subsets[12]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[12]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[12]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[12]->states[action_.first] 
						= std::move(infoState);
				}
				else if (diff1 <= 0.25 && diff1 > 0.2) {
					// У второго игрока больше шансов на 25%
					subsets[13]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[13]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[13]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[13]->states[action_.first] 
						= std::move(infoState);
				}
				else if (diff1 <= 0.3 && diff1 > 0.25) {
					// У второго игрока больше шансов на 30%
					subsets[14]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[14]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[14]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[14]->states[action_.first] 
						= std::move(infoState);
				}
				else if (diff1 <= 0.35 && diff1 > 0.3) {
					// У второго игрока больше шансов на 35%
					subsets[15]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[15]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[15]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[15]->states[action_.first] 
						= std::move(infoState);
				}
				else if (diff1 <= 0.4 && diff1 > 0.35) {
					// У второго игрока больше шансов на 40%
					subsets[16]->winFirstPlayerSum 
						+= infoState->winFirstPlayerSum;
					subsets[16]->winSecondPlayerSum 
						+= infoState->winSecondPlayerSum;
					subsets[16]->equalResultsSum 
						+= infoState->equalResultsSum;
					subsets[16]->states[action_.first] 
						= std::move(infoState);
				}
			}
			else {

				// Запись подмножеств в файлы

				step = 1;
				number++;

				for (int i = 0; i < 17; i++) {
					subsets.push_back(std::make_unique<StateTree>());
					SerializeStateTree(
						std::move(subsets[i]), statisticFile,
						logFile, std::to_string(number) + 
						"_" + gameName + fileNames[i],
						gameLength, boardSize);
				}

				for (int i = 0; i < 17; i++) {
					subsets[i] = std::make_unique<StateTree>();
				}
			}
	
		}

		auto end = std::chrono::high_resolution_clock::now();
		// Вычисляем продолжительность
		std::chrono::duration<double> duration = end - start;
		std::ofstream out1(logFile, std::ios::app);
		out1 << "Outcomes counted. Time = " << duration.count() << " s.\n";
		out1.close();


		std::cout << "Log file has been written" << std::endl;
	}
	else {
		infoStateTree = InitialStateTree(std::move(state_));
	}
}

void StateOutcomesTree::SerializeStateTree(
	std::unique_ptr<StateTree> infoState, 
	std::string statisticFile,
	std::string logFile, std::string treeFileName,
	std::string gameLength, std::string boardSize) {

	// Запись подмножества в файл.
	// В текстовый файл statisticFile записывается 
	// информация о подмножестве.
	
	// treeFileName - название файла с подмножеством, 
	// количество начальных состояний игры в подмножестве,
	// gameLength, boardSize - информация об игре,
	// winX, winO, equalRezults - количества разных 
	// исходов для всех начальных состояний,
	// allWins - общее количество исходов, 
	// процентные шансы каждого исхода.

	// Само подмножество в виде дерева 
	// записывается в бинарный файл  treeFileName

	int winX = infoState->winFirstPlayerSum, 
		winO = infoState->winSecondPlayerSum, 
		equalRezults = infoState->equalResultsSum;
	double allWins = winX + winO + equalRezults;

	std::ofstream out;
	out.open(statisticFile, std::ios::app);
	out << treeFileName << ";" << infoState->states.size() << ";" 
		<< gameLength << ";" << boardSize << ";" <<
		winX << ";" << winO << ";" << equalRezults << ";" 
		<< allWins << ";" << winX / allWins << ";" 
		<< winO / allWins << ";" << equalRezults / allWins 
		<< ";" << "\n";
	out.close();

	out.open(logFile, std::ios::app);
	out << "1. " << treeFileName << " outcomes serialize...\n";
	auto start = std::chrono::high_resolution_clock::now();

	std::ofstream fout(treeFileName, std::ios::binary);
	infoState->saveToBinary(fout);
	fout.close();
	
	auto end = std::chrono::high_resolution_clock::now();
	// Вычисляем продолжительность
	std::chrono::duration<double> duration = end - start;
	out << "Outcomes serialized. Time = " << duration.count() << " s.\n";
	out.close();
}


//////////////////////////////////////GameAnalysis/////////////////////////////////////////

std::string GameAnalysis::GetStartedStateHistory() const {

	// Получить историю состояний вычесленного состояния

	return "optimalStartedState->stateHistory";
}

std::unique_ptr<int[]> GameAnalysis::GetStartedStateOutcomesNum() {

	// Получить количество исходов для вычесленного состояния

	std::unique_ptr<int[]> result{
		new int[3] {
			optimalStartedState->winFirstPlayerSum,
			optimalStartedState->winSecondPlayerSum,
			optimalStartedState->equalResultsSum
		}
	};
	return std::move(result);
}

//////////////////////////////////////TreeAnalysis/////////////////////////////////////////

std::unique_ptr<StateTree> StartedStateSearch(std::unique_ptr<StateTree>, int) {
	
}

//////////////////////////////////////FirstPlayerAnalysis/////////////////////////////////////////

std::unique_ptr<StateTree> FirstPlayerAnalysis::FirstPlayerStartedStateSearch(std::unique_ptr<StateTree> infoState, int depth) {

	// Обход дерева исходов до определенной глубины и поиск самого выгодного состояния для игрока

	if (depth == depthLimit - 1) {


		int winFirstPlayer = infoState->winFirstPlayerSum;
		int winSecondPlayer = infoState->winSecondPlayerSum;
		double sumRez = winFirstPlayer + winSecondPlayer + infoState->equalResultsSum;

		double diff = (winSecondPlayer - winFirstPlayer) / sumRez;

		if (diff < minDiff) {
			minDiff = diff;

			optimalStartedState->winFirstPlayerSum = infoState->winFirstPlayerSum;
			optimalStartedState->winSecondPlayerSum = infoState->winSecondPlayerSum;
			optimalStartedState->equalResultsSum = infoState->equalResultsSum;
			//optimalStartedState->stateHistory = infoState->stateHistory;
		}
	}

	for (auto& stateElement : infoState->states) {
		stateElement.second = FirstPlayerStartedStateSearch(std::move(stateElement.second), depth + 1);
	}

	return std::move(infoState);
}

std::unique_ptr<StateTree> FirstPlayerAnalysis::StartedStateSearch(std::unique_ptr<StateTree> infoState, int depthLimit_) {

	// Внешняя функция к к обходу дерева исходов (переопределенная функция базового класса)

	optimalStartedState = std::make_unique<StateTree>();
	this->depthLimit = depthLimit_;
	this->minDiff = 100;

	return FirstPlayerStartedStateSearch(std::move(infoState), 0);
}

//////////////////////////////////////SecondPlayerAnalysis/////////////////////////////////////////

std::unique_ptr<StateTree> SecondPlayerAnalysis::SecondPlayerStartedStateSearch(std::unique_ptr<StateTree> infoState, int depth) {

	// Обход дерева исходов до определенной глубины и поиск самого выгодного состояния для игрока

	if (depth == depthLimit - 1) {


		int winFirstPlayer = infoState->winFirstPlayerSum;
		int winSecondPlayer = infoState->winSecondPlayerSum;
		double sumRez = winFirstPlayer + winSecondPlayer + infoState->equalResultsSum;

		double diff = (winFirstPlayer - winSecondPlayer) / sumRez;

		if (diff < minDiff) {
			minDiff = diff;

			optimalStartedState->winFirstPlayerSum = infoState->winFirstPlayerSum;
			optimalStartedState->winSecondPlayerSum = infoState->winSecondPlayerSum;
			optimalStartedState->equalResultsSum = infoState->equalResultsSum;
			//optimalStartedState->stateHistory = infoState->stateHistory;
		}
	}

	for (auto& stateElement : infoState->states) {
		stateElement.second = SecondPlayerStartedStateSearch(std::move(stateElement.second), depth + 1);
	}
	return std::move(infoState);
}

std::unique_ptr<StateTree> SecondPlayerAnalysis::StartedStateSearch(std::unique_ptr<StateTree> infoState, int depthLimit_) {

	// Внешняя функция к к обходу дерева исходов (переопределенная функция базового класса)

	optimalStartedState = std::make_unique<StateTree>();
	this->depthLimit = depthLimit_;
	this->minDiff = 100;

	return SecondPlayerStartedStateSearch(std::move(infoState), 0);
}

//////////////////////////////////////EqualResultAnalysis/////////////////////////////////////////

std::unique_ptr<StateTree> EqualResultAnalysis::EqualResultStartedStateSearch(std::unique_ptr<StateTree> infoState, int depth) {

	// Обход дерева исходов до определенной глубины и поиск самого выгодного состояния для игрока

	if (depth == depthLimit - 1) {


		int winFirstPlayer = infoState->winFirstPlayerSum;
		int winSecondPlayer = infoState->winSecondPlayerSum;
		double sumRez = winFirstPlayer + winSecondPlayer + infoState->equalResultsSum;

		double diff = abs(winFirstPlayer - winSecondPlayer) / sumRez;

		if (diff < minDiff) {
			minDiff = diff;

			optimalStartedState->winFirstPlayerSum = infoState->winFirstPlayerSum;
			optimalStartedState->winSecondPlayerSum = infoState->winSecondPlayerSum;
			optimalStartedState->equalResultsSum = infoState->equalResultsSum;
			//optimalStartedState->stateHistory = infoState->stateHistory;
		}
	}

	for (auto& stateElement : infoState->states) {
		stateElement.second = EqualResultStartedStateSearch(std::move(stateElement.second), depth + 1);
	}
	return std::move(infoState);
}

std::unique_ptr<StateTree> EqualResultAnalysis::StartedStateSearch(std::unique_ptr<StateTree> infoState, int depthLimit_) {

	// Внешняя функция к к обходу дерева исходов (переопределенная функция базового класса)

	optimalStartedState = std::make_unique<StateTree>();
	this->depthLimit = depthLimit_;
	this->minDiff = 100;

	return EqualResultStartedStateSearch(std::move(infoState), 0);
}

//////////////////////////////////////ChanceNodeGameAnalysis/////////////////////////////////////////

void ChanceNodeGameAnalysis::StrategyNum(std::unique_ptr<open_spiel::State> state_) {
	if (state_->IsTerminal()) {
		auto result = state_->Returns()[0];
		if (result > 0) {
			winFirstPlayer += 1;
		}
		else if (result < 0) {
			winSecondPlayer += 1;
		}
		else {
			equalRezult += 1;
		}
		return;
	}
	for (auto action_ : state_->LegalActions()) {
		StrategyNum(std::move(state_->Child(action_)));
	}
}

std::vector<int> ChanceNodeGameAnalysis::GetStrategyNum(std::unique_ptr<open_spiel::State> state_) {
	winFirstPlayer = 0;
	winSecondPlayer = 0;
	equalRezult = 0;
	chanceNodeNum = 0;

	if (state_->IsChanceNode()) {
		std::vector<std::pair<open_spiel::Action, double>> outcomes =
			state_->ChanceOutcomes();
		chanceNodeNum = outcomes.size();
		for (auto action_ : outcomes) {
			StrategyNum(std::move(state_->Child(action_.first)));
		}
	}

	std::vector<int> results{winFirstPlayer, winSecondPlayer, equalRezult, chanceNodeNum};
	return results;
}

std::vector<std::vector<int>> ChanceNodeGameAnalysis::GetSplitStrategyNum(std::unique_ptr<open_spiel::State> state_, int split_) {
	winFirstPlayer = 0;
	winSecondPlayer = 0;
	equalRezult = 0;
	chanceNodeNum = 0;

	std::vector<std::vector<int>> results;
	results.reserve(split_ + 1);

	if (state_->IsChanceNode()) {
		std::vector<std::pair<open_spiel::Action, double>> outcomes =
			state_->ChanceOutcomes();
		chanceNodeNum = outcomes.size();

		int splitNum = chanceNodeNum / split_, splitCheck = 0;
		for (auto action_ : outcomes) {
			StrategyNum(std::move(state_->Child(action_.first)));

			splitCheck++;
			if (splitCheck % splitNum == 0) {
				std::vector<int> result{ winFirstPlayer, winSecondPlayer, equalRezult, splitCheck };
				results.push_back(result);
				winFirstPlayer = 0;
				winSecondPlayer = 0;
				equalRezult = 0;
			}
		}
	}

	std::vector<int> result{ winFirstPlayer, winSecondPlayer, equalRezult, chanceNodeNum };
	results.push_back(result);
	return results;
}