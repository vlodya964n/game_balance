#include "MushroomGlade4x6.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>
#include <math.h>

#include "open_spiel/spiel_utils.h"
#include "open_spiel/utils/tensor_view.h"

namespace open_spiel {
    namespace MushroomGlade4x6 {
        namespace {

            // Facts about the game.
            const GameType kGameType{
                /*short_name=*/"mushroom_glade_4x6",
                /*long_name=*/"Mushroom Glade 4x6",
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
                    {"bonus_diff", GameParameter(kBonusDif)}
            }
            };

            std::shared_ptr<const Game> Factory(const GameParameters& params) {
                return std::shared_ptr<const Game>(new MushroomGlade4x6Game(params));
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
        void MushroomGlade4x6State::DoApplyAction(Action move) {
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

        std::vector<std::pair<Action, double>> MushroomGlade4x6State::ChanceOutcomes() const {
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
        std::vector<Action> MushroomGlade4x6State::LegalActions() const {
            if (IsTerminal()) return {};
            // Can move in any empty cell.
            std::vector<Action> moves;
            for (int cell = 0; cell < actionList.size(); ++cell) {
                moves.push_back(actionList[cell]);
            }
            return moves;
        }

        std::string MushroomGlade4x6State::ActionToString(Player player,
            Action action_id) const {
            return game_->ActionToString(player, action_id);
        }

        // Checking winning combinations on the board
        bool MushroomGlade4x6State::IsWin(Player player) {
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

        int MushroomGlade4x6State::GetEarnedBonusFirstPlayer() {
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
        int MushroomGlade4x6State::GetEarnedBonusSecondPlayer() {
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
        bool MushroomGlade4x6State::IsFull() const { return num_moves_ == parent_game_.MaxGameLength(); }

        // The game constructor. Filling the board with initial values
        MushroomGlade4x6State::MushroomGlade4x6State(std::shared_ptr<const Game> game)
            : State(game),
            parent_game_(open_spiel::down_cast<const MushroomGlade4x6Game&>(*game)) {
            std::fill(begin(board_), end(board_), CellState::kEmpty);
            //board_[kNumCols * (kNumRows / 2)] = CellState::kCross;
            //board_[kNumCols * (kNumRows / 2) + kNumCols - 1] = CellState::kNought;
        }

        // Return the current State on the board
        std::string MushroomGlade4x6State::ToString() const {
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
        bool MushroomGlade4x6State::IsTerminal() const {
            return outcome_ != kInvalidPlayer || IsFull();
        }

        // Return player's winnings
        std::vector<double> MushroomGlade4x6State::Returns() const {
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

        std::string MushroomGlade4x6State::InformationStateString(Player player) const {
            SPIEL_CHECK_GE(player, 0);
            SPIEL_CHECK_LT(player, num_players_);
            return HistoryString();
        }

        std::string MushroomGlade4x6State::ObservationString(Player player) const {
            SPIEL_CHECK_GE(player, 0);
            SPIEL_CHECK_LT(player, num_players_);
            return ToString();
        }

        void MushroomGlade4x6State::ObservationTensor(Player player,
            absl::Span<float> values) const {
            SPIEL_CHECK_GE(player, 0);
            SPIEL_CHECK_LT(player, num_players_);

            // Treat `values` as a 2-d tensor.
            TensorView<2> view(values, { kCellStates, kNumCells }, true);
            for (int cell = 0; cell < kNumCells; ++cell) {
                view[{static_cast<int>(board_[cell]), cell}] = 1.0;
            }
        }

        void MushroomGlade4x6State::UndoAction(Player player, Action move) {
            board_[move] = CellState::kMark;
            current_player_ = player;
            outcome_ = kInvalidPlayer;
            num_moves_ -= 1;
            history_.pop_back();
            --move_number_;
        }

        std::unique_ptr<State> MushroomGlade4x6State::Clone() const {
            return std::unique_ptr<State>(new MushroomGlade4x6State(*this));
        }

        std::string MushroomGlade4x6Game::ActionToString(Player player,
            Action action_id) const {
            return absl::StrCat(StateToString(PlayerToState(player)), "(",
                action_id / kNumCols, ",", action_id % kNumCols, ")");
        }

        MushroomGlade4x6Game::MushroomGlade4x6Game(const GameParameters& params)
            : Game(kGameType, params),
            max_game_length_(ParameterValue<int>("max_game_length", kGameLength)) {
            StatMaxGameLength = ParameterValue<int>("max_game_length", kGameLength);
            BonusDiff = ParameterValue<int>("bonus_diff", kBonusDif);
        }


        void FillBoardVariants() {

            int boardNum = 1;
            for (int i = 0; i < StatMaxGameLength / 2; i++) {
                boardNum *= (kNumRows * (kNumCols / 2) - i) * (kNumRows * (kNumCols / 2) - i);
            }
            boardVariants.clear();
            boardVariants.reserve(boardNum);
            int n_c = kNumCols, n_r = kNumRows, n_size = n_c / 2;
            for (int x1 = 0; x1 < n_size * n_r - 2; x1++) {
                for (int x2 = x1 + 1; x2 < n_size * n_r - 1; x2++) {
                    for (int x3 = x2 + 1; x3 < n_size * n_r; x3++) {
                        for (int o1 = 0; o1 < n_size * n_r - 2; o1++) {
                            for (int o2 = o1 + 1; o2 < n_size * n_r - 1; o2++) {
                                for (int o3 = o2 + 1; o3 < n_size * n_r; o3++) {
                                    std::vector<bool> a_board(n_c * n_r, false);
                                    a_board[x1 / n_size * n_c + x1 % n_size] = true;
                                    a_board[x2 / n_size * n_c + x2 % n_size] = true;
                                    a_board[x3 / n_size * n_c + x3 % n_size] = true;
                                    a_board[(o1 / n_size) * n_c + o1 % n_size + n_size] = true;
                                    a_board[(o2 / n_size) * n_c + o2 % n_size + n_size] = true;
                                    a_board[(o3 / n_size) * n_c + o3 % n_size + n_size] = true;

                                    // Добавить две ячейки через метод GetBalancedPosition


                                    // Добавить вектор в общий вектор
                                    boardVariants.push_back(a_board);
                                }
                            }
                        }
                    }
                    
                }
            }
            boardVariantsNum = boardVariants.size();
            boardChance = 1.0 / boardVariantsNum;
        }

        // Заполнить бонусную таблицу bonusTable
        void FillBonusTable() {
            bonusTable.clear();
            bonusTable.reserve(kNumCols * kNumRows);
            int centralRowIndex = kNumRows / 2;
            for (int r = 0; r < kNumRows; r++) {
                for (int c = 0; c < kNumCols; c++) {
                    bonusTable.push_back(kNumRows - ((r % (kNumRows - 1)) == 0 ? 1 : 0) + kNumCols - c);
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