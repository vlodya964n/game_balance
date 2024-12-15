#include "ModifiedTicTacToe.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "open_spiel/spiel_utils.h"
#include "open_spiel/utils/tensor_view.h"

namespace open_spiel {
    namespace modified_tic_tac_toe {
        namespace {

            // Facts about the game.
            const GameType kGameType{
                /*short_name=*/"modified_tic_tac_toe",
                /*long_name=*/"Modified Tic Tac Toe",
                GameType::Dynamics::kSequential,
                GameType::ChanceMode::kDeterministic,
                GameType::Information::kPerfectInformation,
                GameType::Utility::kZeroSum,
                GameType::RewardModel::kTerminal,
                /*max_num_players=*/2,
                /*min_num_players=*/2,
                /*provides_information_state_string=*/true,
                /*provides_information_state_tensor=*/false,
                /*provides_observation_string=*/true,
                /*provides_observation_tensor=*/true,
                /*parameter_specification=*/{{"max_game_length", GameParameter(kGameLength)}}
            };

            std::shared_ptr<const Game> Factory(const GameParameters& params) {
                return std::shared_ptr<const Game>(new ModifiedTicTacToeGame(params));
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
            case CellState::kNought:
                return "o";
            case CellState::kCross:
                return "x";
            default:
                SpielFatalError("Unknown state.");
            }
        }

        // Checking winning combinations on the board
        bool BoardHasLine(const std::array<CellState, kNumCells>& board,
            const Player player) {
            CellState c = PlayerToState(player);
            return (board[0] == c && board[1] == c && board[2] == c) ||
                (board[3] == c && board[4] == c && board[5] == c) ||
                (board[6] == c && board[7] == c && board[8] == c) ||
                (board[0] == c && board[3] == c && board[6] == c) ||
                (board[1] == c && board[4] == c && board[7] == c) ||
                (board[2] == c && board[5] == c && board[8] == c) ||
                (board[0] == c && board[4] == c && board[8] == c) ||
                (board[2] == c && board[4] == c && board[6] == c);
        }

        bool BoardEndGame(const std::array<CellState, kNumCells>& board) {
            return (board[0] == CellState::kCross || board[1] == CellState::kCross || board[2] == CellState::kCross)
                && (board[0] == CellState::kNought || board[1] == CellState::kNought || board[2] == CellState::kNought)

                && (board[3] == CellState::kCross || board[4] == CellState::kCross || board[5] == CellState::kCross)
                && (board[3] == CellState::kNought || board[4] == CellState::kNought || board[5] == CellState::kNought)

                && (board[6] == CellState::kCross || board[7] == CellState::kCross || board[8] == CellState::kCross)
                && (board[6] == CellState::kNought || board[7] == CellState::kNought || board[8] == CellState::kNought)

                && (board[0] == CellState::kCross || board[3] == CellState::kCross || board[6] == CellState::kCross)
                && (board[0] == CellState::kNought || board[3] == CellState::kNought || board[6] == CellState::kNought)

                && (board[1] == CellState::kCross || board[4] == CellState::kCross || board[7] == CellState::kCross)
                && (board[1] == CellState::kNought || board[4] == CellState::kNought || board[7] == CellState::kNought)

                && (board[2] == CellState::kCross || board[5] == CellState::kCross || board[8] == CellState::kCross)
                && (board[2] == CellState::kNought || board[5] == CellState::kNought || board[8] == CellState::kNought)

                && (board[0] == CellState::kCross || board[4] == CellState::kCross || board[8] == CellState::kCross)
                && (board[0] == CellState::kNought || board[4] == CellState::kNought || board[8] == CellState::kNought)

                && (board[2] == CellState::kCross || board[4] == CellState::kCross || board[6] == CellState::kCross)
                && (board[2] == CellState::kNought || board[4] == CellState::kNought || board[6] == CellState::kNought);
        }

