#include "VCFsolver.h"
#include <random>
#include <iostream>
#include <cassert>
using namespace std;

const Hash128 VCFsolver::zob_plaWhite = Hash128(0xb6f9e465597a77eeULL, 0xf1d583d960a4ce7fULL);
const Hash128 VCFsolver::zob_plaBlack = Hash128(0x853E097C279EBF4EULL, 0xE3153DEF9E14A62CULL);
Hash128 VCFsolver::zob_board[2][sz][sz]; 
#ifdef FORGOMOCUP
VCFHashTable VCFsolver::hashtable(20,2);
#else
VCFHashTable VCFsolver::hashtable(26, 20);
#endif

uint64_t VCFsolver::MAXNODE = 300000;

uint64_t VCFsolver::totalAborted;
uint64_t VCFsolver::totalSolved;
uint64_t VCFsolver::totalnodenum;

inline bool resultNotSure(int32_t result)
{
  return result <= 0 && result != -10000;
}


void VCFsolver::init()
{
  totalSolved = 0;
  totalnodenum = 0;
  mt19937_64 rand(0);
  for (int i = 0; i < 2; i++)
    for (int j = 0; j < sz; j++)
      for (int k = 0; k < sz; k++)
        zob_board[i][j][k] = Hash128(rand(), rand());
}
uint32_t VCFsolver::findEmptyPos(int t, int y, int x)
{
  uint32_t pos = 0;
  switch (t)
  { 
    case 0:
    {
      for (int i = 0; i < 5; i++)
      {
        if (board[y][x+i] == 0)
        {
          pos = pos << 16;
          pos = pos | (sz*y + (x+i));
        }
      }
      return pos;
    }
    case 1:
    {
      for (int i = 0; i < 5; i++)
      {
        if (board[y+i][x] == 0)
        {
          pos = pos << 16;
          pos = pos | (sz*(y+i) + x );
        }
      }
      return pos;
    }
    case 2:
    {
      for (int i = 0; i < 5; i++)
      {
        if (board[y + i][x+i] == 0)
        {
          pos = pos << 16;
          pos = pos | (sz*(y + i) + (x+i));
        }
      }
      return pos;
    }
    case 3:
    {
      for (int i = 0; i < 5; i++)
      {
        if (board[y - i][x + i] == 0)
        {
          pos = pos << 16;
          pos = pos | (sz*(y - i) + (x + i));
        }
      }
      return pos;
    }
    default:return 0;

  }
}

uint32_t VCFsolver::findDefendPosOfFive(int y, int x)
{
  uint32_t emptypos = -1;
  auto check = [&](int t, int y, int x)
  {
    if (mystonecount[t][y][x] == 4 && oppstonecount[t][y][x]%6 == 0 )emptypos= findEmptyPos(t, y, x);
    
  };

  //x
  for (int i = 0; i < 5; i++)
  {
    int x1 = x - i;
    if (x1 < 0)break;
    if (x1 >= sz - 4)continue;
    check(0, y, x1);
    if (emptypos != -1)return emptypos;
  }
  //y
  for (int i = 0; i < 5; i++)
  {
    int y1 = y - i;
    if (y1 < 0)break;
    if (y1 >= sz - 4)continue;
    check(1, y1, x);
    if (emptypos != -1)return emptypos;
  }
  //+x+y
  for (int i = 0; i < 5; i++)
  {
    int x1 = x - i;
    int y1 = y - i;
    if (x1 < 0)break;
    if (x1 >= sz - 4)continue;
    if (y1 < 0)break;
    if (y1 >= sz - 4)continue;
    check(2, y1, x1);
    if (emptypos != -1)return emptypos;
  }
  //+x+y
  for (int i = 0; i < 5; i++)
  {
    int x1 = x - i;
    int y1 = y + i;
    if (x1 < 0)break;
    if (x1 >= sz - 4)continue;
    if (y1 >= sz)break;
    if (y1 < 4)continue;
    check(3, y1, x1);
    if (emptypos != -1)return emptypos;
  }
  return -1;
}

#if RULE==1||RULE==2
void VCFsolver::addNeighborSix(int y, int x, uint8_t pla,int factor)
{
  //cout << int(pla);
  if (pla == 0)return;
  auto stonecount = (pla == C_MY) ? mystonecount: oppstonecount;
  //8 directions
  int x1, y1,t;

  //+x
  t = 0;
  x1 = x + 1;
  y1 = y ;
  if (x1 < sz - 4)
    stonecount[t][y1][x1] +=factor;

  //-x
  t = 0;
  x1 = x - 5;
  y1 = y;
  if (x1 >= 0)
    stonecount[t][y1][x1] += factor;

  //+y
  t = 1;
  x1 = x;
  y1 = y+1;
  if (y1 < sz - 4)
    stonecount[t][y1][x1] += factor;

  //-y
  t = 1;
  x1 = x;
  y1 = y -5;
  if (y1>=0)
    stonecount[t][y1][x1] += factor;

  //+x+y
  t = 2;
  x1 = x+1;
  y1 = y + 1;
  if (x1 < sz - 4&& y1 < sz - 4)
    stonecount[t][y1][x1] += factor;

  //-x-y
  t = 2;
  x1 = x-5;
  y1 = y -5;
  if (x1 >= 0&& y1 >= 0)
    stonecount[t][y1][x1] += factor;

  //+x-y
  t = 3;
  x1 = x + 1;
  y1 = y - 1;
  if (x1 < sz - 4 && y1 >= 4)
    stonecount[t][y1][x1] += factor;

  //-x+y
  t = 3;
  x1 = x -5;
  y1 = y +5;
  if (x1>=0 && y1 <sz)
    stonecount[t][y1][x1] += factor;
}
#endif

