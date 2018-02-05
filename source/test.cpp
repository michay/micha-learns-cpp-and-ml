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
   int last_move_index = -1;

   void check_winner();
   BoardPiece check_row(int x, int y);
   BoardPiece check_col(int x, int y);
   BoardPiece check_diag(int x, int y);

public:
   std::vector<int> possible_moves;

   TicTacToe();

   void set_piece(int index);
   void play_random_move();

   bool is_legal_move(int index) { return find(possible_moves.begin(), possible_moves.end(), index) != possible_moves.end(); }

   bool is_game_over() { return possible_moves.empty() || winner != BoardPiece::Empty; }
   BoardPiece who_won() { return winner; }
   BoardPiece whos_next() { return next_move; }
   int last_move() { return last_move_index; }

   void print_board();
};

class MCNode
{
   int simulations;
   int victories;
   int move;
   bool is_locked;

public:
   TicTacToe game;
   std::vector<MCNode*> children;
   MCNode* parent;

   MCNode(TicTacToe game_board, int game_move);

   void add_child(MCNode* child_p);
   void add_child(TicTacToe game_board, int move);

   int get_move() { return move; }
   int get_simulations() { return simulations; }
   int get_wins() { return victories; }

   void update_simulation_result(bool did_win);

   bool is_first_simulation() { return simulations == 0; }
   double get_ucb1(int total_simulations);

   bool is_terminal() { return is_locked || game.is_game_over(); }
   bool get_locked() { return is_locked; }
   void set_locked(bool locked) { is_locked = locked; }
};

class MCTS
{
   MCNode root;

   void print_tree(int level, MCNode &root);

public:
   MCTS(TicTacToe board): root(board, -1) { }

   static int total_simulations;

   // Game related
   int find_next_move();

   // Print related
   void print_tree();
};
int MCTS::total_simulations = 0;

int MCTS::find_next_move()
{
   // Perform mcts iterations
   MCNode* root_node = &root;
   MCNode* selected_child = nullptr;
   auto max_simulations = 5000;
   BoardPiece next_move = root.game.whos_next();

   while (max_simulations > 0 && !root.is_terminal())
   {
      // Check 0 children
      if (root_node->children.size() == 0)
      {
         // Add all possible moves
         TicTacToe* parent_game = &root_node->game;
         for (auto& move : parent_game->possible_moves)
         {
            // Create copy of board
            TicTacToe game = *parent_game;

            // Play selected move
            game.set_piece(move);

            // Create child
            root_node->add_child(game, move);
         }
      }

      if (root_node->children.size() == 0)
      {
         root_node->set_locked(true);
         root_node = &root;
         selected_child = nullptr;
         continue;
      }

      auto ucb1 = -1.0;
      for (auto& child : root_node->children)
      {
         if (child->get_locked())
         {
            continue;
         }

         if (child->is_first_simulation())
         {
            // Select this child
            selected_child = child;
            break;
         }
         
         auto local_ucb1 = child->get_ucb1(MCTS::total_simulations);
         if (local_ucb1 > ucb1)
         {
            ucb1 = local_ucb1;
            selected_child = child;
         }
      }

      if (!selected_child)
      {
         root_node->set_locked(true);
         
         root_node = &root;
         selected_child = nullptr;
         continue;
      }

      // Check if first visit
      if (selected_child->get_simulations() > 0)
      {
         // Re-scan with child
         root_node = selected_child;
         selected_child = nullptr;
         continue;
      }

      // Leaf node with highest UCB1 - perform rollout with a copy of the game
      TicTacToe game_copy = selected_child->game;
      while (!game_copy.is_game_over())
      {
         game_copy.play_random_move();
      }

      // Check if we won
      auto did_win = game_copy.who_won() == next_move;

      // Update tree
      selected_child->update_simulation_result(did_win);

      // Reset root
      root_node = &root;
      selected_child = NULL;

      // Update performed simulation
      max_simulations--;
   }

   // Select node with highest UCB1
   auto ucb1 = 0.0;
   selected_child = root.children[0];
   for (auto& child : root.children)
   {
      if (!child->is_first_simulation())
      {
         auto compare = child->get_simulations(); //static_cast<double>(child->get_wins()) / child->get_simulations();
         if (ucb1 < compare)
         {
            ucb1 = compare;
            selected_child = child;
         }
      }
   }

   return selected_child->game.last_move();
}

