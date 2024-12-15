#include "MushroomGlade.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>
#include <math.h>

#include "open_spiel/spiel_utils.h"
#include "open_spiel/utils/tensor_view.h"

namespace open_spiel {
    namespace MushroomGlade {
        namespace {

            // Информация об игре
            const GameType kGameType{
                /*short_name=*/"mushroom_glade",
                /*long_name=*/"Mushroom Glade",
                GameType::Dynamics::kSequential,
                GameType::ChanceMode::kExplicitStochastic,
                GameType::Information::kPerfectInformation,
                GameType::Utility::kZeroSum,
                GameType::RewardModel::kTerminal,
                /*max_num_players=*/2,
                /*min_num_players=*/2,
                /*provides_information_state_string=*/true,
                /*provides_information_state_tensor=*/false,
                /*provides_observation_string=*/true,
                /*provides_observation_tensor=*/true,
                /*parameter_specification=*/{
                    {"max_game_length", GameParameter(kGameLength)},
                    {"bonus_diff", GameParameter(kBonusDif)},
                    {"rows_num", GameParameter(kNumRows)},
                    {"cols_num", GameParameter(kNumCols)}
            
            }
            };

            std::shared_ptr<const Game> Factory(const GameParameters& params) {
                return std::shared_ptr<const Game>(new MushroomGladeGame(params));
            }

            // Загрузить игру в список зарегистрированных игр
            REGISTER_SPIEL_GAME(kGameType, Factory);

            RegisterSingleTensorObserver single_tensor(kGameType.short_name);

        }  // namespace

        // Состояние игрока на доске
        CellState PlayerToState(Player player) {
            switch (player) {
            case 0:
                return CellState::kCross;
            case 1:
                return CellState::kNought;
            default:
                SpielFatalError(absl::StrCat("Invalid player id ", player));
                return CellState::kEmpty;
            }
        }

        // Состояние игрока на доске для вывода 
        std::string StateToString(CellState state) {
            switch (state) {
            case CellState::kEmpty:
                return ".";
            case CellState::kMark:
                return "1";
            case CellState::kNought:
                return "o";
            case CellState::kCross:
                return "x";
            default:
                SpielFatalError("Unknown state.");
            }
        }


        // Добавить действие. Перевести игру в следующее состояние 
        void MushroomGladeState::DoApplyAction(Action move) {
            if (IsChanceNode()) {
                // Из выибирается начальное игровое поле из списка всех 
                boardVariant = boardVariants[move];

                // В список действий добавляются номера всех 
                // ячеек со свободными предметами для выбора
                actionList.reserve(parent_game_.MaxGameLength());
                for (int i = 0; i < board_.size(); i++) {
                    if (boardVariant[i] == true) {
                        board_[i] = CellState::kMark;
                        actionList.push_back(i);
                    }
                }

                // Для начала игры назначается первый игрок 
                current_player_ = 0;
            }
            else {
                // Является ли выбранная ячейка на доске пустым предметом
                SPIEL_CHECK_EQ(board_[move], CellState::kMark);

                // Соответствующая ячейка помечается меткой текущего игрока
                board_[move] = PlayerToState(CurrentPlayer());

                // Номер ячейки убирается из списка доступных действий
                for (auto it = actionList.begin(); it != actionList.end(); ++it) {
                    if (*it == move) {
                        actionList.erase(it);
                        break;
                    }
                }
                // Ценность предмета в выбранной ячейке добавляется к текущему игроку
                earndeBonus[current_player_] += bonusTable1[current_player_][move];

                // К следующему состоянию меняется игрок и 
                // добавляется действие в количество проигранных
                current_player_ = 1 - current_player_;
                num_moves_ += 1;
            }
        }

        // Вектор всех возможных значений случайного узла 
        // (начальных состояний доски в начале игры)
        std::vector<std::pair<Action, double>> MushroomGladeState::ChanceOutcomes() const {
            SPIEL_CHECK_TRUE(IsChanceNode());
            std::vector<std::pair<Action, double>> outcomes;

            outcomes.reserve(boardVariantsNum);
            for (int i = 0; i < boardVariantsNum; i++) {
                outcomes.push_back(std::make_pair(i, boardChance));
            }

            return outcomes;
        }

        // Список доступных дейсвтий при текущем состоянии игры
        std::vector<Action> MushroomGladeState::LegalActions() const {
            if (IsTerminal()) return {};
            std::vector<Action> moves;
            for (int cell = 0; cell < actionList.size(); ++cell) {
                moves.push_back(actionList[cell]);
            }
            return moves;
        }

        // Вывести дейсвтия игрока в строковом виде
        std::string MushroomGladeState::ActionToString(Player player,
            Action action_id) const {
            return game_->ActionToString(player, action_id);
        }

