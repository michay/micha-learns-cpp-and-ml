#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <math.h>

#define BOARD_SIZE_X 3
#define BOARD_SIZE_Y 3
#define WIN_CLAUSE 3

enum class BoardPiece
{
   Empty,
   X,
   O
};

class TicTacToe
{
   BoardPiece next_move;
   BoardPiece winner;
   BoardPiece board[BOARD_SIZE_X][BOARD_SIZE_Y];

   void check_winner();
   BoardPiece check_row(int x, int y);
   BoardPiece check_col(int x, int y);
   BoardPiece check_diag(int x, int y);

public:
   std::vector<int> possible_moves;

   TicTacToe();

   void set_piece(int index);
   void play_random_move();

   int is_game_over() { return possible_moves.empty() || winner != BoardPiece::Empty; }
   BoardPiece who_won() { return winner; }

   void print_board();
};

class MCNode
{
   int simulations;
   int victories;
   int last_access;
   int move;

public:
   std::vector<MCNode*> children;

   MCNode(game_move);

   void add_child(MCNode* child_p);

   int get_simulations() { return simulations; }
   int get_wins() { return victories; }

   float get_ucb1(int total_simulations);
};

class MCTS
{
   MCNode root;
   MCNode* create_child();

   void print_tree(int level, MCNode &root);

public:
   MCTS() : root(-1)  {}

   // Game related
   void find_next_move(TicTacToe& board);

   // Print related
   void print_tree();
};

MCNode* MCTS::create_child(void)
{
   MCNode* child_p = new MCNode(timestamp);

   root.add_child(child_p);

   return child_p;
}

void MCTS::find_next_move(TicTacToe& board)
{
   // Create copy of game
   TicTacToe game = board;

   for (auto& move: board.possible_moves)
   {
      root.add_child(
   }
}

void MCTS::print_tree()
{
   print_tree(0, root);
}

void MCTS::print_tree(int level, MCNode &root)
{
   std::cout << std::string(level, ' ') << ">";
   std::cout << "simulations: " << root.get_simulations() << " wins: " << root.get_wins()  << std::endl;
   for(auto& child: root.children)
   {
      print_tree(level + 1, *child);
   }
}

MCNode::MCNode(int game_move)
{
   simulations = 0;
   victories = 0;
   move = game_move;
}

float MCNode::get_ucb1(int total_simulations)
{
   return static_cast<float>(victories)/simulations + 2*sqrt(log(total_simulations)/simulations);
}

void MCNode::add_child(MCNode* child_p)
{
   children.push_back(child_p);
}

TicTacToe::TicTacToe(void) : possible_moves(BOARD_SIZE_X*BOARD_SIZE_Y), next_move(BoardPiece::X), winner(BoardPiece::Empty)
{
   // Reset board
   memset(board, 0, sizeof(board));

   // Reset possible moves
   std::iota(possible_moves.begin(), possible_moves.end(), 0);
}

void TicTacToe::set_piece(int index)
{
   auto x = index / BOARD_SIZE_X;
   auto y = index % BOARD_SIZE_Y;

   // Set piece on the board
   board[x][y] = next_move;

   // Remove from possible moves
   possible_moves.erase(std::remove(possible_moves.begin(), possible_moves.end(), index), possible_moves.end());

   // Set next move
   next_move = (next_move == BoardPiece::X) ? BoardPiece::O : BoardPiece::X;

   // Check winner
   check_winner();
}

void TicTacToe::check_winner()
{
   // Check rows
   for (int i = 0; i < BOARD_SIZE_X; ++i)
   {
      for (int j = 0; j < BOARD_SIZE_Y; ++j)
      {
         if ((winner = check_row(i, j)) != BoardPiece::Empty)
            return;
         if ((winner = check_col(i, j)) != BoardPiece::Empty)
            return;
         if ((winner = check_diag(i, j)) != BoardPiece::Empty)
            return;
      }
   }
}

BoardPiece TicTacToe::check_col(int x, int y)
{
   auto base = board[x][y];

   if (base == BoardPiece::Empty)
      return base;

   if (x + WIN_CLAUSE > BOARD_SIZE_X)
   {
      return BoardPiece::Empty;
   }

   for (int i = 0; i < WIN_CLAUSE; ++i)
   {
      auto piece = board[x + i][y];
      if (piece != base)
      {
         return BoardPiece::Empty;
      }
   }

   return base;
}

BoardPiece TicTacToe::check_row(int x, int y)
{
   auto base = board[x][y];

   if (base == BoardPiece::Empty)
      return base;

   if (y + WIN_CLAUSE > BOARD_SIZE_Y)
      return BoardPiece::Empty;

   for (int i = 0; i < WIN_CLAUSE; ++i)
   {
      auto piece = board[x][y + i];
      if (piece != base)
         return BoardPiece::Empty;
   }

   return base;
}

BoardPiece TicTacToe::check_diag(int x, int y)
{
   auto base = board[x][y];

   if (base == BoardPiece::Empty)
      return base;

   if ((y + WIN_CLAUSE) > BOARD_SIZE_Y)
      return BoardPiece::Empty;

   if ((x + WIN_CLAUSE) <= BOARD_SIZE_X)
   {
      for (int i = 0; i < WIN_CLAUSE; ++i)
      {
         auto piece = board[x + i][y + i];
         if (piece != base)
            return BoardPiece::Empty;
      }
      
      return base;
   }

   if (x >= WIN_CLAUSE)
   {
      for (int i = 0; i < WIN_CLAUSE; ++i)
      {
         auto piece = board[x + i - WIN_CLAUSE][y + i];
         if (piece != base)
            return BoardPiece::Empty;
      }

      return base;
   }
   
   return BoardPiece::Empty;
}


void TicTacToe::play_random_move()
{
   int index = possible_moves[rand() % possible_moves.size()];
   set_piece(index);
}

void TicTacToe::print_board(void)
{
   for (int i = 0; i < BOARD_SIZE_X; ++i)
   {
      for (int j = 0; j < BOARD_SIZE_Y; ++j)
      {
         auto piece = board[i][j];
         if (piece == BoardPiece::Empty)
            std::cout << " ";
         else if (piece == BoardPiece::X)
            std::cout << "X";
         else if (piece == BoardPiece::O)
            std::cout << "O";

         if (j < BOARD_SIZE_Y - 1)
            std::cout << "|";
      }
      std::cout << std::endl;

      if (i < BOARD_SIZE_X - 1)
      {
         for (int j = 0; j < BOARD_SIZE_Y*2 - 1; ++j)
            std::cout << "-";
         std::cout << std::endl;
      }
   }
   std::cout << std::endl;
}

int main(void)
{
   TicTacToe game;
   int user_input;
   auto next_piece = BoardPiece::X;

   srand(0);

   while (!game.is_game_over() && ((std::cout << "Select next location:", std::cin >> user_input, user_input) != -1))
   {
      // Set user  piece
      game.set_piece(user_input);

      // Set PC piece
      //game.play_random_move();
      
      // Print board
      game.print_board();
      
      // Create MCTS object
      MCTS tree;
      tree.find_next_move(game);
      tree.print_tree();
   }

   std::cout << "Winner is: " << static_cast<int>(game.who_won());

// std::cin >> user_input;
// std::cout << "user: " << user_input;

   return 0;
}