void VCFsolver::solve(const Board& board, uint8_t pla, uint8_t& res, uint16_t& loc)
{
  int32_t result=setBoard(board,pla); 
  if (resultNotSure(result))
  {
    auto resultAndLoc = hashtable.get(boardhash);
    result = resultAndLoc & 0xFFFFFFFF;
    rootresultpos = resultAndLoc >> 32;
  }
  if(resultNotSure(result))result=solveIter(true);
  if (nodenum >= MAXNODE && result <= 0)res = 3;
  else if (result > 0)res = 1;
  else if (result == -10000)res = 2;
  else
  {
    cout << "No result!";
  }
  int x = rootresultpos % sz, y = rootresultpos / sz;
  loc = x + 1 + (y + 1) * (sz + 1);


  //some debug information
  totalnodenum += nodenum;
  totalSolved++;

  if (nodenum > 50000)
  {
  //  cout << "nodenum " << nodenum << "  res " << int(res) << endl;
 //   print();
  }
  if (nodenum >= MAXNODE)
  {
    totalAborted++;
//    cout << "Hit vcf upper bound: nodenum=" << nodenum << endl;
    //print();
  }
  if(totalSolved%100000==0)
    {
  //   cout << "  totalSolved " << totalSolved << "  totalNode " << totalnodenum <<  "  totalAborted " << totalAborted << endl;
    }
}

inline void printnum2(int n)
{
  char c1, c2;
  c1 = n / 10;
  c2 = n % 10;
  if (c1 == 0)cout << " ";
  else cout << char(c1 + '0');
  cout<< char(c2 + '0');
}


void VCFsolver::print()
{
  cout << "  ";
  for (int i = 0; i < sz; i++)printnum2(i);
  cout << endl;
  for (int y = 0; y < sz; y++)
  {
    printnum2(y);
    cout << " ";
    for (int x = 0; x < sz; x++)
    {
      auto c = board[y][x];
      if (c == 0)cout << ". ";
      else if (c == 1)cout << "x ";
      else if (c == 2)cout << "o ";
    }
    cout << endl;
  }
  cout << endl;
}
void VCFsolver::printRoot()
{
  cout << "  ";
  for (int i = 0; i < sz; i++)printnum2(i);
  cout << endl;
  for (int y = 0; y < sz; y++)
  {
    printnum2(y);
    cout << " ";
    for (int x = 0; x < sz; x++)
    {
      auto c = rootboard[y][x];
      if (c == 0)cout << ". ";
      else if (c == 1)cout << "x ";
      else if (c == 2)cout << "o ";
    }
    cout << endl;
  }
  cout << endl;
}

int32_t VCFsolver::setBoard(const Board& b, uint8_t pla)
{
#if RULE==2
  forbiddenSide = (pla == C_BLACK) ? C_MY : C_OPP;//����Լ��Ǻ�����Ϊ1������Ϊ2
#endif
  movenum = 0;
  bestmovenum = 10000;
  nodenum = 0;
  threeCount = 0;
  oppFourPos = -1;
  if (pla == C_WHITE)boardhash = zob_plaWhite;
  else boardhash = zob_plaBlack;

  int32_t result = 0;

  //clear
  for (int i = 0; i < 4; i++)
    for (int j = 0; j < sz; j++)
      for (int k = 0; k < sz; k++)
      {
        mystonecount[i][j][k] = 0;
        oppstonecount[i][j][k] = 0;
      }
  for (int y = 0; y < sz; y++)
    for (int x = 0; x < sz; x++)
    {
      short loc = (x + 1) + (y + 1)*(sz + 1);
      auto c = b.colors[loc];

      if (c == 0)board[y][x] = 0;
      else if (c == pla)
      {
        movenum++;
        board[y][x] = C_MY;
        boardhash ^= zob_board[0][y][x];
#if RULE==1
        addNeighborSix(y, x, C_MY, 6);
#endif
#if RULE==2
        if(forbiddenSide== C_MY)
          addNeighborSix(y, x, C_MY, 6);
#endif
      }
      else
      {
        board[y][x] = C_OPP;
        boardhash ^= zob_board[1][y][x];
#if RULE==1
        addNeighborSix(y, x, C_OPP, 6);
#endif
#if RULE==2
        if (forbiddenSide== C_OPP)
          addNeighborSix(y, x, C_OPP, 6);
#endif
      }
      rootboard[y][x] = board[y][x];
    }

  //count stone num

  //x
  for (int y = 0; y < sz; y++)
    for (int x = 0; x < sz-4; x++)
    {
      int mycount = 0, oppcount = 0;
      for (int i = 0; i < 5; i++)
      {
        auto c = board[y][x + i];
        if (c == C_MY)mycount++;
        else if (c == C_OPP)oppcount++;
      }
      mystonecount[0][y][x] += mycount;
      oppstonecount[0][y][x] += oppcount;
    }
  //y
  for (int x = 0; x < sz; x++)
    for (int y = 0; y < sz-4; y++)
    {
      int mycount = 0, oppcount = 0;
      for (int i = 0; i < 5; i++)
      {
        auto c = board[y+i][x];
        if (c == C_MY)mycount++;
        else if (c == C_OPP)oppcount++;
      }
      mystonecount[1][y][x] += mycount;
      oppstonecount[1][y][x] += oppcount;
    }
  //+x+y
  for (int x = 0; x < sz-4; x++)
    for (int y = 0; y < sz - 4; y++)
    {
      int mycount = 0, oppcount = 0;
      for (int i = 0; i < 5; i++)
      {
        auto c = board[y + i][x+i];
        if (c == C_MY)mycount++;
        else if (c == C_OPP)oppcount++;
      }
      mystonecount[2][y][x] += mycount;
      oppstonecount[2][y][x] += oppcount;
    }
  //+x-y
  for (int x = 0; x < sz - 4; x++)
    for (int y = 4; y < sz; y++)
    {
      int mycount = 0, oppcount = 0;
      for (int i = 0; i < 5; i++)
      {
        auto c = board[y - i][x + i];
        if (c == C_MY)mycount++;
        else if (c == C_OPP)oppcount++;
      }
      mystonecount[3][y][x] += mycount;
      oppstonecount[3][y][x] += oppcount;
    }

  

  for (int t = 0; t < 4; t++)
    for (int y = 0; y < sz; y++)
      for (int x = 0; x < sz; x++)
      {
        int my = mystonecount[t][y][x];
        int opp = oppstonecount[t][y][x];
        if (my == 5 || opp == 5)
        {
          cout << "Why you give a finished board here\n";
          print();
          return 0;
        }
        if (my == 4&& opp%6 == 0)
        {
          rootresultpos = findEmptyPos(t, y, x);
          result = 10000-1-movenum;
          return result;
        }
        if (my % 6 == 0&& opp ==4)
        {
          if(oppFourPos==-1)oppFourPos = findEmptyPos(t, y, x);
          else
          {
            auto anotherOppFourPos = findEmptyPos(t, y, x);
            if(anotherOppFourPos!= oppFourPos)result = -10000;//����˫���ˣ�������ֱ��return����Ϊ�Լ��п���ֱ������
          }
          continue;
        }
        if (my == 3 && opp%6 == 0)//����
        {
          uint64_t locs= findEmptyPos(t, y, x);
          uint64_t threeEntry = (uint64_t(uint64_t(t)*sz*sz + uint64_t(y) * sz + x) << 32) | locs;
          threes[threeCount] = threeEntry;
          threeCount++;
        }
      }
  return result;
}

