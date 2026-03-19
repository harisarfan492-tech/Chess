#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>

const int BOARD_SIZE = 8;
const int SQ = 80;
const int winW = BOARD_SIZE * SQ;
const int winH = BOARD_SIZE * SQ;

enum Piece {
	EMPTY = 0,W_PAWN,W_KNIGHT,W_BISHOP,W_ROOK,W_QUEEN,W_KING,
	B_PAWN,B_KNIGHT,B_BISHOP,B_ROOK,B_QUEEN,B_KING
};
bool isWhite(Piece piece)
{ return piece >= W_PAWN && piece <= W_KING;
}
bool isBlack(Piece piece)
{ return piece >= B_PAWN && piece <= B_KING;
}
bool isColor(Piece piece,bool white) { return white ? isWhite(piece) : isBlack(piece); }

Piece chessBoard[BOARD_SIZE][BOARD_SIZE];
bool whiteTurn = true;
bool gameOver = false;
std::string statusText = "White to move";

bool hasSelectedPos = false;
std::pair<int,int> selectedPos;
std::vector<std::pair<int,int>> legalMoves;

void printStatus() {
	std::cout << "=================================\n";
	std::cout << statusText << "\n";
	std::cout << "=================================\n";
}

void initBoard() {
	for (int row = 0; row < BOARD_SIZE; row++)
		for (int col = 0; col < BOARD_SIZE; col++)
			chessBoard[row][col] = EMPTY;

	Piece backRow[8] = { W_ROOK, W_KNIGHT, W_BISHOP, W_QUEEN, W_KING, W_BISHOP, W_KNIGHT, W_ROOK };

	for (int col = 0; col < 8; col++) {
		chessBoard[7][col] = backRow[col];
		chessBoard[6][col] = W_PAWN;
		chessBoard[0][col] = (Piece)(backRow[col] + 6);
		chessBoard[1][col] = B_PAWN;
	}
}

bool inBounds(int row,int col) { return row >= 0 && row < 8 && col >= 0 && col < 8; }

std::pair<int,int> locateKing(bool white) {
	Piece king = white ? W_KING : B_KING;
	for (int row = 0; row < 8; row++)
		for (int col = 0; col < 8; col++)
			if (chessBoard[row][col] == king)
				return { row, col };
	return { -1, -1 };
}

bool squareUnderAttack(std::pair<int,int> pos,bool byWhite) {
	for (int row = 0; row < 8; row++) {
		for (int col = 0; col < 8; col++) {
			Piece piece = chessBoard[row][col];
			if (!isColor(piece,byWhite)) continue;

			if (piece == W_PAWN || piece == B_PAWN) {
				int dir = byWhite ? -1 : 1;
				if (row + dir == pos.first && (col - 1 == pos.second || col + 1 == pos.second))
					return true;
			}

			if (piece == W_KNIGHT || piece == B_KNIGHT) {
				int dr = abs(row - pos.first),dc = abs(col - pos.second);
				if ((dr == 2 && dc == 1) || (dr == 1 && dc == 2)) return true;
			}

			if (piece == W_BISHOP || piece == B_BISHOP || piece == W_QUEEN || piece == B_QUEEN) {
				if (abs(row - pos.first) == abs(col - pos.second)) {
					int dr = (pos.first > row) ? 1 : -1;
					int dc = (pos.second > col) ? 1 : -1;
					int r = row + dr,c = col + dc;
					while (r != pos.first || c != pos.second) {
						if (chessBoard[r][c] != EMPTY) break;
						r += dr; c += dc;
					}
					if (r == pos.first && c == pos.second) return true;
				}
			}

			if (piece == W_ROOK || piece == B_ROOK || piece == W_QUEEN || piece == B_QUEEN) {
				if (row == pos.first || col == pos.second) {
					int dr = (pos.first > row) ? 1 : (pos.first < row) ? -1 : 0;
					int dc = (pos.second > col) ? 1 : (pos.second < col) ? -1 : 0;
					int r = row + dr,c = col + dc;
					while (r != pos.first || c != pos.second) {
						if (chessBoard[r][c] != EMPTY) break;
						r += dr; c += dc;
					}
					if (r == pos.first && c == pos.second) return true;
				}
			}

			if (piece == W_KING || piece == B_KING) {
				if (abs(row - pos.first) <= 1 && abs(col - pos.second) <= 1) return true;
			}
		}
	}
	return false;
}