        // Проверка победы одного из игроков
        bool MushroomGladeState::IsWin(Player player) {
            earndeBonusFirstPlayer = GetEarnedBonusFirstPlayer();
            earndeBonusSecondPlayer = GetEarnedBonusSecondPlayer();
            if (player == 0) {
                return earndeBonusFirstPlayer > earndeBonusSecondPlayer;
            }
            else if (player == 1) {
                return earndeBonusFirstPlayer < earndeBonusSecondPlayer;
            }
            return false;
        }

        // Получить суммарную стоимость предметов первого игрока
        int MushroomGladeState::GetEarnedBonusFirstPlayer() {
            int sum = 0;
            for (int r = 0; r < kNumRows; r++) {
                for (int c = 0; c < kNumCols; c++) {
                    if (board_[r * kNumCols + c] == CellState::kCross) {
                        sum += bonusTable[r * kNumCols + c];
                    }
                }
            }
            return sum;
        }

        // Получить суммарную стоимость предметов второго игрока
        int MushroomGladeState::GetEarnedBonusSecondPlayer() {
            int sum = 0;
            for (int r = 0; r < kNumRows; r++) {
                for (int c = 0; c < kNumCols; c++) {
                    if (board_[r * kNumCols + c] == CellState::kNought) {
                        sum += bonusTable[r * kNumCols + (kNumCols - 1) - c];
                    }
                }
            }
            return sum;
        }

        // Проверка на остаток свободных предметов на доске
        bool MushroomGladeState::IsFull() const { 
            return num_moves_ == parent_game_.MaxGameLength(); 
        }

        // Конструктор начального состояния игры
        MushroomGladeState::MushroomGladeState(std::shared_ptr<const Game> game) 
            : State(game),
            parent_game_(open_spiel::down_cast<const MushroomGladeGame&>(*game)) {
            std::fill(begin(board_), end(board_), CellState::kEmpty);
            //board_[kNumCols * (kNumRows / 2)] = CellState::kCross;
            //board_[kNumCols * (kNumRows / 2) + kNumCols - 1] = CellState::kNought;
        }

        // Текущее состояние игры в виде доски для вывода
        std::string MushroomGladeState::ToString() const {
            std::string str;
            for (int r = 0; r < kNumRows; ++r) {
                for (int c = 0; c < kNumCols; ++c) {
                    absl::StrAppend(&str, StateToString(BoardAt(r, c)));
                }
                if (r < (kNumRows - 1)) {
                    absl::StrAppend(&str, "\n");
                }
            }
            return str;
        }

        // Проверка на конечное состояние
        bool MushroomGladeState::IsTerminal() const {
            return outcome_ != kInvalidPlayer || IsFull();
        }

        // Результаты игры
        std::vector<double> MushroomGladeState::Returns() const {
            if (earndeBonus[0] > earndeBonus[1]) {
                return { 1.0, -1.0 };
            }
            else if (earndeBonus[0] < earndeBonus[1]) {
                return { -1.0, 1.0 };
            }
            else {
                return { 0.0, 0.0 };
            }
        }

        std::string MushroomGladeState::InformationStateString(Player player) const {
            SPIEL_CHECK_GE(player, 0);
            SPIEL_CHECK_LT(player, num_players_);
            return HistoryString();
        }

        std::string MushroomGladeState::ObservationString(Player player) const {
            SPIEL_CHECK_GE(player, 0);
            SPIEL_CHECK_LT(player, num_players_);
            return ToString();
        }

        void MushroomGladeState::ObservationTensor(Player player,
            absl::Span<float> values) const {
            SPIEL_CHECK_GE(player, 0);
            SPIEL_CHECK_LT(player, num_players_);

            // Treat `values` as a 2-d tensor.
            TensorView<2> view(values, { kCellStates, kNumCells }, true);
            for (int cell = 0; cell < kNumCells; ++cell) {
                view[{static_cast<int>(board_[cell]), cell}] = 1.0;
            }
        }

        void MushroomGladeState::UndoAction(Player player, Action move) {
            board_[move] = CellState::kMark;
            current_player_ = player;
            outcome_ = kInvalidPlayer;
            num_moves_ -= 1;
            history_.pop_back();
            --move_number_;
        }

        std::unique_ptr<State> MushroomGladeState::Clone() const {
            return std::unique_ptr<State>(new MushroomGladeState(*this));
        }

        std::string MushroomGladeGame::ActionToString(Player player,
            Action action_id) const {
            return absl::StrCat(StateToString(PlayerToState(player)), "(",
                action_id / kNumCols, ",", action_id % kNumCols, ")");
        }