int32_t VCFsolver::play(int x, int y, uint8_t pla, bool updateHash)
{
  board[y][x] = pla;
  if(updateHash)boardhash ^= zob_board[pla-1][y][x];


#if RULE==1
  addNeighborSix(y, x, pla, 6);
#endif
#if RULE==2
  bool isPlaForbidden = (forbiddenSide==pla);
  if(isPlaForbidden)
    addNeighborSix(y, x, pla, 6);
#endif

  int32_t result = 0;

  if (pla == C_MY)
  {

#if RULE==2
    //����Ҫ��������������Ϊ��ǰһ���Ѿ��жϳ���ʤ����
    //���˫�ĵķ��������������������㣬���ĵ��ҽ���������ľ���Ϊ[0,+-5],[+-5,0],[+-5,+-5]
    //���ĵ���һ��һ������ʤ����Ϊ����û���֡���Ȼ����������Ѿ��ų��˶�����ǰһ������
    //������ָ�������㣬һ����˫�Ľ���
    bool lifeFour = false;
    bool isForbidden = false;

    //�����оݣ���ͬһ������ͬʱ������������������������֮��Խ�������ж��Ƿ���֣����һ���߲�����1��2�����ĵ㶼���ǽ���˵���ǻ���
    int8_t threeCountDir[4] = {0, 0, 0, 0};//ÿ���������3����,���ڵ���2˵�������л���
   // int16_t maybeLifeFourPoses[4][3] ;//���п��ܵ���һ�ֵĻ��ĵ�
    //��һ��ά����4�����򣬵ڶ���ά�ȵĵ�һ�����ǻ��ĵ����1��2���ڶ��������ǻ��ĵ�

#endif
    movenum++;
    int32_t fourPos =-1;//��һ�����γɵĳ��ĵķ��ص�
    bool winThisMove = false;//�޽���˫��
  //  cout << "s\n";
    auto addandcheck = [&](int t, int y, int x)
    {
      mystonecount[t][y][x]++;
      auto msc = mystonecount[t][y][x];
   //   cout << int(msc) << endl;
#if RULE==2

      if (isPlaForbidden)
      {
        if (msc > 5 && msc % 6 == 5)isForbidden = true;//����
      }
#endif
      if (oppstonecount[t][y][x]%6 != 0|| msc <= 2 || msc>5)return;
      if (msc == 3)
      {
#if RULE==2

        if (isPlaForbidden)
        {
          threeCountDir[t]++;
        }
#endif
        uint64_t locs = findEmptyPos(t, y, x);
        uint64_t threeEntry = (uint64_t(uint64_t(t) * sz * sz + uint64_t(y) * sz + x) << 32) | locs;
        threes[threeCount] =threeEntry;
        threeCount++;
      }
      else if (msc == 4)
      {

        if (fourPos == -1)
        {
          fourPos = findEmptyPos(t, y, x);
          //todo white renju,check if black can't play at "fourPos" 
        }
        else
        {
          auto anotherFourPos = findEmptyPos(t, y, x);
          if (anotherFourPos != fourPos)
          {
            //����Ƿ��ǻ���
#if RULE==2
            if (isPlaForbidden)
            {
              if (lifeFour)isForbidden = true;
              else
              {
                int x1 = fourPos % sz, x2 = anotherFourPos % sz, y1 = fourPos / sz, y2 = anotherFourPos / sz;
                int dx = x1 - x2, dy = y1 - y2;
                if ((dx == 0 || dx == 5 || dx == -5) && (dy == 0 || dy == 5 || dy == -5))lifeFour = true;
                else isForbidden = true;
              }
           }

            if (!isForbidden)
#endif
            {
                winThisMove = true;//������޽���˫��
            }
          }
        }
      }
      else /*if (mystonecount[t][y][x] == 5)*/
      {
        cout << "how can you reach here 3" << endl;
        print();
        printRoot();
        result = 1;
      }
    };

    //x
    //todo standard renju: add 6 
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      if (x1 < 0)break;
      if (x1>=sz-4)continue;
      addandcheck(0, y, x1);
    }
    //y
    for (int i = 0; i < 5; i++)
    {
      int y1 = y - i;
      if (y1 < 0)break;
      if (y1 >= sz - 4)continue;
      addandcheck(1, y1, x);
    }
    //+x+y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      int y1 = y - i;
      if (x1 < 0)break;
      if (x1 >= sz - 4)continue;
      if (y1 < 0)break;
      if (y1 >= sz - 4)continue;
      addandcheck(2, y1, x1);
    }
    //+x+y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      int y1 = y + i;
      if (x1 < 0)break;
      if (x1 >= sz - 4)continue;
      if (y1 >= sz)break;
      if (y1 < 4)continue;
      addandcheck(3, y1, x1);
    }