        // Add the Action. Move the Game to the next State 
        void ModifiedTicTacToeState::DoApplyAction(Action move) {
            SPIEL_CHECK_EQ(board_[move], CellState::kEmpty);
            board_[move] = PlayerToState(CurrentPlayer());
            if (HasLine(current_player_)) {
                outcome_ = current_player_;
            }
            current_player_ = 1 - current_player_;
            num_moves_ += 1;
        }

        // List of available actions on the board
        std::vector<Action> ModifiedTicTacToeState::LegalActions() const {
            if (IsTerminal()) return {};
            // Can move in any empty cell.
            std::vector<Action> moves;
            for (int cell = 0; cell < kNumCells; ++cell) {
                if (board_[cell] == CellState::kEmpty) {
                    moves.push_back(cell);
                }
            }
            return moves;
        }

        std::string ModifiedTicTacToeState::ActionToString(Player player,
            Action action_id) const {
            return game_->ActionToString(player, action_id);
        }

        // Checking winning combinations on the board
        bool ModifiedTicTacToeState::HasLine(Player player) const {
            return BoardHasLine(board_, player);
        }

        // Is the board full?
        bool ModifiedTicTacToeState::IsFull() const { return num_moves_ == parent_game_.MaxGameLength()/* || BoardEndGame(board_)*/; }

        // The game constructor. Filling the board with initial values
        ModifiedTicTacToeState::ModifiedTicTacToeState(std::shared_ptr<const Game> game)
            : State(game),
            parent_game_(open_spiel::down_cast<const ModifiedTicTacToeGame&>(*game)) {
            std::fill(begin(board_), end(board_), CellState::kEmpty);
        }

        // Return the current State on the board
        std::string ModifiedTicTacToeState::ToString() const {
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
        bool ModifiedTicTacToeState::IsTerminal() const {
            return outcome_ != kInvalidPlayer || IsFull();
        }

        // Return player's winnings
        std::vector<double> ModifiedTicTacToeState::Returns() const {
            if (HasLine(Player{ 0 })) {
                return { 1.0, -1.0 };
            }
            else if (HasLine(Player{ 1 })) {
                return { -1.0, 1.0 };
            }
            else {
                return { 0.0, 0.0 };
            }
        }

        std::string ModifiedTicTacToeState::InformationStateString(Player player) const {
            SPIEL_CHECK_GE(player, 0);
            SPIEL_CHECK_LT(player, num_players_);
            return HistoryString();
        }

        std::string ModifiedTicTacToeState::ObservationString(Player player) const {
            SPIEL_CHECK_GE(player, 0);
            SPIEL_CHECK_LT(player, num_players_);
            return ToString();
        }

        void ModifiedTicTacToeState::ObservationTensor(Player player,
            absl::Span<float> values) const {
            SPIEL_CHECK_GE(player, 0);
            SPIEL_CHECK_LT(player, num_players_);

            // Treat `values` as a 2-d tensor.
            TensorView<2> view(values, { kCellStates, kNumCells }, true);
            for (int cell = 0; cell < kNumCells; ++cell) {
                view[{static_cast<int>(board_[cell]), cell}] = 1.0;
            }
        }

        void ModifiedTicTacToeState::UndoAction(Player player, Action move) {
            board_[move] = CellState::kEmpty;
            current_player_ = player;
            outcome_ = kInvalidPlayer;
            num_moves_ -= 1;
            history_.pop_back();
            --move_number_;
        }

        std::unique_ptr<State> ModifiedTicTacToeState::Clone() const {
            return std::unique_ptr<State>(new ModifiedTicTacToeState(*this));
        }

        std::string ModifiedTicTacToeGame::ActionToString(Player player,
            Action action_id) const {
            return absl::StrCat(StateToString(PlayerToState(player)), "(",
                action_id / kNumCols, ",", action_id % kNumCols, ")");
        }

        ModifiedTicTacToeGame::ModifiedTicTacToeGame(const GameParameters& params)
            : Game(kGameType, params),
              max_game_length_(ParameterValue<int>("max_game_length", kGameLength)) {}

    }  // namespace tic_tac_toe
}  // namespace open_spiel