bool inCheck(bool white) {
	std::pair<int,int> kingPos = locateKing(white);
	if (kingPos.first == -1) return false;
	return squareUnderAttack(kingPos,!white);
}

std::vector<std::pair<int,int>> getPseudoLegalMoves(std::pair<int,int> pos) {
	std::vector<std::pair<int,int>> movesList;
	Piece piece = chessBoard[pos.first][pos.second];
	if (piece == EMPTY) return movesList;
	bool white = isWhite(piece);

	auto tryAdd = [&](int row,int col) {
		if (!inBounds(row,col)) return false;
		Piece target = chessBoard[row][col];
		if (target == EMPTY) { movesList.push_back({ row, col }); return true; }
		if (isColor(target,!white)) { movesList.push_back({ row, col }); return false; }
		return false;
		};

	if (piece == W_PAWN || piece == B_PAWN) {
		int dir = white ? -1 : 1;
		int startRow = white ? 6 : 1;
		if (inBounds(pos.first + dir,pos.second) && chessBoard[pos.first + dir][pos.second] == EMPTY) {
			movesList.push_back({ pos.first + dir, pos.second });
			if (pos.first == startRow && chessBoard[pos.first + 2 * dir][pos.second] == EMPTY)
				movesList.push_back({ pos.first + 2 * dir, pos.second });
		}
		for (int dc : {-1,1}) {
			int row = pos.first + dir,col = pos.second + dc;
			if (inBounds(row,col) && isColor(chessBoard[row][col],!white))
				movesList.push_back({ row, col });
		}
	
	}
	else if (piece == W_KNIGHT || piece == B_KNIGHT) {
		int offsets[][2] = { {-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1} };
		for (auto& o : offsets) tryAdd(pos.first + o[0],pos.second + o[1]);
	}
	else if (piece == W_KING || piece == B_KING) {
		for (int dr = -1; dr <= 1; dr++)
			for (int dc = -1; dc <= 1; dc++)
				if (dr != 0 || dc != 0)
					tryAdd(pos.first + dr,pos.second + dc);
	}
	else if (piece == W_QUEEN || piece == B_QUEEN) {
		int dirs[][2] = { {-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1} };
		for (auto& d : dirs)
			for (int i = 1; i < 8; i++)
				if (!tryAdd(pos.first + d[0] * i,pos.second + d[1] * i)) break;
	}
	else if (piece == W_BISHOP || piece == B_BISHOP) {
		int dirs[][2] = { {-1,-1},{-1,1},{1,-1},{1,1} };
		for (auto& d : dirs)
			for (int i = 1; i < 8; i++)
				if (!tryAdd(pos.first + d[0] * i,pos.second + d[1] * i)) break;
	}
	else if (piece == W_ROOK || piece == B_ROOK) {
		int dirs[][2] = { {-1,0},{1,0},{0,-1},{0,1} };
		for (auto& d : dirs)
			for (int i = 1; i < 8; i++)
				if (!tryAdd(pos.first + d[0] * i,pos.second + d[1] * i)) break;
	}

	return movesList;
}

std::vector<std::pair<int,int>> getLegalMoves(std::pair<int,int> pos) {
	std::vector<std::pair<int,int>> pseudo = getPseudoLegalMoves(pos);
	std::vector<std::pair<int,int>> legal;
	Piece piece = chessBoard[pos.first][pos.second];
	bool white = isWhite(piece);

	for (auto& move : pseudo) {
		Piece captured = chessBoard[move.first][move.second];
		chessBoard[move.first][move.second] = piece;
		chessBoard[pos.first][pos.second] = EMPTY;

		if (!inCheck(white)) legal.push_back(move);

		chessBoard[pos.first][pos.second] = piece;
		chessBoard[move.first][move.second] = captured;
	}
	return legal;
}

bool hasLegalMoves(bool white) {
	for (int row = 0; row < 8; row++)
		for (int col = 0; col < 8; col++)
			if (isColor(chessBoard[row][col],white) && !getLegalMoves({ row, col }).empty())
				return true;
	return false;
}