#if RULE==2
    //todo renju:check if is forbidden
    //ֻ���33,��Ϊ 44 �����������
    if (isPlaForbidden&&(!isForbidden))
    {
      int maybeLife3 = 0;
      for (int i = 0; i < 4; i++)
      {
        if (threeCountDir[i] >= 2)maybeLife3++;
      }
      if (maybeLife3 >= 2)
      {
        int life3 = 0;
        for (int i = 0; i < 4; i++)
        {
          if (checkLife3(y,x,i))life3++;
        }
        if (life3 >= 2)isForbidden = true;
      }
    }
    winThisMove = winThisMove && (!isForbidden);

#endif
    if (winThisMove)
    {
      result = 10000 - movenum - 1;//˫�Ļ����
      if (bestmovenum > movenum + 1)
      {
        bestmovenum = movenum + 1;
      }
      else if (bestmovenum != movenum + 1)
      {
        cout << "how can you reach here 4\n";
      }
    }
    
    if (fourPos==-1)//�����岻�ǳ���,ֻ��������ʱ�����ֳ��Ĳſ��ܵ�����
    {
      if (oppFourPos == -1)
      {
        cout << "how can you reach here 1  " <<x<<" "<<y<< endl;
        print();
      }
      result = -10000;
    }
#if RULE==2
    if (isForbidden)
      result = -10000;
#endif
  }
  else if (pla == C_OPP)
  {
#if RULE==2
  //����Ҫ��������������Ϊ��ǰһ���Ѿ��жϳ���ʤ����
  //���˫�ĵķ��������������������㣬���ĵ��ҽ���������ľ���Ϊ[0,+-5],[+-5,0],[+-5,+-5]
  //���ĵ���һ��һ������ʤ����Ϊ����û���֡���Ȼ����������Ѿ��ų��˶�����ǰһ������
  //������ָ�������㣬һ����˫�Ľ���
  bool lifeFour = false;
  bool isForbidden = false;

  //�����оݣ���ͬһ������ͬʱ������������������������֮��Խ�������ж��Ƿ���֣����һ���߲�����1��2�����ĵ㶼���ǽ���˵���ǻ���
  int8_t threeCountDir[4] = { 0, 0, 0, 0 };//ÿ���������3����,���ڵ���2˵�������л���
 // int16_t maybeLifeFourPoses[4][3] ;//���п��ܵ���һ�ֵĻ��ĵ�
  //��һ��ά����4�����򣬵ڶ���ά�ȵĵ�һ�����ǻ��ĵ����1��2���ڶ��������ǻ��ĵ�

#endif
    //todo renju:check if is forbidden
    oppFourPos = -1;//���ֳ���ֱ�Ӽ�¼��oppFourPos��

    bool winThisMove = false;//�޽���˫��

    auto addandcheck = [&](int t, int y, int x)
    {
      oppstonecount[t][y][x]++;
      auto osc = oppstonecount[t][y][x];
#if RULE==2

      if (isPlaForbidden)
      {
        if (osc > 5 && osc % 6 == 5)isForbidden = true;//����
      }
#endif
      if (mystonecount[t][y][x]%6 != 0 || osc < 3 || osc >5)return;//����в
      if (osc == 3)
      {
#if RULE==2

        if (isPlaForbidden)
        {
          threeCountDir[t]++;
        }
#endif
      }
      else if (osc == 4)
      {
        if (oppFourPos == -1)oppFourPos = findEmptyPos(t, y, x);
        else
        {
          auto anotherFourPos = findEmptyPos(t, y, x);
          if (anotherFourPos != oppFourPos)
          {
            //����Ƿ��ǻ���
#if RULE==2
            if (isPlaForbidden)
            {
              if (lifeFour)isForbidden = true;
              else
              {
                int x1 = oppFourPos % sz, x2 = anotherFourPos % sz, y1 = oppFourPos / sz, y2 = anotherFourPos / sz;
                int dx = x1 - x2, dy = y1 - y2;
                if ((dx == 0 || dx == 5 || dx == -5) && (dy == 0 || dy == 5 || dy == -5))lifeFour = true;
                else isForbidden = true;
              }
            }

            if (!isForbidden)
#endif
            {
              winThisMove = true;//������޽���˫��
            }


          }
        }
      }
      else// if (oppstonecount[t][y][x] == 5)
      {
        cout << "how can you reach here 2" << endl;
        result = 2;
      }
    };

    //x
    //todo standard renju: add 6 
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      if (x1 < 0)break;
      if (x1 >= sz - 4)continue;
      addandcheck(0, y, x1);
    }
    //y
    for (int i = 0; i < 5; i++)
    {
      int y1 = y - i;
      if (y1 < 0)break;
      if (y1 >= sz - 4)continue;
      addandcheck(1, y1, x);
    }
    //+x+y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      int y1 = y - i;
      if (x1 < 0)break;
      if (y1 < 0)break;
      if (x1 >= sz - 4)continue;
      if (y1 >= sz - 4)continue;
      addandcheck(2, y1, x1);
    }
    //+x+y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      int y1 = y + i;
      if (x1 < 0)break;
      if (y1 >= sz)break;
      if (x1 >= sz - 4)continue;
      if (y1 < 4)continue;
      addandcheck(3, y1, x1);
    }

