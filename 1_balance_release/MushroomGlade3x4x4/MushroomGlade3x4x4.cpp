#include "MushroomGlade3x4x4.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>
#include <math.h>

#include "open_spiel/spiel_utils.h"
#include "open_spiel/utils/tensor_view.h"

namespace open_spiel {
    namespace MushroomGlade3x4x4 {
        namespace {

            // Facts about the game.
            const GameType kGameType{
                /*short_name=*/"mushroom_glade_3x4x4",
                /*long_name=*/"Mushroom Glade 3x4x4",
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
                return std::shared_ptr<const Game>(new MushroomGladeGame3x4x4(params));
            }

            // Upload the game to the registered list
            REGISTER_SPIEL_GAME(kGameType, Factory);

            RegisterSingleTensorObserver single_tensor(kGameType.short_name);

        }  // namespace



        // Player's State on the board
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

        // The Player's State on the output board
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


        // Add the Action. Move the Game to the next State 
        void MushroomGladeState3x4x4::DoApplyAction(Action move) {
            if (IsChanceNode()) {
                boardVariant = boardVariants[move];
                actionList.reserve(parent_game_.MaxGameLength());
                for (int i = 0; i < board_.size(); i++) {
                    if (boardVariant[i] == true) {
                        board_[i] = CellState::kMark;
                        actionList.push_back(i);
                    }
                }
                current_player_ = 0;
            }
            else {
                SPIEL_CHECK_EQ(board_[move], CellState::kMark);
                board_[move] = PlayerToState(CurrentPlayer());
                for (auto it = actionList.begin(); it != actionList.end(); ++it) {
                    if (*it == move) {
                        actionList.erase(it);
                        break;
                    }
                }
                earndeBonus[current_player_] += bonusTable1[current_player_][move];
                /*if (num_moves_ == kGameLength) {
                    if (IsWin(current_player_)) {
                        outcome_ = current_player_;
                    }
                    else if (IsWin(1 - current_player_)) {
                        outcome_ = 1 - current_player_;
                    }
                }*/

                current_player_ = 1 - current_player_;
                num_moves_ += 1;
            }
        }

        std::vector<std::pair<Action, double>> MushroomGladeState3x4x4::ChanceOutcomes() const {
            SPIEL_CHECK_TRUE(IsChanceNode());
            std::vector<std::pair<Action, double>> outcomes;

            outcomes.reserve(boardVariantsNum);
            for (int i = 0; i < boardVariantsNum; i++) {
                outcomes.push_back(std::make_pair(i, boardChance));
            }

            return outcomes;
        }


        // Далее изменить DoApplyAction, HasLine, BoardHasLine, Return

        // List of available actions on the board
        std::vector<Action> MushroomGladeState3x4x4::LegalActions() const {
            if (IsTerminal()) return {};
            // Can move in any empty cell.
            std::vector<Action> moves;
            for (int cell = 0; cell < actionList.size(); ++cell) {
                moves.push_back(actionList[cell]);
            }
            return moves;
        }

        std::string MushroomGladeState3x4x4::ActionToString(Player player,
            Action action_id) const {
            return game_->ActionToString(player, action_id);
        }