void makeMove(std::pair<int,int> from,std::pair<int,int> to) {
	chessBoard[to.first][to.second] = chessBoard[from.first][from.second];
	chessBoard[from.first][from.second] = EMPTY;
	whiteTurn = !whiteTurn;

	bool inChk = inCheck(whiteTurn);
	bool canMove = hasLegalMoves(whiteTurn);

	if (!canMove) {
		gameOver = true;
		if (inChk) statusText = whiteTurn ? "Checkmate! Black wins!" : "Checkmate! White wins!";
		else statusText = "Stalemate! Draw!";
	}
	else if (inChk) statusText = whiteTurn ? "White in check!" : "Black in check!";
	else statusText = whiteTurn ? "White to move" : "Black to move";

	printStatus();
}

void resetGame() {
	initBoard();
	whiteTurn = true;
	gameOver = false;
	statusText = "White to move";
	hasSelectedPos = false;
	legalMoves.clear();
	std::cout << "\n*** GAME RESET ***\n";
	printStatus();
}

std::pair<int,int> screenToPos(int mouseX,int mouseY) {
	if (mouseY >= BOARD_SIZE * SQ) return { -1, -1 };
	int col = mouseX / SQ,row = mouseY / SQ;
	if (inBounds(row,col)) return { row, col };
	return { -1, -1 };
}

void saveBoard(const std::string& filename) {
	std::ofstream out(filename);
	if (!out.is_open()) {
		std::cerr << "Failed to save game!\n";
		return;
	}

	out << (whiteTurn ? 1 : 0) << "\n";
	out << (gameOver ? 1 : 0) << "\n";

	for (int row = 0; row < 8; row++) {
		for (int col = 0; col < 8; col++) {
			out << (int)chessBoard[row][col];
			if (col < 7) out << " ";
		}
		out << "\n";
	}

	out.close();
	std::cout << "*** Game saved to " << filename << " ***\n";
}

void loadBoard(const std::string& filename) {
	std::ifstream in(filename);
	if (!in.is_open()) {
		std::cerr << "Failed to load game! File not found.\n";
		return;
	}

	int turn;
	in >> turn;
	whiteTurn = (turn == 1);

	int gameOverInt;
	in >> gameOverInt;
	gameOver = (gameOverInt == 1);

	for (int row = 0; row < 8; row++) {
		for (int col = 0; col < 8; col++) {
			int p;
			in >> p;
			chessBoard[row][col] = (Piece)p;
		}
	}

	in.close();

	hasSelectedPos = false;
	legalMoves.clear();

	if (!gameOver) {
		bool inChk = inCheck(whiteTurn);
		if (inChk) statusText = whiteTurn ? "White in check!" : "Black in check!";
		else statusText = whiteTurn ? "White to move" : "Black to move";
	}

	std::cout << "*** Game loaded from " << filename << " ***\n";
	printStatus();
}


sf::Texture pieceTextures[13];

const char* textureFiles[13] = {
	"", 
	"assets/white-pawn.png",
	"assets/white-knight.png",
	"assets/white-bishop.png",
	"assets/white-rook.png",
	"assets/white-queen.png",
	"assets/white-king.png",
	"assets/black-pawn.png",
	"assets/black-knight.png",
	"assets/black-bishop.png",
	"assets/black-rook.png",
	"assets/black-queen.png",
	"assets/black-king.png"
};