#if RULE==2
    //todo renju:check if is forbidden
    //ֻ���33,��Ϊ 44 �����������
    if (isPlaForbidden && (!isForbidden))
    {
      int maybeLife3 = 0;
      for (int i = 0; i < 4; i++)
      {
        if (threeCountDir[i] >= 2)maybeLife3++;
      }
      if (maybeLife3 >= 2)
      {
        int life3 = 0;
        for (int i = 0; i < 4; i++)
        {
          if (checkLife3(y, x, i))life3++;
        }
        if (life3 >= 2)isForbidden = true;
      }
    }
    winThisMove = winThisMove && (!isForbidden);

#endif
    if (winThisMove)
    {
      result = -10000;
    }

#if RULE==2
    if (isForbidden)//ץ���ɹ�
    {
      result = 10000 - movenum - 1;//˫�Ļ����
      if (bestmovenum > movenum + 1)
      {
        bestmovenum = movenum + 1;
      }
      else if (bestmovenum != movenum + 1)
      {
        cout << "how can you reach here 5\n";
      }
    }
#endif
  }
  return result;
}

void VCFsolver::undo(int x, int y, int64_t oppFourPos1, uint64_t threeCount1, bool updateHash)
{
  threeCount = threeCount1;
  oppFourPos = oppFourPos1;//������û��Ҫ������������ԭ���޸ģ���������
  //result = 0;
  auto pla = board[y][x];
  board[y][x] = 0;
#if RULE==1
  addNeighborSix(y, x, pla, -6);
#endif
#if RULE==2
  if (forbiddenSide ==pla )
    addNeighborSix(y, x, pla, -6);
#endif
  if (updateHash) boardhash ^= zob_board[pla - 1][y][x];

  //��ط����븴�úܲ�����Ҫ�ģ���Ҫ��һ���
  if (pla == C_MY)
  {
    movenum--;
    //x
    //todo standard renju:-6 
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      if (x1 < 0)break;
      if (x1 >= sz - 4)continue;
      mystonecount[0][y][x1]--;
    }
    //y
    for (int i = 0; i < 5; i++)
    {
      int y1 = y - i;
      if (y1 < 0)break;
      if (y1 >= sz - 4)continue;
      mystonecount[1][y1][x]--;
    }
    //+x+y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      int y1 = y - i;
      if (x1 < 0)break;
      if (y1 < 0)break;
      if (x1 >= sz - 4)continue;
      if (y1 >= sz - 4)continue;
      mystonecount[2][y1][x1]--;
    }
    //+x+y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      int y1 = y + i;
      if (x1 < 0)break;
      if (y1 >= sz)break;
      if (x1 >= sz - 4)continue;
      if (y1 < 4)continue;
      mystonecount[3][y1][x1]--;
    }
  }
  else if (pla == C_OPP)
  {
    //x
    //todo standard renju:-6 
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      if (x1 < 0)break;
      if (x1 >= sz - 4)continue;
      oppstonecount[0][y][x1]--;
    }
    //y
    for (int i = 0; i < 5; i++)
    {
      int y1 = y - i;
      if (y1 < 0)break;
      if (y1 >= sz - 4)continue;
      oppstonecount[1][y1][x]--;
    }
    //+x+y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      int y1 = y - i;
      if (x1 < 0)break;
      if (y1 < 0)break;
      if (x1 >= sz - 4)continue;
      if (y1 >= sz - 4)continue;
      oppstonecount[2][y1][x1]--;
    }
    //+x+y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      int y1 = y + i;
      if (x1 < 0)break;
      if (y1 >= sz)break;
      if (x1 >= sz - 4)continue;
      if (y1 < 4)continue;
      oppstonecount[3][y1][x1]--;
    }
  }
  else
  {
    cout << "bug";
  }
}


