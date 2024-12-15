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

        // Заданные игровые параметры
        // Количество игроков
        inline constexpr int kNumPlayers = 2;
        // Количество строк игрового поля
        inline constexpr int kNumRows = 3;
        // Количество стролбцов игрового поля
        inline constexpr int kNumCols = 4;
        // Общее количество ячеек игрового поля
        inline constexpr int kNumCells = kNumRows * kNumCols;
        // Длина игры (количество предметов на поле)
        inline constexpr int kGameLength = 4;
        inline constexpr int kBonusDif = 0;
        // Количество различных состояний ячейки
        inline constexpr int kCellStates = 2 + kNumPlayers;  // '.', '1', 'o', 'x'


        // Состояние ячейки поля
        enum class CellState {
            kEmpty,   // .
            kMark,    // 1
            kNought,  // O
            kCross,   // X
        };

        class MushroomGladeGame;

        // Объект состояния игры
        class MushroomGladeState : public State {
        public:
            MushroomGladeState(std::shared_ptr<const Game> game);

            MushroomGladeState(const MushroomGladeState&) = default;
            MushroomGladeState& operator=(const MushroomGladeState&) = default;

            Player CurrentPlayer() const override { 
                // Какой игрок ходит на текущем состоянии игры
                return IsTerminal() ? kTerminalPlayerId : current_player_;
            }
            std::string ActionToString(Player player, Action action_id) const override;

            // Список элементов для выбора в случайном узле 
            std::vector<std::pair<Action, double>> ChanceOutcomes() const override; 
            std::string ToString() const override;
            
            // Ялвяется ли конечным текущее состояние игры
            bool IsTerminal() const override; 
            
            // Показать результаты игры, значения ф-ции полезности для игроков
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
            // Доска, на которой будут выбираться предметы на протяжении игры
            std::array<CellState, kNumCells> board_; 
            // Выбрать действие игрока для текущего состояния
            void DoApplyAction(Action move) override; 

        private:
            const MushroomGladeGame& parent_game_;

            // Игрок победил?
            bool IsWin(Player player);

            // Свободные предметы на доске закончились?
            bool IsFull() const;

            // Игра начинается со случайного узла, 
            // который выбирает начальное состояние доски
            Player current_player_ = kChancePlayerId; 
            Player outcome_ = kInvalidPlayer;
            int num_moves_ = 0;

            // Выбранное для игры начальное состояние
            std::vector<bool> boardVariant; 

            // Список доступных действий для игроков
            std::vector<int> actionList; 

            // Вычислить суммарную стоимость 
            // всех предметов для игроков
            int GetEarnedBonusFirstPlayer(); 
            int GetEarnedBonusSecondPlayer(); 
            std::vector<int> earndeBonus{ 0, 0 };

            int earndeBonusFirstPlayer = 0;
            int earndeBonusSecondPlayer = 0;
        };

        // Функции инициализации набора 
        // начальных игровых состояний
        static void RecurInsertElementOnBoard(std::vector<bool>, int, int, int, int); 
        static void FillBoardVariants();
        static void FillBonusTable();
        static void InsertElementOnBoard(std::vector<bool>, int, int, const int&, const int&);

        // Параметры для инициализации в начале игры
        static int StatNumRows; // Количество строк игрового поля
        static int StatNumCols; // Количество столбцов игрового поля
        static int StatMaxGameLength; // Количество ходов (предметов на поле)
        static int BonusDiff{kBonusDif};

        // Суммарная стоимость всех предметов для каждого игрока
        static int GetSumBonusFirstPlayer(std::vector<bool>); 
        static int GetSumBonusSecondPlayer(std::vector<bool>);
        static int GetBalancedPosition(int&);

        // Шанс выбора одного их состояний в игре
        static double boardChance;
        // Количество начальных состояний
        static int boardVariantsNum; 
        // Список всех начальных состояний 
        // в виде досок с расположенными на них предметах
        static std::vector <std::vector<bool>> boardVariants; 

        static std::vector<int> bonusTable;
        static std::vector<int> diffScale; 
        // карты ценностей предметов для двух игроков
        static std::vector<std::vector<int>> bonusTable1; 

        // Объект игры
        class MushroomGladeGame : public Game {
        public:
            explicit MushroomGladeGame(const GameParameters& params);
            int NumDistinctActions() const override { return kGameLength; }

            // Инициализация состояния игры, 
            // генерация всех начальных состояний игрового поля 
            std::unique_ptr<State> NewInitialState() const override { 
                if (boardVariants.empty()) {
                    FillBonusTable();
                    FillBoardVariants();
                }
                return std::unique_ptr<State>(new MushroomGladeState(shared_from_this()));
            }
            // Количество игроков игры
            int NumPlayers() const override { return kNumPlayers; }
            // Минимальный выигрыш 
            double MinUtility() const override { return -1; }
            absl::optional<double> UtilitySum() const override { return 0; }
            // Максимальный выигрыш
            double MaxUtility() const override { return 1; }
            std::vector<int> ObservationTensorShape() const override {
                return { kCellStates, kNumRows, kNumCols };
            }

            // Количество ходов в игре
            int MaxGameLength() const override { return max_game_length_; } 
            std::string ActionToString(Player player, Action action_id) const override;

        private:
            const int max_game_length_;
        };

        // Состояния ячейки определенного игрока
        CellState PlayerToState(Player player);
        // Состояние игровой доски в сточном виде
        std::string StateToString(CellState state);


        inline std::ostream& operator<<(std::ostream& stream, const CellState& state) {
            return stream << StateToString(state);
        }

    }  // namespace 
}  // namespace open_spiel