        // Checking winning combinations on the board
        bool MushroomGladeState3x4x4::IsWin(Player player) {
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

        int MushroomGladeState3x4x4::GetEarnedBonusFirstPlayer() {
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
        int MushroomGladeState3x4x4::GetEarnedBonusSecondPlayer() {
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

        // Is the board full?
        bool MushroomGladeState3x4x4::IsFull() const { return num_moves_ == parent_game_.MaxGameLength(); }

        // The game constructor. Filling the board with initial values
        MushroomGladeState3x4x4::MushroomGladeState3x4x4(std::shared_ptr<const Game> game)
            : State(game),
            parent_game_(open_spiel::down_cast<const MushroomGladeGame3x4x4&>(*game)) {
            std::fill(begin(board_), end(board_), CellState::kEmpty);
            //board_[kNumCols * (kNumRows / 2)] = CellState::kCross;
            //board_[kNumCols * (kNumRows / 2) + kNumCols - 1] = CellState::kNought;
        }

        // Return the current State on the board
        std::string MushroomGladeState3x4x4::ToString() const {
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

        // Is the current state final?
        bool MushroomGladeState3x4x4::IsTerminal() const {
            return outcome_ != kInvalidPlayer || IsFull();
        }

        // Return player's winnings
        std::vector<double> MushroomGladeState3x4x4::Returns() const {
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

        std::string MushroomGladeState3x4x4::InformationStateString(Player player) const {
            SPIEL_CHECK_GE(player, 0);
            SPIEL_CHECK_LT(player, num_players_);
            return HistoryString();
        }

        std::string MushroomGladeState3x4x4::ObservationString(Player player) const {
            SPIEL_CHECK_GE(player, 0);
            SPIEL_CHECK_LT(player, num_players_);
            return ToString();
        }

        void MushroomGladeState3x4x4::ObservationTensor(Player player,
            absl::Span<float> values) const {
            SPIEL_CHECK_GE(player, 0);
            SPIEL_CHECK_LT(player, num_players_);

            // Treat `values` as a 2-d tensor.
            TensorView<2> view(values, { kCellStates, kNumCells }, true);
            for (int cell = 0; cell < kNumCells; ++cell) {
                view[{static_cast<int>(board_[cell]), cell}] = 1.0;
            }
        }

        void MushroomGladeState3x4x4::UndoAction(Player player, Action move) {
            board_[move] = CellState::kMark;
            current_player_ = player;
            outcome_ = kInvalidPlayer;
            num_moves_ -= 1;
            history_.pop_back();
            --move_number_;
        }

        std::unique_ptr<State> MushroomGladeState3x4x4::Clone() const {
            return std::unique_ptr<State>(new MushroomGladeState3x4x4(*this));
        }

        std::string MushroomGladeGame3x4x4::ActionToString(Player player,
            Action action_id) const {
            return absl::StrCat(StateToString(PlayerToState(player)), "(",
                action_id / kNumCols, ",", action_id % kNumCols, ")");
        }

        MushroomGladeGame3x4x4::MushroomGladeGame3x4x4(const GameParameters& params)
            : Game(kGameType, params),
            max_game_length_(ParameterValue<int>("max_game_length", kGameLength)) {
            StatMaxGameLength = ParameterValue<int>("max_game_length", kGameLength);
            BonusDiff = ParameterValue<int>("bonus_diff", kBonusDif);
            StatNumRows = ParameterValue<int>("rows_num", kNumRows);
            StatNumCols = ParameterValue<int>("cols_num", kNumCols);
        }


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

            /*for (int x1 = 0; x1 < n_c * n_r - 3; x1++) {
                for (int x2 = x1 + 1; x2 < n_c * n_r - 2; x2++) {
                    for (int o1 = x2 + 1; o1 < n_c * n_r - 1; o1++) {
                        for (int o2 = o1 + 1; o2 < n_c * n_r; o2++) {
                            std::vector<bool> a_board(n_c * n_r, false);
                            a_board[x1] = true;
                            a_board[x2] = true;
                            a_board[o1] = true;
                            a_board[o2] = true;

                            boardVariants.push_back(a_board);
                        }
                    }
                }
            }*/


            /*int boardNum = 1;
            for (int i = 0; i < StatMaxGameLength / 2 - 1; i++) {
                boardNum *= (kNumRows * (kNumCols / 2) - i) * (kNumRows * (kNumCols / 2) - i);
            }

            boardNum /= 4;
            boardNum *= 7;
            boardVariants.clear();
            boardVariants.reserve(boardNum);
            int n_c = kNumCols, n_r = kNumRows, n_size = n_c / 2;*/
            //for (int x1 = 0; x1 < n_size * n_r - 1; x1++) {
            //    for (int x2 = x1 + 1; x2 < n_size * n_r; x2++) {
            //        for (int o1 = 0; o1 < n_size * n_r - 1; o1++) {
            //            for (int o2 = o1 + 1; o2 < n_size * n_r; o2++) {
            //                
            //                std::vector<bool> a_board(n_c * n_r, false);
            //                a_board[x1 / n_size * n_c + x1 % n_size] = true;
            //                a_board[x2 / n_size * n_c + x2 % n_size] = true;
            //                a_board[(o1 / n_size) * n_c + o1 % n_size + n_size] = true;
            //                a_board[(o2 / n_size) * n_c + o2 % n_size + n_size] = true;

            //                // Добавить две ячейки через метод GetBalancedPosition
            //                
            //                int difference = GetSumBonusFirstPlayer(a_board) - GetSumBonusSecondPlayer(a_board);

            //                int addAct = 2;

            //                int balancedColumn1 = GetBalancedPosition(difference), 
            //                    balancedColumn2 = GetBalancedPosition(difference);
            //                InsertElementOnBoard(a_board, balancedColumn1, balancedColumn2, n_r, n_c);

            //                for (int i = 0; i < n_c / 2; i++) {
            //                    for (int j = n_c / 2; j < n_c; j++) {
            //                        if (i + j == n_c - 1) {
            //                            continue;
            //                        }
            //                        InsertElementOnBoard(a_board, i, j, n_r, n_c);
            //                    }
            //                }
            //                
            //            }
            //        }
            //    }
            //}


            boardVariantsNum = boardVariants.size();
            boardChance = 1.0 / boardVariantsNum;
        }

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

        void InsertElementOnBoard(std::vector<bool> board_, int elem1, int elem2, const int& n_r, const int& n_c) {
            for (int r = 0; r < n_r; r++) {
                if (board_[r * n_c + elem1] == false) {
                    board_[r * n_c + elem1] = true;
                    break;
                }
            }

            for (int r = 0; r < n_r; r++) {
                if (board_[r * n_c + elem2] == false) {
                    board_[r * n_c + elem2] = true;
                    break;
                }
            }
            // Добавить вектор в общий вектор
            boardVariants.push_back(board_);
        }

        // Заполнить бонусную таблицу bonusTable
        void FillBonusTable() {
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

        // Получить номер столбца для балансировки разницы бонусных сумм игроков
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

    }  // namespace tic_tac_toe
}  // namespace open_spiel