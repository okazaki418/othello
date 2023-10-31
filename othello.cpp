// オセロのプログラム
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#define MASK_HORIZONTAL 0x7e7e7e7e7e7e7e7e
#define MASK_VERTICAL 0x00ffffffffffff00
#define MASK_DIAGONAL 0x007e7e7e7e7e7e00

using namespace std;

// 手番の列挙型
typedef enum _TEBAN : int {
  SENTE = -1,
  GOTE  =  1,
} TEBAN;

const uint64_t INITBLACK = 0;
const uint64_t INITWHITE = 0;

// BitBoard 黒番、白番の盤面状態と手番情報を保持
typedef struct _BitBoard {
  uint64_t black, white;
  TEBAN teban;
} BitBoard;

// Othelloクラス
class Othello {
 public:
  static void init(BitBoard *board, TEBAN teban);  //初期盤面
  static int loadfile(BitBoard *board);            // ファイル読み込み
  static void show(BitBoard *board);               //盤面表示
  static uint64_t reverse(uint64_t pos, BitBoard *board);  //裏返し処理
  static uint64_t canReverse(BitBoard *board);  //置ける場所を返す
};

// 初期化
void Othello::init(BitBoard *board, TEBAN teban) {
  board->black = INITBLACK;
  board->white = INITWHITE;
  board->teban = teban;
}

// ファイル読み込み
int Othello::loadfile(BitBoard *board) {
  ifstream ifs("./board.txt");  // ./board.txtを読み込み
  string buf, str;

  if (ifs.fail()) {  // ファイルが存在しない場合
    cerr << "Failed to open file." << endl;

    // 一般的な初期位置に石を配置
    board->black = 0x810000000;
    board->white = 0x1008000000;

    return 1;
  } else {
    // bufに文字列を一列ずつ読ませ、strに足していくことで改行無視
    while (getline(ifs, buf)) {str += buf;}

    // ファイルから読み込んだ文字列をboardの中に入れる
    for (int i=0; i<64; i++) {
      board->black <<= 1;
      board->white <<= 1;

      if (str[i] == 'b') {
        board->black += 1;
      } else if (str[i] == 'w') {
        board->white += 1;
      }
    }
  }
  return 0;
}

// 盤面表示
void Othello::show(BitBoard *board) {
  int i, j;

  for (i=0; i<8; i++) {
    //一列出力
    for (j=0; j<8; j++) {
      if (((uint64_t)1 << (63 - (i*8 + j)) & board->black) != 0) {
        cout << "○";
      } else if (((uint64_t)1 << (63 - (i*8 + j)) & board->white) != 0) {
        cout << "●";
      } else if (((uint64_t)1 << (63 - (i*8 + j)) &
                  Othello::canReverse(board)) != 0) {
        cout << "×";
      } else {
        cout << "－";
      }
    }
    cout << endl;
  }
  fflush(stdout);
}

// 与えられたposから裏返る石の座標を返す
uint64_t Othello::reverse(uint64_t pos, BitBoard *board) {
  int i;
  uint64_t revd_board = 0;
  uint64_t enemy, me, tmp = 0;

  // 手番判定
  if (board->teban == SENTE) {
    enemy = board->white;
    me = board->black;
  } else {
    enemy = board->black;
    me = board->white;
  }

  // 右
  tmp = 0;
  for (i=1; ((pos >> i) & MASK_HORIZONTAL & enemy) != 0; i++) {
    tmp = tmp | (pos >> i);
  }
  if (((pos >> i) & me) != 0) {
    revd_board = revd_board | tmp;
  }

  // 左
  tmp = 0;
  for (i=1; ((pos << i) & MASK_HORIZONTAL & enemy) != 0; i++) {
    tmp = tmp | (pos << i);
  }
  if (((pos << i) & me) != 0) {
    revd_board = revd_board | tmp;
  }

  // 上
  tmp = 0;
  for (i = 1; ((pos << (i*8)) & MASK_VERTICAL & enemy) != 0; i++) {
    tmp = tmp | (pos << (i*8));
  }
  if ((pos << (i*8) & me) != 0) {
    revd_board = revd_board | tmp;
  }

  // 下
  tmp = 0;
  for (i=1; ((pos >> (i*8)) & MASK_VERTICAL & enemy) != 0; i++) {
    tmp = tmp | (pos >> (i*8));
  }
  if ((pos >> (i*8) & me) != 0) {
    revd_board = revd_board | tmp;
  }

  // 右上
  tmp = 0;
  for (i=1; ((pos << (i*7)) & MASK_DIAGONAL & enemy) != 0; i++) {
    tmp = tmp | (pos << (i*7));
  }
  if ((pos << (i*7) & me) != 0) {
    revd_board = revd_board | tmp;
  }

  // 左上
  tmp = 0;
  for (i=1; ((pos << (i*9)) & MASK_DIAGONAL & enemy) != 0; i++) {
    tmp = tmp | (pos << (i*9));
  }
  if ((pos << (i*9) & me) != 0) {
    revd_board = revd_board | tmp;
  }

  // 右下
  tmp = 0;
  for (i=1; ((pos >> (i*9)) & MASK_DIAGONAL & enemy) != 0; i++) {
    tmp = tmp | (pos >> (i*9));
  }
  if ((pos >> (i*9) & me) != 0) {
    revd_board = revd_board | tmp;
  }

  // 左下
  tmp = 0;
  for (i=1; ((pos >> (i*7)) & MASK_DIAGONAL & enemy) != 0; i++) {
    tmp = tmp | (pos >> (i*7));
  }
  if ((pos >> (i*7) & me) != 0) {
    revd_board = revd_board | tmp;
  }

  return revd_board;
}

// 合法手を返す関数
uint64_t Othello::canReverse(BitBoard *board) {
  int i;
  uint64_t legalboard = 0, empty, temp;
  empty = ~(board->black | board->white);  // 空きマス

  for (i=0; i<64; i++) {
    temp = (empty & ((uint64_t)1 << i));  // マスが空いているか
    if (temp!=0) {                      // 空いている場合の処理

      if (reverse(temp,board)!=0) {   // そのマスに置いたとき裏返せるなら
        legalboard = legalboard | temp;  // 合法手に追加
      }
    }
  }
  return legalboard;
}

int main() {
  BitBoard board;

  string color;
  cout << "石の色は？" << endl;
  cout << "黒 → bを入力" << endl;
  cout << "白 → wを入力" << endl;

  while(1) {
    cin >> color;

    if (color == "b" || color == "B") {
      Othello::init(&board, SENTE);
      break;
    } else if (color == "w" || color == "W") {
      Othello::init(&board, GOTE);
      break;
    } else {
      cout << "wかbで入力してください" << endl;
    }
  }
  Othello::loadfile(&board);  // ファイルから文字列を読み込み
  Othello::show(&board);  // 置かれている石と次における場所を表示
                          //
  return 0;
}