int main() {
	initBoard();

	sf::RenderWindow window(sf::VideoMode({ (unsigned)winW, (unsigned)winH }),"Chess - Proper Rules");
	window.setFramerateLimit(60);

	//  textures 
	bool spritesLoaded = true;
	for (int p = 1; p <= 12; p++) {
		if (!pieceTextures[p].loadFromFile(textureFiles[p])) {
			std::cerr << "Failed to load: " << textureFiles[p] << std::endl;
			spritesLoaded = false;
		}
	}

	//   console controls
	std::cout << "\n========================================\n";
	std::cout << "        CHESS - CONTROLS\n";
	std::cout << "========================================\n";
	std::cout << "R         - Reset game\n";
	std::cout << "ESC       - Deselect piece\n";
	std::cout << "S         - Save game\n";
	std::cout << "L         - Load game\n";
	std::cout << "Left Click  - Select/Move piece\n";
	std::cout << "Right Click - Deselect piece\n";
	std::cout << "========================================\n\n";
	printStatus();

	while (window.isOpen()) {
		while (auto event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>()) window.close();

			if (auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
				if (keyPressed->code == sf::Keyboard::Key::R) resetGame();
				if (keyPressed->code == sf::Keyboard::Key::Escape) {
					hasSelectedPos = false;
					legalMoves.clear();
					std::cout << "Selection cleared.\n";
				}
				if (keyPressed->code == sf::Keyboard::Key::S) saveBoard("chess_save.txt");
				if (keyPressed->code == sf::Keyboard::Key::L) loadBoard("chess_save.txt");
			}

			if (!gameOver && event->is<sf::Event::MouseButtonPressed>()) {
				auto* mouseBtn = event->getIf<sf::Event::MouseButtonPressed>();

				if (mouseBtn->button == sf::Mouse::Button::Right) {
					hasSelectedPos = false;
					legalMoves.clear();
					std::cout << "Selection cleared.\n";
				}
				else if (mouseBtn->button == sf::Mouse::Button::Left) {
					std::pair<int,int> pos = screenToPos(mouseBtn->position.x,mouseBtn->position.y);
					if (pos.first != -1) {
						if (!hasSelectedPos) {
							if (isColor(chessBoard[pos.first][pos.second],whiteTurn)) {
								selectedPos = pos;
								hasSelectedPos = true;
								legalMoves = getLegalMoves(pos);
								std::cout << "Piece selected at (" << pos.first << ", " << pos.second << ")\n";
							}
						}
						else {
							auto it = std::find(legalMoves.begin(),legalMoves.end(),pos);
							if (it != legalMoves.end()) {
								std::cout << "Move: (" << selectedPos.first << ", " << selectedPos.second
									<< ") -> (" << pos.first << ", " << pos.second << ")\n";
								makeMove(selectedPos,pos);
							}
							hasSelectedPos = false;
							legalMoves.clear();
						}
					}
				}
			}
		}

		window.clear(sf::Color(50,50,50));

		// board
		for (int row = 0; row < 8; row++) {
			for (int col = 0; col < 8; col++) {
				sf::RectangleShape sq({ (float)SQ,(float)SQ });
				sq.setPosition({ (float)(col * SQ), (float)(row * SQ) });
				sq.setFillColor((row + col) % 2 ? sf::Color(180,140,100) : sf::Color(240,220,180));
				window.draw(sq);
			}
		}

		// Check
		if (!gameOver && inCheck(whiteTurn)) {
			std::pair<int,int> kingPos = locateKing(whiteTurn);
			if (kingPos.first != -1) {
				sf::RectangleShape check({ (float)SQ,(float)SQ });
				check.setPosition({ (float)(kingPos.second * SQ), (float)(kingPos.first * SQ) });
				check.setFillColor(sf::Color(255,0,0,100));
				window.draw(check);
			}
		}

		// Select Square
		if (hasSelectedPos) {
			sf::RectangleShape h({ (float)SQ,(float)SQ });
			h.setPosition({ (float)(selectedPos.second * SQ), (float)(selectedPos.first * SQ) });
			h.setFillColor(sf::Color(255,255,0,100));
			window.draw(h);
		}

		//  legal move
		for (auto& m : legalMoves) {
			sf::CircleShape dot(8);
			dot.setPosition({ (float)(m.second * SQ + SQ / 2 - 8), (float)(m.first * SQ + SQ / 2 - 8) });
			dot.setFillColor(sf::Color(0,200,0,150));
			window.draw(dot);
		}

		// PIECES
		for (int row = 0; row < 8; row++) {
			for (int col = 0; col < 8; col++) {
				Piece piece = chessBoard[row][col];
				if (piece != EMPTY && spritesLoaded) {
					sf::Sprite sprite(pieceTextures[piece]);
					sf::Vector2u texSize = pieceTextures[piece].getSize();
					float scale = (float)SQ / std::max(texSize.x,texSize.y);
					sprite.setScale({ scale, scale });
					sprite.setPosition({ (float)(col * SQ), (float)(row * SQ) });
					window.draw(sprite);
				}
			}
		}

		window.display();
	}

	return 0;
}


















 //made by
// Haris Arfan (25L-0758), Muhammad Ahmad Manzoor (25L-0784) , Hassan Tahir (25L-0898)