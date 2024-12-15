#pragma once

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "open_spiel/spiel.h"

// Simple game of Noughts and Crosses:
// https://en.wikipedia.org/wiki/Tic-tac-toe
//
// Parameters: none

namespace open_spiel {
    namespace modified_tic_tac_toe {

        // Constants.
        inline constexpr int kNumPlayers = 2;
        inline constexpr int kNumRows = 3;
        inline constexpr int kNumCols = 3;
        inline constexpr int kNumCells = kNumRows * kNumCols;
        inline constexpr int kGameLength = 9;
        inline constexpr int kCellStates = 1 + kNumPlayers;  // empty, 'x', and 'o'.

        // https://math.stackexchange.com/questions/485752/tictactoe-state-space-choose-calculation/485852
        inline constexpr int kNumberStates = 5478;

        // State of a cell.
        enum class CellState {
            kEmpty,
            kNought,  // O
            kCross,   // X
        };

        class ModifiedTicTacToeGame;  // Needed for back-pointer to parent game.

        // State of an in-play game.
        class ModifiedTicTacToeState : public State {
        public:
            ModifiedTicTacToeState(std::shared_ptr<const Game> game);

            ModifiedTicTacToeState(const ModifiedTicTacToeState&) = default;
            ModifiedTicTacToeState& operator=(const ModifiedTicTacToeState&) = default;

            Player CurrentPlayer() const override {
                return IsTerminal() ? kTerminalPlayerId : current_player_;
            }
            std::string ActionToString(Player player, Action action_id) const override;
            std::string ToString() const override;
            bool IsTerminal() const override;
            std::vector<double> Returns() const override;
            std::string InformationStateString(Player player) const override;
            std::string ObservationString(Player player) const override;
            void ObservationTensor(Player player,
                absl::Span<float> values) const override;
            std::unique_ptr<State> Clone() const override;
            void UndoAction(Player player, Action move) override;
            std::vector<Action> LegalActions() const override;
            CellState BoardAt(int cell) const { return board_[cell]; }
            CellState BoardAt(int row, int column) const {
                return board_[row * kNumCols + column];
            }
            Player outcome() const { return outcome_; }

            // Only used by Ultimate Tic-Tac-Toe.
            void SetCurrentPlayer(Player player) { current_player_ = player; }

        protected:
            std::array<CellState, kNumCells> board_;
            void DoApplyAction(Action move) override;

        private:
            const ModifiedTicTacToeGame& parent_game_;
            bool HasLine(Player player) const;  // Does this player have a line?
            bool IsFull() const;                // Is the board full?
            Player current_player_ = 0;         // Player zero goes first
            Player outcome_ = kInvalidPlayer;
            int num_moves_ = 0;
        };

        // Game object.
        class ModifiedTicTacToeGame : public Game {
        public:
            explicit ModifiedTicTacToeGame(const GameParameters& params);
            int NumDistinctActions() const override { return kNumCells; }
            std::unique_ptr<State> NewInitialState() const override {
                return std::unique_ptr<State>(new ModifiedTicTacToeState(shared_from_this()));
            }
            int NumPlayers() const override { return kNumPlayers; }
            double MinUtility() const override { return -1; }
            absl::optional<double> UtilitySum() const override { return 0; }
            double MaxUtility() const override { return 1; }
            std::vector<int> ObservationTensorShape() const override {
                return { kCellStates, kNumRows, kNumCols };
            }
            int MaxGameLength() const override { return max_game_length_; }
            std::string ActionToString(Player player, Action action_id) const override;

        private:
            const int max_game_length_;
        };

        CellState PlayerToState(Player player);
        std::string StateToString(CellState state);

        // Does this player have a line?
        bool BoardHasLine(const std::array<CellState, kNumCells>& board,
            const Player player);

        inline std::ostream& operator<<(std::ostream& stream, const CellState& state) {
            return stream << StateToString(state);
        }

    }  // namespace tic_tac_toe
}  // namespace open_spiel