void MCNode::update_simulation_result(bool did_win)
{
   // Update simulations 
   simulations++;
   MCTS::total_simulations++;
   if (did_win)
      victories++;

   if (parent != nullptr)
      parent->update_simulation_result(did_win);
}

void MCTS::print_tree()
{
   print_tree(0, root);
}

void MCTS::print_tree(int level, MCNode &root)
{
   if (level > 1)
      return;

   for (int i = 0; i < level; ++i)
   {
      std::cout << "  ";
   }
   std::cout << ">";
   std::cout << "move: " << root.get_move() << " simulations: " << root.get_simulations() << " wins: " << root.get_wins() << " ucb1: " << root.get_ucb1(MCTS::total_simulations) << std::endl;
   for(auto& child: root.children)
   {
      print_tree(level + 1, *child);
   }
}

MCNode::MCNode(TicTacToe game_board, int game_move)
{
   simulations = 0;
   victories = 0;
   game = game_board;
   move = game_move;
   parent = nullptr;
   is_locked = false;
}
 

double MCNode::get_ucb1(int total_simulations)
{
   return static_cast<double>(victories)/simulations + 2*sqrt(log(total_simulations)/simulations);
}

void MCNode::add_child(TicTacToe game_board, int move)
{
   MCNode* child_p = new MCNode(game_board, move);
   add_child(child_p);
}

void MCNode::add_child(MCNode* child_p)
{
   child_p->parent = this;
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

   // Check already won
   if (winner != BoardPiece::Empty)
      return;

   // Set piece on the board
   board[x][y] = next_move;

   // Remove from possible moves
   possible_moves.erase(std::remove(possible_moves.begin(), possible_moves.end(), index), possible_moves.end());

   // Set next move
   next_move = (next_move == BoardPiece::X) ? BoardPiece::O : BoardPiece::X;
   last_move_index = index;

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
   bool did_find;

   if ((y + WIN_CLAUSE) > BOARD_SIZE_Y)
      return BoardPiece::Empty;

   if (base == BoardPiece::Empty)
      return base;

   if ((x + WIN_CLAUSE) <= BOARD_SIZE_X)
   {
      did_find = true;
      for (int i = 0; i < WIN_CLAUSE; ++i)
      {
         auto piece = board[x + i][y + i];
         if (piece != base)
         {
            did_find = false;
            break;
         }
      }
      
      if (did_find)
         return base;
   }

   if (x >= WIN_CLAUSE-1)
   {
      did_find = true;
      for (int i = 0; i < WIN_CLAUSE; ++i)
      {
         auto piece = board[x - i][y + i];
         if (piece != base)
         {
            did_find = false;
            break;
         }
      }

      if (did_find)
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

   int pc_first;
   std::cout << "select who will start (0 - me, 1 - pc): ";
   std::cin >> pc_first;

   if (pc_first)
   {
      // Create MCTS object
      MCTS tree(game);
      auto pc_move = tree.find_next_move();
      game.set_piece(pc_move);

      // Print board
      game.print_board();
      //tree.print_tree();
   }

   while (!game.is_game_over() && ((std::cout << "Select next location:", std::cin >> user_input, user_input) != -1))
   {
      if (!game.is_legal_move(user_input))
      {
         std::cout << "illgeal move" << std::endl;
         continue;;
      }

      // Set user  piece
      game.set_piece(user_input);

      // Create MCTS object
      MCTS tree(game);
      auto pc_move = tree.find_next_move();
      game.set_piece(pc_move);

      // Print board
      game.print_board();
      //tree.print_tree();
   }

   if (game.who_won() == BoardPiece::X)
      std::cout << "X won!" << std::endl;
   else if (game.who_won() == BoardPiece::O)
      std::cout << "X won!" << std::endl;
   else
      std::cout << "finished a tie" << std::endl;

   // Wait for response
   std::cin >> user_input;

   return 0;
}
