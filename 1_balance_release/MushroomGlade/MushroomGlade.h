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
    namespace MushroomGlade {

        // �������� ������� ���������
        // ���������� �������
        inline constexpr int kNumPlayers = 2;
        // ���������� ����� �������� ����
        inline constexpr int kNumRows = 3;
        // ���������� ��������� �������� ����
        inline constexpr int kNumCols = 4;
        // ����� ���������� ����� �������� ����
        inline constexpr int kNumCells = kNumRows * kNumCols;
        // ����� ���� (���������� ��������� �� ����)
        inline constexpr int kGameLength = 4;
        inline constexpr int kBonusDif = 0;
        // ���������� ��������� ��������� ������
        inline constexpr int kCellStates = 2 + kNumPlayers;  // '.', '1', 'o', 'x'


        // ��������� ������ ����
        enum class CellState {
            kEmpty,   // .
            kMark,    // 1
            kNought,  // O
            kCross,   // X
        };

        class MushroomGladeGame;

        // ������ ��������� ����
        class MushroomGladeState : public State {
        public:
            MushroomGladeState(std::shared_ptr<const Game> game);

            MushroomGladeState(const MushroomGladeState&) = default;
            MushroomGladeState& operator=(const MushroomGladeState&) = default;

            Player CurrentPlayer() const override { 
                // ����� ����� ����� �� ������� ��������� ����
                return IsTerminal() ? kTerminalPlayerId : current_player_;
            }
            std::string ActionToString(Player player, Action action_id) const override;

            // ������ ��������� ��� ������ � ��������� ���� 
            std::vector<std::pair<Action, double>> ChanceOutcomes() const override; 
            std::string ToString() const override;
            
            // �������� �� �������� ������� ��������� ����
            bool IsTerminal() const override; 
            
            // �������� ���������� ����, �������� �-��� ���������� ��� �������
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

            void SetCurrentPlayer(Player player) { current_player_ = player; }

        protected:
            // �����, �� ������� ����� ���������� �������� �� ���������� ����
            std::array<CellState, kNumCells> board_; 
            // ������� �������� ������ ��� �������� ���������
            void DoApplyAction(Action move) override; 

        private:
            const MushroomGladeGame& parent_game_;

            // ����� �������?
            bool IsWin(Player player);

            // ��������� �������� �� ����� �����������?
            bool IsFull() const;

            // ���� ���������� �� ���������� ����, 
            // ������� �������� ��������� ��������� �����
            Player current_player_ = kChancePlayerId; 
            Player outcome_ = kInvalidPlayer;
            int num_moves_ = 0;

            // ��������� ��� ���� ��������� ���������
            std::vector<bool> boardVariant; 

            // ������ ��������� �������� ��� �������
            std::vector<int> actionList; 

            // ��������� ��������� ��������� 
            // ���� ��������� ��� �������
            int GetEarnedBonusFirstPlayer(); 
            int GetEarnedBonusSecondPlayer(); 
            std::vector<int> earndeBonus{ 0, 0 };

            int earndeBonusFirstPlayer = 0;
            int earndeBonusSecondPlayer = 0;
        };

        // ������� ������������� ������ 
        // ��������� ������� ���������
        static void RecurInsertElementOnBoard(std::vector<bool>, int, int, int, int); 
        static void FillBoardVariants();
        static void FillBonusTable();
        static void InsertElementOnBoard(std::vector<bool>, int, int, const int&, const int&);

        // ��������� ��� ������������� � ������ ����
        static int StatNumRows; // ���������� ����� �������� ����
        static int StatNumCols; // ���������� �������� �������� ����
        static int StatMaxGameLength; // ���������� ����� (��������� �� ����)
        static int BonusDiff{kBonusDif};

        // ��������� ��������� ���� ��������� ��� ������� ������
        static int GetSumBonusFirstPlayer(std::vector<bool>); 
        static int GetSumBonusSecondPlayer(std::vector<bool>);
        static int GetBalancedPosition(int&);

        // ���� ������ ������ �� ��������� � ����
        static double boardChance;
        // ���������� ��������� ���������
        static int boardVariantsNum; 
        // ������ ���� ��������� ��������� 
        // � ���� ����� � �������������� �� ��� ���������
        static std::vector <std::vector<bool>> boardVariants; 

        static std::vector<int> bonusTable;
        static std::vector<int> diffScale; 
        // ����� ��������� ��������� ��� ���� �������
        static std::vector<std::vector<int>> bonusTable1; 

        // ������ ����
        class MushroomGladeGame : public Game {
        public:
            explicit MushroomGladeGame(const GameParameters& params);
            int NumDistinctActions() const override { return kGameLength; }

            // ������������� ��������� ����, 
            // ��������� ���� ��������� ��������� �������� ���� 
            std::unique_ptr<State> NewInitialState() const override { 
                if (boardVariants.empty()) {
                    FillBonusTable();
                    FillBoardVariants();
                }
                return std::unique_ptr<State>(new MushroomGladeState(shared_from_this()));
            }
            // ���������� ������� ����
            int NumPlayers() const override { return kNumPlayers; }
            // ����������� ������� 
            double MinUtility() const override { return -1; }
            absl::optional<double> UtilitySum() const override { return 0; }
            // ������������ �������
            double MaxUtility() const override { return 1; }
            std::vector<int> ObservationTensorShape() const override {
                return { kCellStates, kNumRows, kNumCols };
            }

            // ���������� ����� � ����
            int MaxGameLength() const override { return max_game_length_; } 
            std::string ActionToString(Player player, Action action_id) const override;

        private:
            const int max_game_length_;
        };

        // ��������� ������ ������������� ������
        CellState PlayerToState(Player player);
        // ��������� ������� ����� � ������� ����
        std::string StateToString(CellState state);


        inline std::ostream& operator<<(std::ostream& stream, const CellState& state) {
            return stream << StateToString(state);
        }

    }  // namespace 
}  // namespace open_spiel