int32_t VCFsolver::solveIter(bool isRoot)
{
  //��hash��������֮�����stonecount֮ǰ����������
  nodenum++;
  if (nodenum>= MAXNODE)
  {
    return -(movenum+1);
  }

  int32_t maxMovenum = bestmovenum - 1;
  if (movenum + 2 > maxMovenum)return -(movenum + 2);//��һ�ֿ϶��޷����壬������movenum+2��

  //�ȱ���
  auto oppFourPos_old = oppFourPos;
  auto threeCount_old = threeCount;

  //�ȿ���������һ���ǲ��ǳ���
  if (oppFourPos != -1)
  {
    int solutionPos = -1;
    int x = oppFourPos % sz, y = oppFourPos / sz;
    int32_t result=play(x,y, C_MY,true);
    if (resultNotSure(result) )//˵���Լ��ķ���Ҳ�ǳ���,���Լ������㡣����˫�ģ��������ٻ���Ҫmovenum+2��
    {
        auto defendPos = findDefendPosOfFive(y, x);
        result = play(defendPos % sz, defendPos / sz, C_OPP, true);
        if (result == 0)result = solveIter(false);
        undo(defendPos % sz, defendPos / sz, oppFourPos_old, threeCount_old, true);
    }
    undo(x, y, oppFourPos_old, threeCount_old, true);
    //if (resultNotSure(result))cout << "bug: no result";
    if (result >0)//vcf�ɹ�
    {
      solutionPos = oppFourPos;
      if (isRoot)rootresultpos = oppFourPos;
    }

    //��hash��
    hashtable.set(boardhash, (int64_t(solutionPos) << 32) | int64_t(result));

    return result;
  }

  //�����Ƕ���û�г��ĵ����
  int solutionPos = -1;
  int bestresult = -10000;
  for (int threeID = threeCount - 1; threeID >= 0; threeID--)
  {

    auto threeEntry = threes[threeID];
    uint16_t pos1 = threeEntry & 0xFF, pos2 = (threeEntry >> 16) & 0xFF;
    threeEntry = threeEntry >> 32;
    int t = threeEntry/(sz*sz);
    threeEntry = threeEntry % (sz * sz);
    int y = threeEntry / sz;
    int x = threeEntry % sz;

    if (oppstonecount[t][y][x]%6 != 0 || mystonecount[t][y][x] != 3)continue;//��������Ѿ�ʧЧ

    auto playandcalculate = [&](uint16_t posMy, uint16_t posOpp)
    {
      uint8_t x1 = posMy % sz, y1 = posMy / sz, x2 = posOpp % sz, y2 = posOpp / sz;
      auto hashchange = zob_board[0][y1][x1] ^ zob_board[1][y2][x2];
      boardhash ^= hashchange;

      //�޽��������hash�����н������Ӻ�飬Ϊ�˱����½���
#if RULE!=2
      auto resultAndLoc = hashtable.get(boardhash);
      int32_t result = resultAndLoc & 0xFFFFFFFF;
      bool shouldCalculate = (result <= 0 && result != 10000 && -result <= maxMovenum);
#else 
      int32_t result = 0;
      bool shouldCalculate = true;
#endif


      if (shouldCalculate)
      {
        result = play(x1, y1, C_MY,false);

        if (resultNotSure(result))
        {
            result = play(x2, y2, C_OPP, false);
            if (resultNotSure(result))
            {
#if RULE==2
              auto resultAndLoc = hashtable.get(boardhash);
              result = resultAndLoc & 0xFFFFFFFF;
              if (resultNotSure(result))
#endif
                result = solveIter(false);
            }
            undo(x2, y2, oppFourPos_old, threeCount_old, false);
          
        }
        undo(x1, y1, oppFourPos_old, threeCount_old, false);
        if (result == 0)cout << "Result=0";
      }
      boardhash ^= hashchange;
      if (result > bestresult)
      {
        bestresult = result;
        solutionPos = posMy;
      }
    };

    playandcalculate(pos1, pos2);
    if (bestresult >= 10000 - movenum - 2)break;//����Ѿ�����˫�ģ������迼�������߷�
    playandcalculate(pos2, pos1);
    if (bestresult >= 10000 - movenum - 2)break;//����Ѿ�����˫�ģ������迼�������߷�
  }
  hashtable.set(boardhash, (int64_t(solutionPos) << 32) | int64_t(bestresult));
  if (isRoot)rootresultpos = solutionPos;
  //cout << isRoot << " " << int(solutionPos) << endl;
  return bestresult;

}