        // Конструктор игры
        MushroomGladeGame::MushroomGladeGame(const GameParameters& params)
            : Game(kGameType, params),
              max_game_length_(ParameterValue<int>("max_game_length", kGameLength)) {
            StatMaxGameLength = ParameterValue<int>("max_game_length", kGameLength);
            BonusDiff = ParameterValue<int>("bonus_diff", kBonusDif);
            StatNumRows = ParameterValue<int>("rows_num", kNumRows);
            StatNumCols = ParameterValue<int>("cols_num", kNumCols);
        }

        // Генерация списка всех начальных состояний игры
        void FillBoardVariants() {
            
            int boardNum = 1;
            for (int i = 0; i < StatMaxGameLength; i++) {
                boardNum *= (kNumRows * kNumCols - i);
            }

            boardNum /= StatMaxGameLength;
            boardVariants.clear();
            boardVariants.reserve(boardNum);
            int n_c = kNumCols, n_r = kNumRows;
            
            std::vector<bool> a_board(n_c * n_r, false);
            RecurInsertElementOnBoard(a_board, 0, n_r, n_c, StatMaxGameLength - 1);

            boardVariantsNum = boardVariants.size();
            boardChance = 1.0 / boardVariantsNum;
        }

        // Функция вставки предмета на доску, 
        // рекурсивно вставляется фиксированное 
        // число предметов,после чего доска сохраняется 
        // С помощью неё перебираются все варианты 
        // расположения m предметов на доске и сохраняются 
        // в список начальных состояний для случайного 
        // выбора одного из них в начале игры
        void RecurInsertElementOnBoard(std::vector<bool> board_, int startPosition, int n_r, int n_c, int len) {
            
            if (len >= 0) {
                for (int x = startPosition; x < n_c * n_r - len; x++) {
                    board_[x] = true;
                    RecurInsertElementOnBoard(board_, x + 1, n_r, n_c, len - 1);
                    board_[x] = false;
                }
            }
            else {
                boardVariants.push_back(board_);
            }
        }

        // Заполнить бонусную карту поля bonusTable для двух игроков
        void FillBonusTable() {
            // Заполнение карты ценностей для первого игрока
            bonusTable.clear();
            bonusTable.reserve(kNumCols * kNumRows);
            int centralRowIndex = kNumRows / 2;
            for (int r = 0; r < kNumRows; r++) {
                for (int c = 0; c < kNumCols; c++) {
                    bonusTable.push_back(kNumRows - abs(centralRowIndex - r) + kNumCols - c);
                }
            }

            for (int c = 0; c < kNumCols; c++) {
                diffScale.push_back(bonusTable[c] - bonusTable[kNumCols - 1 - c]);
            }

            bonusTable1.clear();
            bonusTable1.push_back(bonusTable);

            // Заполнение карты ценностей для второго игрока
            std::vector<int> reversBT;
            reversBT.reserve(bonusTable.size());
            for (int r = 0; r < bonusTable.size(); r++) {
                reversBT.push_back(bonusTable[r] - diffScale[r % kNumCols]);
            }
            bonusTable1.push_back(reversBT);
        }

        // Получить сумму очков по бонусной таблице первого игрока
        int GetSumBonusFirstPlayer(std::vector<bool> a_board_) {
            int sum = 0;
            for (int r = 0; r < kNumRows; r++) {
                for (int c = 0; c < kNumCols; c++) {
                    if (a_board_[r * kNumCols + c] == true) {
                        sum += bonusTable[r * kNumCols + c];
                    }
                }
            }
            return sum;
        }

        // Получить сумму очков по бонусной таблице второго игрока
        int GetSumBonusSecondPlayer(std::vector<bool> a_board_) {
            int sum = 0;
            for (int r = 0; r < kNumRows; r++) {
                for (int c = 0; c < kNumCols; c++) {
                    if (a_board_[r * kNumCols + c] == true) {
                        sum += bonusTable[r * kNumCols + (kNumCols - 1) - c];
                    }
                }
            }
            return sum;
        }

        // Получить номер столбца для балансировки 
        // разницы бонусных сумм игроков
        int GetBalancedPosition(int& difference) {
            int balancedCol = 0;
            int minDifferrence = abs(diffScale[0] + difference);
            int newDifference = diffScale[0] + difference;
            for (int c = 1; c < diffScale.size(); c++) {
                if (abs(diffScale[c] + difference) < minDifferrence) {
                    balancedCol = c;
                    newDifference = diffScale[c] + difference;
                    minDifferrence = abs(newDifference);
                }
            }

            difference = newDifference;
            return balancedCol;
        }

    }  // namespace 
}  // namespace open_spiel