#if RULE==2
bool VCFsolver::isForbiddenMove(int y, int x,bool fiveForbidden)//������
{
  if (board[y][x] != 0)return false;
  auto p = forbiddenSide;
  auto bstonecount = (p == C_MY) ? mystonecount : oppstonecount;
  auto wstonecount = (p == C_MY) ?oppstonecount: mystonecount ;
  board[y][x] = p;
  addNeighborSix(y, x, p, 6);

  bool isForbidden = false;

  bool five = false;
  bool lifeFour = false;

    //�����оݣ���ͬһ������ͬʱ������������������������֮��Խ�������ж��Ƿ���֣����һ���߲�����1��2�����ĵ㶼���ǽ���˵���ǻ���
    int8_t threeCountDir[4] = { 0, 0, 0, 0 };//ÿ���������3����,���ڵ���2˵�������л���

    int32_t fourPos = -1;//��һ�����γɵĳ��ĵķ��ص�
    auto addandcheck = [&](int t, int y, int x)
    {
      bstonecount[t][y][x]++;
      auto bsc = bstonecount[t][y][x];
     // cout <<int(bsc)<<" ";
        if (bsc > 5 && bsc % 6 == 5)isForbidden = true;//����
      if (wstonecount[t][y][x] % 6 != 0 || bsc <= 2 || bsc > 5)return;
      if (bsc == 3)
      {
          threeCountDir[t]++;
      }
      else if (bsc == 4)
      {
        if (fourPos == -1)
        {
          fourPos = findEmptyPos(t, y, x);
        }
        else
        {
          auto anotherFourPos = findEmptyPos(t, y, x);
          if (anotherFourPos != fourPos)
          {
            //����Ƿ��ǻ���
              if (lifeFour)isForbidden = true;
              else
              {
                int x1 = fourPos % sz, x2 = anotherFourPos % sz, y1 = fourPos / sz, y2 = anotherFourPos / sz;
                int dx = x1 - x2, dy = y1 - y2;
                if ((dx == 0 || dx == 5 || dx == -5) && (dy == 0 || dy == 5 || dy == -5))lifeFour = true;
                else isForbidden = true;
              }
            }

        }
      }
      else /*if (mystonecount[t][y][x] == 5)*/
      {
        five = true;
       // cout << x<<y<<"how can you reach here 6" << endl;
      //  print();
      }
    };

    //x
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      if (x1 < 0)break;
      if (x1 >= sz - 4)continue;
      addandcheck(0, y, x1);
    }
    //y
    for (int i = 0; i < 5; i++)
    {
      int y1 = y - i;
      if (y1 < 0)break;
      if (y1 >= sz - 4)continue;
      addandcheck(1, y1, x);
    }
    //+x+y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      int y1 = y - i;
      if (x1 < 0)break;
      if (x1 >= sz - 4)continue;
      if (y1 < 0)break;
      if (y1 >= sz - 4)continue;
      addandcheck(2, y1, x1);
    }
    //+x+y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      int y1 = y + i;
      if (x1 < 0)break;
      if (x1 >= sz - 4)continue;
      if (y1 >= sz)break;
      if (y1 < 4)continue;
      addandcheck(3, y1, x1);
    }

    //ֻ���33,��Ϊ 44 �����������
    if (!isForbidden)
    {
      int maybeLife3 = 0;
      for (int i = 0; i < 4; i++)
      {
        if (threeCountDir[i] >= 2)maybeLife3++;
      }
      if (maybeLife3 >= 2)
      {
        int life3 = 0;
        for (int i = 0; i < 4; i++)
        {
          if (checkLife3(y, x, i))life3++;
        }
        if (life3 >= 2)isForbidden = true;
      }
    }

    //�ָ�ԭ��
    board[y][x] = 0;
    addNeighborSix(y, x, p, -6);
    auto subandcheck = [&](int t, int y, int x)
    {
      bstonecount[t][y][x]--;
    };

    //x
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      if (x1 < 0)break;
      if (x1 >= sz - 4)continue;
      subandcheck(0, y, x1);
    }
    //y
    for (int i = 0; i < 5; i++)
    {
      int y1 = y - i;
      if (y1 < 0)break;
      if (y1 >= sz - 4)continue;
      subandcheck(1, y1, x);
    }
    //+x+y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      int y1 = y - i;
      if (x1 < 0)break;
      if (x1 >= sz - 4)continue;
      if (y1 < 0)break;
      if (y1 >= sz - 4)continue;
      subandcheck(2, y1, x1);
    }
    //+x+y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      int y1 = y + i;
      if (x1 < 0)break;
      if (x1 >= sz - 4)continue;
      if (y1 >= sz)break;
      if (y1 < 4)continue;
      subandcheck(3, y1, x1);
    }


    if (five)isForbidden = fiveForbidden;
    //ע�������жϵ�ʱ��������������fiveForbidden=true�����嵱Ȼ���ǽ��֣������������ֻ���ڻ����ж��е��ã�������һ���ǻ��ģ��������ͬʱ���������ǻ��ģ������嵱�����ָպÿ����û��ĵ��жϱ����ȷ��


   // print();
  //  cout << y << " " << x << " " << isForbidden << endl;
      return isForbidden;


}
bool VCFsolver::checkLife3(int y, int x, int t)//����Ƿ��ǻ���
{
  //ǰ�����Ѿ����Ӳ���stonecount������
  auto p = forbiddenSide;
  auto stonecount = (p == C_MY) ? mystonecount : oppstonecount;
  auto ostonecount = (p == C_MY) ? oppstonecount:mystonecount  ;
  int16_t pos1 = -1, pos2 = -1;//�������ĵ㣬���ֻ��0������1������-1���

  int ct=0;//��������������������ct=1���޻��ģ�ct=2��1�����ĵ㣬ct=3���������ĵ�
  int ctstart=-1;//�ӵڼ�����Ԫ�鿪ʼ������������
  //����t��stonecount�һ��ĵ�

  switch (t)
  {

  case 0:
    //x
    for (int i = 0; i < 5; i++)
    {
      int y1 = y;
      int x1 = x - i;
      if (x1 < 0)break;
      if (x1 >= sz - 4)continue;
      //������θ���4��
      int sc = stonecount[t][y1][x1];
      int osc = ostonecount[t][y1][x1];
      if (sc == 3&&(osc%6==0))
      {
        ct++;
        if (ctstart == -1)
        {
          ctstart = i;
        }
        
      }
      else
      {
        if (ct >= 2)break;//�ҵ�������3��
        if (ct == 1)//������һ��3������
        {
          ct = 0;
          ctstart = -1;
        }
      }
    }
    if (ct == 2)//���������߸�һ������ס�Ļ���
    {
      //+xx+x+��+x+xx+��o+xxx++
      for (int i = 0; i < 4; i++)
      {
        int y1 = y ;
        int x1 = x - ctstart + i;
        if (board[y1][x1] == 0)
        {
          pos1 = y1 * sz + x1;
          break;
        }
      }
      if (pos1 == -1)
      {
        cout << "cant find 3\n";
        print();
      }
    }
    else if (ct == 3)//���ŵĻ���
    {
      //+xxx+
      pos1 = (y)*sz + (x - ctstart + 3);
      pos2 = (y)*sz + (x - ctstart -1);
    }


    break;
  case 1:
    //y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x;
      int y1 = y - i;
      if (y1 < 0)break;
      if (y1 >= sz - 4)continue;
      int sc = stonecount[t][y1][x1];
      int osc = ostonecount[t][y1][x1];
      if (sc == 3 && (osc % 6 == 0))
      {
        ct++;
        if (ctstart == -1)
        {
          ctstart = i;
        }

      }
      else
      {
        if (ct >= 2)break;//�ҵ�������3��
        if (ct == 1)//������һ��3������
        {
          ct = 0;
          ctstart = -1;
        }
      }
    }
    if (ct == 2)//���������߸�һ������ס�Ļ���
    {
      //+xx+x+��+x+xx+��o+xxx++
      for (int i = 0; i < 4; i++)
      {
        int y1 = y - ctstart + i;
        int x1 = x;
        if (board[y1][x1] == 0)
        {
          pos1 = y1 * sz + x1;
          break;
        }
      }
      if(pos1==-1)
      {
        cout << "cant find 3\n";
        print();
      }
    }
    else if (ct == 3)//���ŵĻ���
    {
      //+xxx+
      pos1 = (y - ctstart + 3)*sz + (x);
      pos2 = (y - ctstart - 1)*sz + (x );
    }

    break;
  case 2:
    //+x+y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      int y1 = y - i;
      if (x1 < 0)break;
      if (x1 >= sz - 4)continue;
      if (y1 < 0)break;
      if (y1 >= sz - 4)continue;
      int sc = stonecount[t][y1][x1];
      int osc = ostonecount[t][y1][x1];
      if (sc == 3 && (osc % 6 == 0))
      {
        ct++;
        if (ctstart == -1)
        {
          ctstart = i;
        }

      }
      else
      {
        if (ct >= 2)break;//�ҵ�������3��
        if (ct == 1)//������һ��3������
        {
          ct = 0;
          ctstart = -1;
        }
      }
    }
    if (ct == 2)//���������߸�һ������ס�Ļ���
    {
      //+xx+x+��+x+xx+��o+xxx++
      for (int i = 0; i < 4; i++)
      {
        int y1 = y - ctstart + i;
        int x1 = x - ctstart + i;
        if (board[y1][x1] == 0)
        {
          pos1 = y1 * sz + x1;
          break;
        }
      }
      if (pos1 == -1)
      {
        cout << "cant find 3\n";
        print();
      }
    }
    else if (ct == 3)//���ŵĻ���
    {
      //+xxx+
      pos1 = (y - ctstart + 3) * sz + (x - ctstart + 3);
      pos2 = (y - ctstart - 1) * sz + (x - ctstart - 1);
    }

    break;
  case 3:
    //+x+y
    for (int i = 0; i < 5; i++)
    {
      int x1 = x - i;
      int y1 = y + i;
      if (x1 < 0)break;
      if (x1 >= sz - 4)continue;
      if (y1 >= sz)break;
      if (y1 < 4)continue;
      int sc = stonecount[t][y1][x1];
      int osc = ostonecount[t][y1][x1];
      if (sc == 3 && (osc % 6 == 0))
      {
        ct++;
        if (ctstart == -1)
        {
          ctstart = i;
        }

      }
      else
      {
        if (ct >= 2)break;//�ҵ�������3��
        if (ct == 1)//������һ��3������
        {
          ct = 0;
          ctstart = -1;
        }
      }
    }
    if (ct == 2)//���������߸�һ������ס�Ļ���
    {
      //+xx+x+��+x+xx+��o+xxx++
      for (int i = 0; i < 4; i++)
      {
        int y1 = y + ctstart - i;
        int x1 = x - ctstart + i;
        if (board[y1][x1] == 0)
        {
          pos1 = y1 * sz + x1;
          break;
        }
      }
      if (pos1 == -1)
      {
        cout << "cant find 3\n";
        print();
      }
    }
    else if (ct == 3)//���ŵĻ���
    {
      //+xxx+
      pos1 = (y + ctstart - 3) * sz + (x - ctstart + 3);
      pos2 = (y + ctstart + 1) * sz + (x - ctstart - 1);
    }

  }
  

  if (pos1 != -1 && (!isForbiddenMove(pos1 / sz, pos1 % sz,true)))return true;
    if (pos2 != -1 && (!isForbiddenMove(pos2 / sz, pos2 % sz, true)))return true;

    return false;
}
void VCFsolver::printForbiddenMap()
{
  cout << "  ";
  for (int i = 0; i < sz; i++)printnum2(i);
  cout << endl;
  for (int y = 0; y < sz; y++)
  {
    printnum2(y);
    cout << " ";
    for (int x = 0; x < sz; x++)
    {
      auto c = board[y][x];
      if (isForbiddenMove(y, x))cout << "& ";
      else if (c == 0)cout << ". ";
      else if (c == 1)cout << "x ";
      else if (c == 2)cout << "o ";
    }
    cout << endl;
  }
  cout << endl;
}
#endif