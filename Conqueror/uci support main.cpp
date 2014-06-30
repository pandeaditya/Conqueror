/**
    This file is part of Conqueror Chess Engine.

    Conqueror Chess Engine is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Conqueror Chess Engine is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Conqueror Chess Engine.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
@author adityapande
*/

///main changes uci[5] -> uci[4]....

#include<bits/stdc++.h>
const int TIMES = 1000;
const int VERBOSE = 1;
#define CONV(a,b) (int)(a[b+1])*8+a[b]-489
const int NUM_TYPES = 5; //numtypes does not include king
const char EMPTY = '.';
const int INF = 1000000;
const int HINF = 500000;
const int DINF = 2000000;
const int QUIET_CUT = 10;

const int WINDOW = 33;
int MAX_DEPTH = 3;
const int MIN_DEPTH = 1;
const bool Very_Cautious = false;
const int KILLER_LEVELS = 1;
char* killers[100][KILLER_LEVELS];

int killer_count[100]={0};

const int NORM = 3;

int max_time = 300000;

int LIMIT = max_time/NORM;
int TimeTrouble = 100;

int king_end_game[]=
{-50,-40,-30,-20,-20,-30,-40,-50,
-30,-20,-10,  0,  0,-10,-20,-30,
-30,-10, 20, 30, 30, 20,-10,-30,
-30,-10, 30, 40, 40, 30,-10,-30,
-30,-10, 30, 40, 40, 30,-10,-30,
-30,-10, 20, 30, 30, 20,-10,-30,
-30,-30,  0,  0,  0,  0,-30,-30,
-50,-30,-30,-30,-30,-30,-30,-50};

int king_middle_game[]={
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-30,-40,-40,-50,-50,-40,-40,-30,
-20,-30,-30,-40,-40,-30,-30,-20,
-10,-20,-20,-20,-20,-20,-20,-10,
 20, 20,  0,  0,  0,  0, 20, 20,
 20, 30, 10,  0,  0, 10, 30, 20};

int rqnodes = 0;

/**
    IMPLEMENTED:
    hybrid board rep
    minimax search
    alphabeta pruning
    quiescence search
    static evaluation
    legal move generator
    iterative deepening
    MVV/LVA move reordering
    makemove
    unmakemove
*/





using namespace std;
typedef unsigned long long int LL;

string filename = "out.txt";
ofstream file (filename.c_str());
//5 jun TODO
//flag for all_child_gen, etc DONE MAYBE
//iterative deepening DONE
//move ordering with deepening DONE
//killer moves


//unmakemove TEST
//continue TEST unmakemove by replacing thinking->copy_from with unmake move whenever applicable (not in PROMOTION)


//call makemove and unmakemove

//QS

//REMOVE EVAL etc from Position class as they will only be needed in class Move

//Killer moves

//TODO test transposition in alphabeta depth = 0 and extend the same to all depths in both searches


//TODO TEST PROMOTION.....

//TODO try QS with null moves in 1. Q mode 2. Both Q and all branch mode
//TODO try removing check opponent from quiet search
//TODO maybe isCheck is wrong try working with pseudo legal moves
//TODO print all pos which evaluate above 150 or less than -150 in QS at depths 2,3,4...


//TODO
//9. taking input (make kind of a 2 player -no cpu- chess game) //also good for testing

//1. check all & and | usage for possible logical errors or precedence issues
//2. test castling
//3. check for proprer typecastings to LL whenever required
//4. restrict to only board based printing after sufficient testing


//7. check combos of || and && for possible precedence issues

//11. 3 rep draw
//12. rewrite and test check

//HELPER FUNCTIONS
void erase(vector<int> &c,int v){c.erase(remove(c.begin(),c.end(),v),c.end());}


int interceptions[64];

int square(int a){return (a*a);}

LL random_bit_string[13][64];
LL lrand()
{
    LL n=rand()%256;

    for(int i=0;i<7;i++)
    {
        n<<=8;
        n|=rand()%256;
    }
    return n;
}

template <class T>
inline void setBit(T &bitvector,int n,bool b)//tested OK
{
	if(b)
		bitvector = bitvector | ( ((T)1)<<n );
	else
		bitvector = bitvector & ~( ((T)1)<<n );
}
template <class T>
inline bool getBit(T bitvector,int n)//tested OK
{
	return (bitvector >> n)&( ((T)1) );
}

void append_vector_elements(vector<int> &dest,vector<int> &source)
{
	for(int i=0;i<source.size();i++)dest.push_back(source[i]);
}

enum color {BLACK, WHITE};
enum pieces {PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING};
char *piece_notation = "PRNBQK";
int PROMOTION = PAWN, promotion_pos=0;
char CAPTURE = EMPTY;
bool NULL_MOVE = false;//TODO
int VALUE[]={100,500,320,330,900,INF};

int MOB_OPEN_VALS[]={1,3,6,5,3,3};
int MOB_END_VALS[]={1,3,4,3,3,6};
int generated_ct[NUM_TYPES+1][2]={0};

enum branch_mode {NO_BRANCH, QUIESCENT, ALL_BRANCH};


int getValue(char c)
{
    for(int i=0;i<=NUM_TYPES;i++)if(piece_notation[i]==c)return VALUE[i];
    return 0;
}


class Position;
Position *thinking;
class Move;

struct Bundle{
	///Position *board;
	char*muci;
	bool exact,lower,upper,useful;
	int depth,Value;
	LL hash_val;
};

map<LL,Bundle> hash_table;

vector<int> dx[NUM_TYPES+1],dy[NUM_TYPES+1];

class Move{
    public:
    char uci[5],capture;
    int eval, enpassfile;
    int bits;//bool evaluated(1), game_ended(2), turn(3), castled[2](4,5),right_castle_possible[2](6,7),left_castle_possible[2](8,9),flag_all_move_gen(12), flag_last_move_is_en_pass(13);
    vector<Move*> children;
    Move* parent;


    void set_last_en(bool val){setBit(bits,12,val);}
    bool get_last_en(){return getBit(bits,12);}


    void delchild()
    {
        for(int i=0;i<children.size();i++){children[i]->delchild();delete children[i];}
        children.clear();//faster
        //children.shrink_to_fit();
        //children.swap(vector<Move*>());
        std::vector<Move*>(children).swap(children);
        //std::vector<Move*> tmp(children);    // copy elements into a temporary vector
        //children.swap(tmp);
        setBit(bits,11,0);
    }
    inline void set_all_move(bool val){setBit(bits,11,val);}
    inline bool all_move(){return getBit(bits,11);}
    inline void set_evaluated(bool val){setBit(bits,0,val);}
    inline void verdict(int value){eval=value;set_evaluated(true);set_game_ended(true);delchild();}
    Move(int flags,int enpass,int start,int end,int reorder = 0, char promotion=0,bool en_pass_played=0)
    {
        if(start==end){uci[0]=uci[1]=uci[2]=uci[3]='0';uci[4]=0;}
        else{
            uci[0]=(start&7)+'a';
            uci[1]=(start>>3)+'1';
            uci[2]=(end&7)+'a';
            uci[3]=(end>>3)+'1';
            uci[4]=promotion;}
        bits = flags;
        eval = reorder;
        set_game_ended(false);
        enpassfile = enpass;
        //delchild();
        capture = CAPTURE;
        set_evaluated(false);
        set_last_en(en_pass_played);
        set_all_move(false);
    }
    Move(Move &z)
    {
        for(int i=0;i<4;i++)uci[i]='0';
        uci[4]=0;
        bits=z.bits;
        eval=z.eval;
        set_game_ended(false);
        enpassfile = -1;
        capture = z.capture;
        set_evaluated(false);
        set_all_move(false);
        set_last_en(0);
        setBit(bits,4,!((z.bits>>4)&1));
        parent=&z;
    }
    bool equal(char *in)
    {
        for(int i=0;i<5;i++)if(tolower(in[i])!=tolower(uci[i]))return 0;
        return 1;
    }
    void print(){for(int j=0;j<5;j++)if(uci[j]){cout<<(char)tolower(uci[j]);file<<(char)tolower(uci[j]);}}
    inline bool turn(){return ((bits>>4)&1);}
    inline void set_turn(bool val){setBit(bits,4,val);}
    inline bool evaluated(){return (bits&1);}
    inline bool right_castle_possible(bool side){return ((bits>>(5+side))&1);}
    inline void set_right_castle_possible(bool val,bool side){setBit(bits,5+side,val);}
    inline bool left_castle_possible(bool side){return ((bits>>(7+side))&1);}
    inline void set_left_castle_possible(bool val,bool side){setBit(bits,7+side,val);}
    inline bool game_ended(){return ((bits>>1)&1);}
    inline void set_game_ended(bool val){setBit(bits,1,val);}
};
bool compare(Move *a,Move *b)
{
	return (a->eval)>(b->eval);
}
bool compare2(Move *a,Move *b)
{
	return (a->eval)<(b->eval);
}

int history[2][64][64]={0};

bool hist_turn;
int kill_helper;

Move *pv,*root;


int dist(Move *mv)
{
    int d=1;
    while(mv!=root && mv->parent!=NULL)
    {
        mv=mv->parent;
        d++;
    }
    if(mv!=root)d=25;
        return d;
}

bool histsort(Move *a,Move *b)
{
    if(a==pv)return 1;
    if(b==pv)return 0;
    int ax=(a->uci[4]>='A')||(a->capture>='A'), bx=(b->uci[4]>='A')||(b->capture>='A');
    if(ax && bx)return 0;
    if(ax)return 1;
    if(bx)return 0;
    return history[hist_turn][CONV(a->uci,0)][CONV(a->uci,2)]>history[hist_turn][CONV(a->uci,0)][CONV(b->uci,2)];
}

bool killsort(Move *a,Move *b)
{
    if(a==pv)return 1;
    if(b==pv)return 0;
    int ax=(a->uci[4]>='A')||(a->capture>='A'), bx=(b->uci[4]>='A')||(b->capture>='A');
    if(ax && bx)return 0;
    if(ax)return 1;

    if(bx)return 0;
    for(int i=0;i<killer_count[kill_helper];i++)
    {

        if(a->equal(killers[kill_helper][i])){ax=1;}
        if(b->equal(killers[kill_helper][i])){bx=1;}
    }
    return ax>bx;
}



/*bool capsort(Move *a,Move *b)
{
    //promotions and captures
    int ax=0,bx=0;
    ax = 1 + ((getValue(a->capture)*100)/100);//implement capturer
    bx = 1 + ((getValue(b->capture)*100)/100);//implement capturer
    ax+=getValue(a->uci[4]);
    bx+=getValue(b->uci[4]);
    return ax>bx;
}*/



class Position //Hybrid position model (bitvector + Vector coordinates ...)
{
	LL colorPiece[2];//bitvector 0..63 each of the bits represent if a white piece is there(0) or not
	LL pieceType[NUM_TYPES]; //not req for king, a int suffices for king
	vector<int> pieces[NUM_TYPES+1][2];
public:

	int reorder;
	static int object_count;
	/*bool evaluated,game_ended,castled[2],turn,right_castle_possible[2],left_castle_possible[2];*/
	int flags;
	int en_passant_file; //-1 when no opponent pawn moved double on last turn
	//int eval;//make it private after testing
	//Position *first_child;
	//Position *next_peer;//public only for testing purposes (TODO: make these ptrs private after the testing)
	static int get_object_count(){return object_count;}
	//functions
	//=======================
	//960 constructor
	inline bool turn(){return ((flags>>4)&1);}
	void set_turn(bool val){setBit(flags,4,val);}
	//bool evaluated(){return flags&1;}
	//void set_evaluated(bool val){setBit(flags,0,val);}
	bool right_castle_possible(bool side){return (flags>>(5+side))&1;}
	void set_right_castle_possible(bool val,bool side){setBit(flags,5+side,val);}
	bool left_castle_possible(bool side){return (flags>>(7+side))&1;}
	void set_left_castle_possible(bool val,bool side){setBit(flags,7+side,val);}
	//bool game_ended(){return (flags>>1)&1;}
	//void set_game_ended(bool val){setBit(flags,1,val);}
	/*void init()
	{
		reorder = 0;
		flags = 496;//=>turn = WHITE = 1 and all castling possible
		en_passant_file = -1;
		int i;
		for(i=0;i<64;i++)
		{
			setBit(colorPiece[WHITE],i,i<16);
			setBit(colorPiece[BLACK],i,i>=48);
		}
		for(i=0;i<NUM_TYPES;i++){pieceType[i] = 0;}
		//keeping the 16 pawns (PAWNKEEP) uncomment the following lines LATER
		for(i=8;i<16;i++)
		{
			setBit(pieceType[PAWN],i,true);
			setBit(pieceType[PAWN],63-i,true);
		}
		//now keeping the 14 pieces (not req to place king in this representation)
		//SUMMING TO 63 MEANS SYMMETRY ACROSS 'DIAGONALS KIND OF' BUT DOES NOT HOLD FOR QUEENS AND KING IN BEGINNING

		setBit(pieceType[ROOK],0,true);
		setBit(pieceType[ROOK],63,true);

		setBit(pieceType[ROOK],7,true);
		setBit(pieceType[ROOK],56,true);

		setBit(pieceType[KNIGHT],1,true);
		setBit(pieceType[KNIGHT],62,true);

		setBit(pieceType[KNIGHT],6,true);
		setBit(pieceType[KNIGHT],57,true);

		setBit(pieceType[BISHOP],2,true);
		setBit(pieceType[BISHOP],61,true);

		setBit(pieceType[BISHOP],5,true);
		setBit(pieceType[BISHOP],58,true);

		setBit(pieceType[QUEEN],3,true);
		setBit(pieceType[QUEEN],59,true);

		//now placing the 2 kings
		pieces[KING][BLACK].push_back(60);
		pieces[KING][WHITE].push_back(4);

		pieces[ROOK][WHITE].push_back(0);
		pieces[ROOK][WHITE].push_back(7);
		pieces[ROOK][BLACK].push_back(63);
		pieces[ROOK][BLACK].push_back(56);

		pieces[KNIGHT][WHITE].push_back(1);
		pieces[KNIGHT][WHITE].push_back(6);
		pieces[KNIGHT][BLACK].push_back(62);
		pieces[KNIGHT][BLACK].push_back(57);

		pieces[BISHOP][WHITE].push_back(2);
		pieces[BISHOP][WHITE].push_back(5);
		pieces[BISHOP][BLACK].push_back(61);
		pieces[BISHOP][BLACK].push_back(58);

		pieces[QUEEN][WHITE].push_back(3);
		pieces[QUEEN][BLACK].push_back(59);

		//(PAWNKEEP) uncomment
		for(i=8;i<16;i++)
		{
			pieces[PAWN][WHITE].push_back(i);
			pieces[PAWN][BLACK].push_back(63-i);
		}
		//(PAWNKEEP) comment

	}*/


	int identify(int pos)
	{
	    if(pieces[KING][0][0]==pos || pieces[KING][1][0]==pos)return KING;
	    for(int i=0;i<NUM_TYPES;i++)
        {
            if(getBit(pieceType[i],pos))return i;
        }
	    return -1;
	}

	LL hash()///TESTED OK
	{
	    LL h = 0;
        for(int i=0;i<2;i++)
            for(int j=0;j<NUM_TYPES;j++)
            for(int k=0;k<64;k++)
            if( getBit(colorPiece[i] & pieceType[j],k))
            h^=random_bit_string[i*5+j][k];
            ///h ^= (colorPiece[i] & pieceType[j]) & (random_bit_string[i*6+j]);
        h^=random_bit_string[10][pieces[KING][WHITE][0]];
        h^=random_bit_string[11][pieces[KING][BLACK][0]];
        int flag_mask = 0;
        flag_mask = ((1<<7)-1);
        LL tmp= ((flag_mask & (flags>>2)) | ( (en_passant_file+1)<<7 ) );
        for(int k=0;k<64;k++)if(getBit(tmp,k))h^=random_bit_string[12][k];
	    return h;
	}

	char symbol(int i){
		if(i==pieces[KING][WHITE][0])return 'K';
		if(i==pieces[KING][BLACK][0])return 'k';
		for(int j=0;j<NUM_TYPES;j++)if(getBit(pieceType[j],i)){
			return piece_notation[j] + 32*getBit(colorPiece[BLACK],i) ;
		}
		return EMPTY;
	}
	char symbol(int i,int j){return symbol((i<<3)+j);}
	void copy_from(Position &z)//copy function
	{
		if(this == &z){return;}//Invalid copy attempt as it should not change anything
		flags = z.flags;
		reorder = z.reorder;
		en_passant_file = z.en_passant_file;
		colorPiece[WHITE] = z.colorPiece[WHITE];
		colorPiece[BLACK] = z.colorPiece[BLACK];
		int i,j;
		for(i=0;i<NUM_TYPES;i++)pieceType[i]=z.pieceType[i];

		for(i=0;i<NUM_TYPES+1;i++)
		{
			pieces[i][WHITE].clear();
			pieces[i][BLACK].clear();
		}
		for(i=0;i<NUM_TYPES+1;i++)
		{
			for(j=0;j<z.pieces[i][WHITE].size();j++)pieces[i][WHITE].push_back(z.pieces[i][WHITE][j]);
			for(j=0;j<z.pieces[i][BLACK].size();j++)pieces[i][BLACK].push_back(z.pieces[i][BLACK][j]);

		}

	}

    void init(char *fen)
    {
        reorder = 0;
        en_passant_file = -1;
        int index = 56,i;
        flags = 0;
        char ch;
        for(i=0;i<NUM_TYPES;i++)pieceType[i] = 0;
        colorPiece[0]=colorPiece[1]=0;
        for(i=0;fen[i]&&(index>=0);i++)
        {
            if(fen[i]>'9')
            {
                ch = fen[i];
                bool color = ch<'a';
                if(ch>='a') ch-=32;
                setBit(colorPiece[color],index,true);
                for(int j=0;j<=NUM_TYPES;j++)
                {
                if(piece_notation[j]==ch){
                    if(j<NUM_TYPES)setBit(pieceType[j],index,true);
                    pieces[j][color].push_back(index);
                    break;
                    }
                }
                index++;
            }
            else if(fen[i]>'0'){
                index+=fen[i]-'0';
            }
            else index-=16;
        }



        set_turn(fen[i]=='w');
        i=i+2;

        if(fen[i]!='-')
        {
            for(;fen[i]>' ';i++)
            {
                if(fen[i]=='K'){set_right_castle_possible(1,WHITE);}
                else if(fen[i]=='k')set_right_castle_possible(1,BLACK);
                else if(fen[i]=='Q')set_left_castle_possible(1,WHITE);
                else if(fen[i]=='q')set_left_castle_possible(1,BLACK);
            }
        }
        else i++;

        if(fen[i+1]!='-')
        {
            en_passant_file = fen[i+1]-'a';
            //i+=4;
        }

        //else i+=3;
        ///leaving any more specifications...

    }

	Position()//default constructor
	{
		object_count++;
		//init();
        init("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	}
	Position(char *fen)
	{
	object_count++;
	init(fen);
    }
	Position(Position &z)//copy constructor
	{
		object_count++;
		copy_from(z);
	}



	void print()
	{
	    if(VERBOSE)
        {
			int i,j;
			for(i=7;i>=0;i--)
			{
				for(j=0;j<8;j++){cout<<' ';cout<<symbol(i,j);}
				cout<<endl;
			}
        }
	}

	int isCheckv2(bool side, LL &pinned) //side is irrespective of the turn so no turn should be in the code below
		//NOT TESTED
		//optimized code based on bitvectors and bitmagic
	{
		//TODO
		int ret = 0;//0 means no check, 1..64=>check from position, >64 =>double check
		pinned = (LL)0;
		if(abs((pieces[KING][WHITE][0]>>3)-(pieces[KING][BLACK][0]>>3))<=1 && abs((pieces[KING][WHITE][0]&7)-(pieces[KING][BLACK][0]&7))<=1 )ret = pieces[KING][!side][0];

		//pawn check
		if(side==WHITE){if(pieces[KING][WHITE][0]<48)
		{
		    LL opp_pawns = colorPiece[!side] & pieceType[PAWN];
			if((pieces[KING][WHITE][0]&7) > 0){if( getBit(opp_pawns, pieces[KING][WHITE][0]+7) )ret = pieces[KING][WHITE][0]+7 +1; }
			if((pieces[KING][WHITE][0]&7) < 7){if( getBit(opp_pawns, pieces[KING][WHITE][0]+9) )ret = pieces[KING][WHITE][0]+9 +1; }
		}
		}
		else if(pieces[KING][BLACK][0]>15)
		{
		    LL opp_pawns = colorPiece[!side] & pieceType[PAWN];
			if((pieces[KING][BLACK][0]&7) > 0){if( getBit(opp_pawns, pieces[KING][BLACK][0]-9) )ret = pieces[KING][BLACK][0]-9 +1; }
			if((pieces[KING][BLACK][0]&7) < 7){if( getBit(opp_pawns, pieces[KING][BLACK][0]-7) )ret = pieces[KING][BLACK][0]-7 +1; }
		}

		//knight check
		int i;
		for(i=0;i<pieces[KNIGHT][!side].size();i++)
		{
			int knight_pos = pieces[KNIGHT][!side][i];
			if(square((knight_pos>>3)-(pieces[KING][side][0]>>3))+square((knight_pos&7)-(pieces[KING][side][0]&7))==5)ret = knight_pos +1;
		}






		LL all_pieces = colorPiece[WHITE] | colorPiece[BLACK];
		LL st_sliders = colorPiece[!side] & (pieceType[ROOK] | pieceType[QUEEN]);

		int pinnable;

		//rook and queen
		//LEFT
		//if((pieces[KING][side][0]&7)>0)//for left to exist
		{
			for(i=pieces[KING][side][0]-1;(i&7)!=7;i--)//check
			{
				if( getBit(all_pieces, i) )
                {
                    if( getBit(st_sliders, i) )
                    {
                        if(ret) return 65;//double check
                        else ret = i +1;
                    }

                    if(getBit(colorPiece[side], i))
                    {
                        pinnable = i;
                        i--;///imp
                        for(;(i&7)!=7;i--)///imp
                        {
                            if( getBit(st_sliders, i) )///imp
                            {
                                setBit(pinned,pinnable,true);
                            }
                            else if( getBit(all_pieces, i) )
                            {
                                break;
                            }
                        }
                    }
                    break;
                }
			}


		}

		//RIGHT

		{
			for(i=pieces[KING][side][0]+1;(i&7)!=0;i++)//check
			{
				if( getBit(all_pieces, i) )
                {
                    if( getBit(st_sliders, i) )
                    {
                        if(ret) return 65;//double check
                        else ret = i +1;
                    }
                    if(getBit(colorPiece[side], i))
                    {
                        pinnable = i;
                        i++;///imp
                        for(;(i&7)!=0;i++)///imp
                        {
                            if( getBit(st_sliders, i) )///imp
                            {
                                setBit(pinned,pinnable,true);
                            }
                            else if( getBit(all_pieces, i) )
                            {
                                break;
                            }
                        }
                    }
                    break;
                }
			}
		}

		//UP

		for(i=pieces[KING][side][0]+8;i<64;i+=8)
		{
			if( getBit(all_pieces, i) )
                {
                    if( getBit(st_sliders, i) )
                    {
                        if(ret) return 65;//double check
                        else ret = i +1;
                    }
                    if(getBit(colorPiece[side], i))
                    {
                        pinnable = i;
                        i+=8;///imp
                        for(;i<64;i+=8)///imp
                        {
                            if( getBit(st_sliders, i) )///imp
                            {
                                setBit(pinned,pinnable,true);
                            }
                            else if( getBit(all_pieces, i) )
                            {
                                break;
                            }
                        }
                    }
                    break;
                }
		}

		//DOWN

		for(i=pieces[KING][side][0]-8;i>=0;i-=8)
		{
			if( getBit(all_pieces, i) )
                {
                    if( getBit(st_sliders, i) )
                    {
                        if(ret) return 65;//double check
                        else ret = i +1;
                    }
                    if(getBit(colorPiece[side], i))
                    {
                        pinnable = i;
                        i-=8;///imp
                        for(;i>=0;i-=8)///imp
                        {
                            if( getBit(st_sliders, i) )///imp
                            {
                                setBit(pinned,pinnable,true);
                            }
                            else if( getBit(all_pieces, i) )
                            {
                                break;
                            }
                        }
                    }
                    break;
                }
		}


		//bishop and queen
		LL diag_sliders = colorPiece[!side] & (pieceType[BISHOP] | pieceType[QUEEN]);
		//left up
		//if((pieces[KING][side][0]&7)>0)//for left to exist
		{
			for(i=pieces[KING][side][0]+7;(i&7)!=7 && i<64;i+=7)//check
			{
				if( getBit(all_pieces, i) )
                {
                    if(getBit(diag_sliders, i))
                    {
                        if(ret)return 65;
                        else ret = i+1;
                    }
                    if(getBit(colorPiece[side], i))
                    {
                        pinnable = i;
                        i+=7;///imp
                        for(;(i&7)!=7 && i<64;i+=7)///imp
                        {
                            if( getBit(diag_sliders, i) )///imp
                            {
                                setBit(pinned,pinnable,true);
                            }
                            else if( getBit(all_pieces, i) )
                            {
                                break;
                            }
                        }
                    }
                    break;
                }
			}
		}


		//left down

		{
			for(i=pieces[KING][side][0]-9;(i&7)!=7 && i>=0;i-=9)//check
			{
				if( getBit(all_pieces, i) )
                {
                    if(getBit(diag_sliders, i))
                    {
                        if(ret)return 65;
                        else ret = i+1;
                    }
                    if(getBit(colorPiece[side], i))
                    {
                        pinnable = i;
                        i-=9;///imp
                        for(;(i&7)!=7 && i>=0;i-=9)///imp
                        {
                            if( getBit(diag_sliders, i) )///imp
                            {
                                setBit(pinned,pinnable,true);
                            }
                            else if( getBit(all_pieces, i) )
                            {
                                break;
                            }
                        }
                    }
                    break;
                }
			}
		}



		//right down

		{
			for(i=pieces[KING][side][0]-7;(i&7)!=0 && i>=0;i-=7)//check
			{
				if( getBit(all_pieces, i) )
                {
                    if(getBit(diag_sliders, i))
                    {
                        if(ret)return 65;
                        else ret = i+1;
                    }
                    if(getBit(colorPiece[side], i))
                    {
                        pinnable = i;
                        i-=7;///imp
                        for(;(i&7)!=0 && i>=0;i-=7)///imp
                        {
                            if( getBit(diag_sliders, i) )///imp
                            {
                                setBit(pinned,pinnable,true);
                            }
                            else if( getBit(all_pieces, i) )
                            {
                                break;
                            }
                        }
                    }
                    break;
                }
			}
		}

		//right up
		{
			for(i=pieces[KING][side][0]+9;(i&7)!=0 && i<64;i+=9)//check
			{
				if( getBit(all_pieces, i) )
                {
                    if(getBit(diag_sliders, i))
                    {
                        if(ret)return 65;
                        else ret = i+1;
                    }
                    if(getBit(colorPiece[side], i))
                    {
                        pinnable = i;
                        i+=9;///imp
                        for(;(i&7)!=0 && i<64;i+=9)///imp
                        {
                            if( getBit(diag_sliders, i) )///imp
                            {
                                setBit(pinned,pinnable,true);
                            }
                            else if( getBit(all_pieces, i) )
                            {
                                break;
                            }
                        }
                    }
                    break;
                }
			}
		}

		return ret;
	}




	bool isCheck(bool side) //side is irrespective of the turn
		//NOT TESTED
		//optimized code based on bitvectors and bitmagic
	{
		//TODO
		if(abs((pieces[KING][WHITE][0]>>3)-(pieces[KING][BLACK][0]>>3))<=1 && abs((pieces[KING][WHITE][0]&7)-(pieces[KING][BLACK][0]&7))<=1 )return true;//if kings close to each other return true

		//pawn check
		if(side==WHITE){if(pieces[KING][WHITE][0]<48)
		{
		    LL opp_pawns = colorPiece[!side] & pieceType[PAWN];
			if((pieces[KING][WHITE][0]&7) > 0){if( getBit(opp_pawns,pieces[KING][WHITE][0]+7) )return true; }
			if((pieces[KING][WHITE][0]&7) < 7){if( getBit(opp_pawns,pieces[KING][WHITE][0]+9) )return true; }
		}}
		else if(pieces[KING][BLACK][0]>15)
		{
		    LL opp_pawns = colorPiece[!side] & pieceType[PAWN];
			if((pieces[KING][BLACK][0]&7) > 0){if( getBit(opp_pawns,pieces[KING][BLACK][0]-9) )return true; }
			if((pieces[KING][BLACK][0]&7) < 7){if( getBit(opp_pawns,pieces[KING][BLACK][0]-7)  )return true; }
		}

		//knight check

		int i;
		for(i=0;i<pieces[KNIGHT][!side].size();i++)
		{
			int knight_pos = pieces[KNIGHT][!side][i];
			if(square((knight_pos>>3)-(pieces[KING][side][0]>>3))+square((knight_pos&7)-(pieces[KING][side][0]&7))==5)return true;
		}

		//rook and queen
        LL all_pieces = colorPiece[WHITE] | colorPiece[BLACK];
        LL st_sliders = colorPiece[!side] & (pieceType[ROOK] | pieceType[QUEEN]);
		//LEFT
		//if((pieces[KING][side][0]&7)>0)//for left to exist
		{
			for(i=pieces[KING][side][0]-1;(i&7)!=7;i--)//check
			{
				if( getBit(all_pieces, i) )
                {
                    if(getBit(st_sliders, i))return true;
                    break;
                }
			}
		}

		//RIGHT
		//if((pieces[KING][side][0]&7)<7)//for right to exist
		{
			for(i=pieces[KING][side][0]+1;(i&7)!=0;i++)//check
			{
				if( getBit(all_pieces, i) )
                {
                    if(getBit(st_sliders, i))return true;
                    break;
                }
			}
		}

		//UP
		for(i=pieces[KING][side][0]+8;i<64;i+=8)
		{
			if( getBit(all_pieces, i) )
                {
                    if(getBit(st_sliders, i))return true;
                    break;
                }
		}


		//DOWN
		for(i=pieces[KING][side][0]-8;i>=0;i-=8)
		{
			if( getBit(all_pieces, i) )
                {
                    if(getBit(st_sliders, i))return true;
                    break;
                }
		}


		//bishop and queen
		LL diag_slider = colorPiece[!side] & (pieceType[BISHOP] | pieceType[QUEEN]);
		//left up
		//if((pieces[KING][side][0]&7)>0)//for left to exist
		{
			for(i=pieces[KING][side][0]+7;(i&7)!=7 && i<64;i+=7)//check
			{
				if( getBit(all_pieces, i) )
                {
                    if(getBit(diag_slider,i))return true;
                    break;
                }
			}
		}


		//left down
		//if((pieces[KING][side][0]&7)>0)//for left to exist
		{
			for(i=pieces[KING][side][0]-9;(i&7)!=7 && i>=0;i-=9)//check
			{
				if( getBit(all_pieces, i) )
                {
                    if(getBit(diag_slider,i))return true;
                    break;
                }
			}
		}



		//right down

		//if((pieces[KING][side][0]&7)<7)//for right to exist
		{
			for(i=pieces[KING][side][0]-7;(i&7)!=0 && i>=0;i-=7)//check
			{
				if( getBit(all_pieces, i) )
                {
                    if(getBit(diag_slider,i))return true;
                    break;
                }
			}
		}

		//right up
		//if((pieces[KING][side][0]&7)<7)//for right to exist
		{
			for(i=pieces[KING][side][0]+9;(i&7)!=0 && i<64;i+=9)//check
			{
				if( getBit(all_pieces, i) )
                {
                    if(getBit(diag_slider,i))return true;
                    break;
                }
			}
		}
		return false;
	}

	//parameterized contructor
	//pgn
	//fen etc
	//char [64]

	void add(int &generated, int &ct_quiet, Position * &prev_child, int pos, int npos, Move* parent, int MODE, int en_pass, bool checkable,int epp)//helper for a redundant task of adding children in move_gen()
	{
		//Make Move *now here itself
		//fun(generated&, prev_child )
		if(!checkable || !(*thinking).isCheck(turn()))
		{
			if(MODE==NO_BRANCH){ generated++; generated_ct[identify(pos)][turn()]++; (*thinking).copy_from(*this); return;}

			///if(flag_opponent_check)thinking->reorder += 150;///modified
			if(MODE==QUIESCENT){
				if( !(PROMOTION || (CAPTURE!=EMPTY) ) )
				{
				    bool flag_opponent_check = (*thinking).isCheck(!turn());
				    if(!flag_opponent_check)
                    {
                        ct_quiet++;(*thinking).copy_from(*this); return;}
                    }
                }
				prev_child = thinking;
				thinking->set_turn(!thinking->turn());
				if(en_pass){thinking->en_passant_file = en_pass-1;}
				else {thinking->en_passant_file = -1;}
				//TODO check update castling flags
				if(prev_child->pieces[KING][WHITE][0]!=4 ){prev_child->set_right_castle_possible(false,WHITE); prev_child->set_left_castle_possible(false,WHITE);}
				if(!getBit(prev_child->pieceType[ROOK] & prev_child->colorPiece[WHITE],0)){prev_child->set_left_castle_possible(false,WHITE);}
				if(!getBit(prev_child->pieceType[ROOK] & prev_child->colorPiece[WHITE],7)){prev_child->set_right_castle_possible(false,WHITE);}
				if(prev_child->pieces[KING][BLACK][0]!=60 ){prev_child->set_right_castle_possible(0,BLACK); prev_child->set_left_castle_possible(0,BLACK);}
				if(!getBit(prev_child->pieceType[ROOK] & prev_child->colorPiece[BLACK],56)){prev_child->set_left_castle_possible(0,BLACK);}
				if(!getBit(prev_child->pieceType[ROOK] & prev_child->colorPiece[BLACK],63)){prev_child->set_right_castle_possible(0,BLACK);}

				if(PROMOTION)
				{
					while(PROMOTION--){Move *now=new Move(thinking->flags,thinking->en_passant_file,pos,npos,thinking->reorder,piece_notation[PROMOTION+1]);now->delchild();parent->children.push_back(now);
					now->parent = parent;generated++; generated_ct[identify(pos)][turn()]++;now->set_game_ended(false);}
					(*thinking).copy_from(*this);
				}
				else{
					Move *now=new Move(thinking->flags,thinking->en_passant_file,pos,npos,thinking->reorder,0,epp);
					now->delchild();now->parent = parent;
					parent->children.push_back(now);
					generated++;generated_ct[identify(pos)][turn()]++;
					thinking->unmakemove(now);
					///thinking->copy_from(*this);
					reorder = 0;
					now->set_game_ended(false);
					//thinking->print();
					//(*thinking).copy_from(*this);
					//thinking->print();

				}
		}
		else {(*thinking).copy_from(*this);reorder=0;}
	}
	int move_gen(Move *parent, int MODE = ALL_BRANCH)
	{
		if(MODE==ALL_BRANCH && parent->all_move()){return parent->children.size();}
		parent->delchild();
		bool qm = MODE==QUIESCENT;
		LL pinned;
		int check = isCheckv2(turn(),pinned);

		if(qm && check)
		{
			MODE = ALL_BRANCH;
		}
		if(MODE == ALL_BRANCH)parent->set_all_move(true);
		else parent->set_all_move(false);

		if(parent->game_ended())return 0;

		if(check && check!=65)
        {

            for(int zx=0;zx<64;zx++)interceptions[zx]=0;
            //memset(interceptions,0,64);
            int cm = check-1;
            int delta = 1, diff = check-1-pieces[KING][turn()][0];
            for(int zx=9;zx>6;zx--)
            {
                if(diff%zx==0){delta=zx;break;}
            }
            if(diff<0)delta=-delta;
            for(int zx=pieces[KING][turn()][0];zx!=check-1;)
            {
                zx+=delta;
                interceptions[zx]=1;
            }
        }
		(*thinking).copy_from(*this);
		int generated = 0,ct_quiet = 0;
		Position *prev_child = NULL;
		//try pawn single forward moves and double pawn moves 'with no jump condition'
		int i, pos,npos,dPos=(thinking->turn())?8:-8, moveBy;

		if(check!=65)for(moveBy=1;moveBy<=2;moveBy++)
			for(i=0;i < thinking->pieces[PAWN][turn()].size();i++)
			{
				pos = thinking->pieces[PAWN][turn()][i];
				npos = pos+dPos*moveBy;

				if(check)if(!interceptions[npos])continue;


				bool condition[3]={0,npos <64 && npos >=0,
					((pos <=15 && turn()==WHITE)|| (pos >=48 && turn()==BLACK))
					&& !getBit(colorPiece[WHITE] | colorPiece[BLACK],pos+dPos) };
				thinking->reorder = 0;


				if(condition[moveBy] && !getBit(colorPiece[WHITE],npos) && !getBit(colorPiece[BLACK],npos))
				{
				    if(MODE == NO_BRANCH && check==0 && !getBit(pinned,pos) )
                    {
                        generated++;
                        generated_ct[identify(pos)][turn()]++;
                        continue;
                    }

                    bool checkable = !(check==0 && !getBit(pinned,pos) );
					thinking->pieces[PAWN][turn()][i] = npos;
					setBit(thinking->colorPiece[turn()],npos,true);
					setBit(thinking->colorPiece[turn()],pos,false);
					setBit(thinking->pieceType[PAWN],npos,true);
					setBit(thinking->pieceType[PAWN],pos,false);
					if(npos<8 || npos>=56){PROMOTION = QUEEN;promotion_pos=npos;}
					else PROMOTION = 0;
					//(*thinking).print();//for testing
					CAPTURE = EMPTY;
					add(generated,ct_quiet,prev_child,pos, npos, parent, MODE,(moveBy==2)*((pos&7)+1) ,checkable,0 );
				}
			}

			//en passant test
			//TODO



			if(thinking->en_passant_file!=-1 )
			{
				int ddx[2]={-1,1};
				int file = (turn()==WHITE) ? 4 : 3;
				for(int dxc = 0;dxc<2;dxc++)
					for(i=0;i < thinking->pieces[PAWN][turn()].size();i++)
					{
						pos = thinking->pieces[PAWN][turn()][i];
						int temp0 = (pos&7)+ddx[dxc];
						if(temp0>=0 && temp0<8 && (pos>>3)==file && ((pos+ddx[dxc])&7)==en_passant_file )//&&
						{
						    /*if(MODE == NO_BRANCH && check==0 && !getBit(pinned,pos) )
                            {
                                generated++;
                                generated_ct[identify(pos)][turn()]++;
                                continue;
                            }*/

                            bool checkable = !(check==0 && !getBit(pinned,pos) );

							npos = pos+dPos+ddx[dxc];

							if(check && check-1 != pos+ddx[dxc])continue;

							thinking->reorder = VALUE[PAWN];
							thinking->pieces[PAWN][turn()][i] = npos;
							setBit(thinking->colorPiece[turn()],npos,true);
							setBit(thinking->colorPiece[turn()],pos,false);
							setBit(thinking->pieceType[PAWN],npos,true);
							setBit(thinking->pieceType[PAWN],pos,false);

							//delete the pawn killed
							setBit(thinking->colorPiece[!turn()],pos+ddx[dxc],false);
							setBit(thinking->pieceType[PAWN],pos+ddx[dxc],false);

							erase(thinking->pieces[PAWN][!turn()],pos+ddx[dxc]);
							//(*thinking).print();//for testing
							CAPTURE = 'P';
							add(generated,ct_quiet,prev_child, pos,npos,parent, MODE,(moveBy==2)*((pos&7)+1), checkable,1 );
						}
					}


			}


			for(int picked_piece=(check==65)?KING:0;picked_piece<NUM_TYPES+1;picked_piece++)
				for(i=0;i < thinking->pieces[picked_piece][turn()].size();i++)
					for(int dir = 0;dir < dx[picked_piece].size(); dir++)
					{
						int maxMove = (picked_piece==KNIGHT || picked_piece==KING || (picked_piece ==PAWN) )?1:7;
						for(moveBy=1;moveBy<=maxMove;moveBy++)
						{

							pos = thinking->pieces[picked_piece][turn()][i];
							int temp0 = (pos&7)+moveBy*dx[picked_piece][dir];

							if(picked_piece!=PAWN)npos = pos+8*moveBy*dy[picked_piece][dir]+moveBy*dx[picked_piece][dir];//more generic than the idea used in check but somewhat slower
							else npos = pos+8*moveBy*(turn()==WHITE?1:-1)+moveBy*dx[picked_piece][dir];


							if((picked_piece!=PAWN || getBit(colorPiece[!turn()],npos))&&(npos>=0 && npos<64 && temp0 >=0 && temp0 < 8 && !getBit(colorPiece[turn()],npos)))
							{
							    if(MODE == NO_BRANCH && check==0 && !getBit(pinned,pos) && picked_piece!=KING && !getBit(thinking->colorPiece[!turn()],npos))
                                {
                                generated++;
                                generated_ct[identify(pos)][turn()]++;
                                continue;
                                }

                                if(getBit(colorPiece[!turn()],npos))maxMove=0;

                                if(check && picked_piece!=KING )if(!interceptions[npos] && !NULL_MOVE)
                                {

                                if(!Very_Cautious)continue; ///MAYBE BUGGY
                                }

                                bool checkable = !(check==0 && !getBit(pinned,pos) && picked_piece != KING);
								thinking->pieces[picked_piece][turn()][i] = npos;
								thinking->reorder = 0;
								CAPTURE = EMPTY;
								if(getBit(thinking->colorPiece[!turn()],npos))//capturing move
								{
									maxMove=0;//will make this iteration the last iteration in this direction
									if(NULL_MOVE == false)if(npos==pieces[KING][!turn()][0])
									{
										parent->verdict((turn()==WHITE)?INF:-INF);
										return 0;

									}
									for(int it = 0; it<NUM_TYPES;it++)
									{if(getBit(thinking->pieceType[it],npos)){
										erase(thinking->pieces[it][!turn()],npos);break;}
									}
									setBit(thinking->colorPiece[!turn()],npos,false);
									for(int y=0;y<NUM_TYPES;y++)if(getBit(thinking->pieceType[y],npos)){thinking->reorder = 30+(VALUE[y]*100)/VALUE[picked_piece];CAPTURE = piece_notation[y];}
									for(int y=0;y<NUM_TYPES;y++){setBit(thinking->pieceType[y],npos,false);}
								}
								setBit(thinking->colorPiece[turn()],npos,true);
								setBit(thinking->colorPiece[turn()],pos,false);
								if(picked_piece!=KING){
									setBit(thinking->pieceType[picked_piece],npos,true);
									setBit(thinking->pieceType[picked_piece],pos,false);
								}
								if(picked_piece==PAWN && (npos<8 || npos>=56)){PROMOTION = 4;promotion_pos=npos;}
								else PROMOTION = 0;
								add(generated, ct_quiet, prev_child, pos, npos, parent, MODE, 0 , checkable,0);

							}
							else{break;}
						}
					}
					//castling
					if(MODE!=QUIESCENT && !check)
					{
						LL isEmpty = colorPiece[WHITE] | colorPiece[BLACK];
						int adds = (turn()==BLACK)?56:0;
						bool left=left_castle_possible(turn()) && !getBit(isEmpty, 1+adds) && !getBit(isEmpty, 2+adds) && !getBit(isEmpty, 3+adds),
							right=right_castle_possible(turn()) && !getBit(isEmpty, 5+adds) && !getBit(isEmpty, 6+adds);
						if(left||right)
						{

							bool arr[2]={left,right};int diff[2]={-1,1};
							bool valid_move = false;
							for(int iter = 0;iter<2;iter++)
								if(arr[iter]){
									setBit(thinking->colorPiece[turn()],4+adds+diff[iter],true);
									setBit(thinking->colorPiece[turn()],4+adds,false);
									thinking->pieces[KING][turn()][0]+=diff[iter];
									thinking->reorder = 20;
									if(!thinking->isCheck(turn()))
									{
										setBit(thinking->colorPiece[turn()],4+adds+diff[iter],false);
										setBit(thinking->colorPiece[turn()],4+adds+2*diff[iter],true);
										thinking->pieces[KING][turn()][0]+=diff[iter];
										if(!thinking->isCheck(turn()))
										{
											valid_move = true;


											//move the rook now
											int rookPos = iter?0+adds:7+adds;
											npos = 4+adds-diff[iter];
											setBit(thinking->colorPiece[turn()],npos,true);
											setBit(thinking->colorPiece[turn()],rookPos,false);
											setBit(thinking->pieceType[ROOK],npos,true);
											setBit(thinking->pieceType[ROOK],rookPos,false);
											///setBit(thinking->flags,4+turn(),true);
											for(int h=0;h < thinking->pieces[ROOK][turn()].size();h++)if(thinking->pieces[ROOK][turn()][h]==rookPos)
											{
												thinking->pieces[ROOK][turn()][h]=npos;
												break;
											}
											PROMOTION = 0;;
											add(generated, ct_quiet, prev_child, 4+adds, 4+adds+2*diff[iter],parent,MODE, 0 ,true,0 );
										}

									}

								}

								if(!valid_move)
								{
									thinking->copy_from(*this);
								}
						}
					}

					if(!NULL_MOVE)
					{
						if(generated == 0 && MODE != QUIESCENT)//either stalemate or checkmate
						{
						    parent->verdict(check?(turn()==WHITE?-INF:INF):0);
							return 0;
						}
						if(! ( pieceType[PAWN] ||  pieceType[ROOK] || pieceType[QUEEN]))//insufficient mating material
						{


							if( !pieceType[BISHOP] && (pieces[KNIGHT][WHITE].size() + pieces[KNIGHT][BLACK].size()<=1)){parent->verdict(0);return 0;}
							else if( !pieceType[KNIGHT] && (pieces[BISHOP][WHITE].size() + pieces[BISHOP][BLACK].size()==1)){parent->verdict(0);return 0;}
						}
					}

					/*if(MODE == QUIESCENT && children.size())
					{
					thinking->copy_from(*this);
					thinking->reorder = 100;
					children.push_back(thinking);
					thinking = new Position(*this);
					}*/
					/*if(qm && parent->children.size())
					{
					Move nm(flags,en_passant_file,0,0,120);
					nm.set_turn(!turn());
					}*/

					//REORDER
					stable_sort(parent->children.begin(),parent->children.end(),compare);

					return generated;
	}//end of move_gen function


	int eval_weak_pawns(bool side)
	{
		LL bitvector = colorPiece[side] & pieceType[PAWN];
		int covered[10]={0},isolated = 0,doubled = pieces[PAWN][side].size();
		for(int i=0;i<64;i++)if(getBit(bitvector,i) && !covered[(i&7)+1]){covered[(i&7)+1]=1;doubled--;}
		for(int i=1;i<9;i++)if(covered[i] && !covered[i-1]&&!covered[i+1]){isolated++;}
		return -(7*doubled+2*isolated);
	}
	bool bishop_pair(bool side){return pieces[BISHOP][side].size()==2;}
	int king_safety(bool side)
	{
		int ret=0;
		int quad_file_left=0,quad_file_right=7,quad_rank_down=0,quad_rank_up=7,king_pos=pieces[KING][side][0];

		if((king_pos>>3)>3)quad_rank_down=4;
		else quad_rank_up=3;
		if((king_pos&7)>3)quad_file_left=4;
		else quad_file_right=3;
		for(int i=quad_rank_down;i<=quad_rank_up;i++)
			for(int j=quad_file_left;j<=quad_file_right;j++)
				ret+=((getBit(colorPiece[side],(i<<3)+j))?1:0) - ((getBit(colorPiece[!side],(i<<3)+j))?((getBit(pieceType[QUEEN],(i<<3)+j))?3:1):0);
		return ret>0?0:ret;
	}
	void makemove(Move *move)
	{

		flags = move->bits;
		en_passant_file = move->enpassfile;
		//null move
		if(move->uci[0]=='0')return;






		int pos=(move->uci[1]-'1')*8+(move->uci[0]-'a'),npos=(move->uci[3]-'1')*8+(move->uci[2]-'a');
        int turn = getBit(colorPiece[WHITE],pos)?WHITE:BLACK;
		///int turn = !move->turn() ;//! is imp
		//special cases

		//castle



		if( abs((pos&7)-(npos&7))>1 && pos==pieces[KING][turn][0])
		{

			pieces[KING][turn][0]=npos;
			int rpos = pos+(npos>pos?3:-4), rnpos = pos+(npos>pos?1:-1);
			for(int i=0;i<pieces[ROOK][turn].size();i++)if(pieces[ROOK][turn][i]==rpos){pieces[ROOK][turn][i]=rnpos;break;}
			setBit(colorPiece[turn],rnpos,true);
			setBit(colorPiece[turn],rpos,false);
			setBit(pieceType[ROOK],rnpos,true);
			setBit(pieceType[ROOK],rpos,false);

			setBit(colorPiece[turn],pos,false);
			setBit(colorPiece[turn],npos,true);


			return;
		}

		int mp = 0;
		for(;mp<NUM_TYPES;mp++)if(getBit(pieceType[mp],pos))break;


		//en passant
		setBit(colorPiece[turn],npos,true);
		setBit(colorPiece[turn],pos,false);
		if(mp!=KING)setBit(pieceType[mp],npos,true);
		if(mp!=KING)setBit(pieceType[mp],pos,false);


		for(int i=0;i<pieces[mp][turn].size();i++)if(pieces[mp][turn][i]==pos){pieces[mp][turn][i]=npos;break;}


		if(move->capture>='A'){int ctype = 0;
		for(;ctype<NUM_TYPES;ctype++)if(piece_notation[ctype]==move->capture)break;
		///int ef = move->parent->enpassfile;
		///bool en_pass = (npos&7)==ef && getBit(pieceType[PAWN],pos) && (npos>>3)==((turn==WHITE)?4:3);
		bool en_pass = move->get_last_en();
		int cpos = en_pass?((pos>>3)*8+(npos&7)):npos;
		setBit(colorPiece[!turn],cpos,false);
		if(ctype!=mp || cpos!=npos)
        setBit(pieceType[ctype],cpos,false);//MAY CAUSE EXIT IF ctype == king BUT IT SHOULD NOT HAPPEN
		erase(pieces[ctype][!turn],cpos);
		}



		//promotion
		if(move->uci[4]){
			erase(pieces[PAWN][turn],npos);
			int ptype = 0;

			for(;piece_notation[ptype]!=move->uci[4];ptype++);//golf replace != by -
			pieces[ptype][turn].push_back(npos);
			setBit(pieceType[PAWN],npos,false);
			setBit(pieceType[ptype],npos,true);
		}

	}
	void unmakemove(Move *move)
	{
		flags = move->parent->bits;
		bool en_pass = move->get_last_en();


        en_passant_file = move->parent->enpassfile;
		//null move
		if(move->uci[0]=='0')return;


		int npos=(move->uci[1]-'1')*8+(move->uci[0]-'a'),pos=(move->uci[3]-'1')*8+(move->uci[2]-'a');
		///int turn = !move->turn() ;//! is imp
		int turn = getBit(colorPiece[WHITE],pos)?WHITE:BLACK;
		//special cases
		//castle
		if( abs((pos&7)-(npos&7))>1 && pos==pieces[KING][turn][0])
		{
			pieces[KING][turn][0]=npos;
			int rnpos = npos+(npos<pos?3:-4), rpos = npos+(npos<pos?1:-1);
			for(int i=0;i<pieces[ROOK][turn].size();i++)if(pieces[ROOK][turn][i]==rpos){pieces[ROOK][turn][i]=rnpos;break;}
			setBit(colorPiece[turn],rnpos,true);
			setBit(colorPiece[turn],rpos,false);
			setBit(pieceType[ROOK],rnpos,true);
			setBit(pieceType[ROOK],rpos,false);

			setBit(colorPiece[turn],pos,false);
			setBit(colorPiece[turn],npos,true);

			return;
		}
		int mp = 0;
		for(;mp<NUM_TYPES;mp++)if(getBit(pieceType[mp],pos))break;

		//en passant
		setBit(colorPiece[turn],npos,true);
		setBit(colorPiece[turn],pos,false);
		if(mp!=KING)setBit(pieceType[mp],npos,true);
		if(mp!=KING)setBit(pieceType[mp],pos,false);

		for(int i=0;i<pieces[mp][turn].size();i++)if(pieces[mp][turn][i]==pos){pieces[mp][turn][i]=npos;break;}

		if(move->capture>='A'){int ctype = 0;
		for(;ctype<NUM_TYPES;ctype++)if(piece_notation[ctype]==move->capture)break;
		int cpos = en_pass?((npos>>3)*8+(pos&7)):pos;
		setBit(colorPiece[!turn],cpos,true);
		setBit(pieceType[ctype],cpos,true);//MAY CAUSE EXIT IF ctype == king BUT IT SHOULD NOT HAPPEN
		pieces[ctype][!turn].push_back(cpos);
		}


		//promotion
		if(move->uci[4]){
			pieces[PAWN][turn].push_back(npos);

			int ptype = 0;
			for(;piece_notation[ptype]!=move->uci[4];ptype++);//golf replace != by -
			erase(pieces[ptype][turn],npos);
			setBit(pieceType[PAWN],npos,true);
			setBit(pieceType[ptype],npos,false);
		}
	}

	void Play(Move *&move,char *play)
	{
		if(play[0]=='U' && move->parent!=NULL){unmakemove(move);move=move->parent;return;}
		if(!move->children.size())move_gen(move);
		bool match=false;
		int i;
		for(i=0;i<move->children.size();i++)if(move->children[i]->equal(play)){match=true;break;}
		if(i<move->children.size()){
			move = move->children[i];
			makemove(move);
		}
	}

	bool open_file(LL bv, int file)
	{
	    for(;file<64;file+=8)
        {
            if(getBit(bv,file))return 0;
        }
        return 1;
	}

	int rook_battery(bool side)
	{
	    if(pieces[ROOK][side].size()>1)
        {
            if( (pieces[ROOK][side][0]&7)==(pieces[ROOK][side][1]&7) || (pieces[ROOK][side][0]>>3)==(pieces[ROOK][side][1]>>3))return 15;
        }
        return 0;
	}

	bool same_diag(int a,int b)
	{
	    int ar=a&7,ac=a>>3,br=b&7,bc=b>>3;
	    if(ar+ac==br+bc)return 1;
	    if(ar-ac==br-bc)return 1;
	    return 0;
	}

	bool same_line(int a,int b)
	{
	    if( ((a&7)==(b&7))||( (a>>3)==(b>>3) ))return 1;
	    return 0;
	}

	int evaluate(Move *move)//white - black
	{

        NULL_MOVE=0;
		//TODO uncomment above lines after testing
		for(int i=0;i<=NUM_TYPES;i++)for(int j=0;j<2;j++)generated_ct[i][j]=0;

		int mobility = move_gen(move,NO_BRANCH);

		if(move->evaluated() || move->game_ended())
        { move->set_evaluated(1);
            return move->eval;
        }



		move->eval = 0;
		for(int side = 0;side < 2; side++)for(int picked_piece = 0;picked_piece < NUM_TYPES; picked_piece++ )
			move->eval += pieces[picked_piece][side].size()*VALUE[picked_piece]*(side==turn()?1:-1);



		///positional evaluation

		//now subtract opponent mobility
		set_turn(!turn());
		NULL_MOVE = true;
		mobility -= move_gen(move,NO_BRANCH);

		set_turn(!turn());
		NULL_MOVE = false;

		//doubled and isolated pawn penalty
		move->eval+=eval_weak_pawns(turn())-eval_weak_pawns(!turn());


		//passed pawn*
		int nm[] = {turn(),!turn()};
		for(int sd=0;sd<2;sd++)
        for(int i=0;i<pieces[PAWN][nm[sd]].size();i++)
        {
            int moving_dir = (nm[sd]==WHITE)?8:-8;
            bool passed = true, obstructed = false;
            int pos = pieces[PAWN][nm[sd]][i];
            for(int npos=pos+moving_dir;npos>=0 && npos<64;npos+=moving_dir)
            {
                if(getBit(pieceType[PAWN],npos))
                {
                    passed=false;
                    break;
                }
                if(getBit(colorPiece[!nm[sd]],npos))
                {
                    obstructed=true;
                }

            }
            if(passed)
                {
                    int scores[8]={0,3,6,9,14,21,35,35};
                    int adj = scores[nm[sd]?pos/8:7-pos/8];
                    LL pawns = colorPiece[nm[sd]]&pieceType[PAWN];
                    for(int dx=-1;dx<2;dx++)
                        for(int dy=-1;dy<2;dy++)if(dx||dy)
                        {
                            int x,y;
                            x = pos&7;
                            y = pos>>3;
                            x+=dx;
                            y+=dy;
                            if(x>=0 && x<8 && y>=0 && y<8)if(getBit(pawns,x+y*8))
                            {
                                adj *= 2;
                            }
                        }
                    if(obstructed)adj/=3;
                    move->eval+=adj * (nm[sd]==turn()?1:-1);

                }
        }

		//center pawns
		int coords[]={26,27,28,34,35,36};
		for(int i=0;i<6;i++)
			if(getBit(pieceType[PAWN],coords[i]))move->eval+=5*((getBit(colorPiece[turn()],coords[i]))?1:-1);

		//bishop pair
		move->eval += bishop_pair(turn())-bishop_pair(!turn());

		//bad bishop



		//semi open and open files for rook

		for(int sd=0;sd<2;sd++)
		for(int i=0;i<pieces[ROOK][nm[sd]].size();i++)
        {
            if(open_file(colorPiece[nm[sd]]&pieceType[PAWN],pieces[ROOK][nm[sd]][i]&7))move->eval+=3*(nm[sd]==turn()?1:-1);
            if(open_file(pieceType[PAWN],pieces[ROOK][nm[sd]][i]&7))move->eval+=7*(nm[sd]==turn()?1:-1);
        }




		//queen batteries with rook and bishop
		for(int sd=0;sd<2;sd++)
		for(int i=0;i<pieces[QUEEN][nm[sd]].size();i++)for(int j=0;j<pieces[BISHOP][nm[sd]].size();j++)
        {
            if(same_diag(pieces[QUEEN][nm[sd]][i],pieces[BISHOP][nm[sd]][j]))move->eval+=2*(nm[sd]==turn()?1:-1);
        }

        for(int sd=0;sd<2;sd++)
		for(int i=0;i<pieces[QUEEN][nm[sd]].size();i++)for(int j=0;j<pieces[ROOK][nm[sd]].size();j++)
        {
            if(same_line(pieces[QUEEN][nm[sd]][i],pieces[BISHOP][nm[sd]][j]))move->eval+=3*(nm[sd]==turn()?1:-1);
        }

		//rook batteries
		move->eval+=rook_battery(turn())-rook_battery(!turn());
		//king safety

		move->eval+=king_safety(turn())-king_safety(!turn());//TODO check if king safety works correctly



		///cout<<"king safety !turn = "<<king_safety(!turn())<<endl;
		///move->eval+=3*(left_castle_possible(turn())+right_castle_possible(turn())-(left_castle_possible(!turn())+right_castle_possible(!turn())));




		int sign = turn()==WHITE?1:-1;


		move->eval*=sign;



		int opening=0,endgame=0;

		opening += king_middle_game[ 63-pieces[KING][WHITE][0] ] - king_middle_game[ pieces[KING][BLACK][0] ] ;
		endgame += king_end_game[ 63-pieces[KING][WHITE][0] ] - king_end_game[ pieces[KING][BLACK][0] ] ;

		for(int i=0;i<=NUM_TYPES;i++)
		opening+=(generated_ct[i][WHITE]-generated_ct[i][BLACK])*MOB_OPEN_VALS[i];

		for(int i=0;i<=NUM_TYPES;i++)
		endgame+=(generated_ct[i][WHITE]-generated_ct[i][BLACK])*MOB_END_VALS[i];


        int phase = 24;
        phase -= (pieces[BISHOP][WHITE].size() + pieces[KNIGHT][WHITE].size() + pieces[ROOK][WHITE].size()*2 + pieces[QUEEN][WHITE].size()*4 ) ;
        phase -= (pieces[BISHOP][BLACK].size() + pieces[KNIGHT][BLACK].size() + 2*pieces[ROOK][BLACK].size() + 4*pieces[BLACK][WHITE].size() ) ;
        phase = (phase * 256 + (24 / 2)) / 24;

		move->eval += ((opening * (256 - phase)) + (endgame * phase)) / 256;


		///move->eval-=15*(getBit(flags,5)-getBit(flags,4));
		move->set_evaluated(true);
		return move->eval;
	}

	int Quiescence(Move *move,bool side, int alpha, int beta, int depth = QUIET_CUT)
	{
		rqnodes++;
		int standingPat = evaluate(move) * ((side)?1:-1);
		if(standingPat > HINF || standingPat < -HINF)
        {
            move->eval = (INF-dist(move))*(standingPat>0?1:-1);
            return move->eval;
        }



		if(depth <= 0){
			return standingPat;}




		if(move->game_ended()){
			return move->eval * ((side)?1:-1);}

		if (standingPat >= beta)
		{

			return beta;
		}
		if (alpha < standingPat)
		{
			alpha = standingPat;
		}
		move_gen(move,QUIESCENT);


		if(move->children.size()==0){
			return standingPat;}

		int score;
		for(int i=0;i<move->children.size();i++)
		{
			makemove(move->children[i]);

			score = -Quiescence(move->children[i],!side, -beta, -alpha,depth - 1);
			unmakemove(move->children[i]);

			if (score >= beta)
			{

				return beta;
			}
			if (score > alpha)
			{
				alpha = score;
			}

		}
		return alpha;
	}



	int alphabeta(Move *move,int depth, int alpha, int beta, bool maximizingPlayer)//modified for exact position evaluation
	{
		if(move->game_ended()){
                return move->eval;
        }




		LL hash_val = hash();
        Bundle *ttEntry = NULL;
        if ( hash_table.find(hash_val) == hash_table.end() ) {
              ttEntry = new Bundle;
              ttEntry->muci = NULL;
              ttEntry->Value;
              ttEntry->depth = 0;
              ttEntry->useful = 0;
              ttEntry->exact = 0;
              ttEntry->lower = 0;
              ttEntry->upper = 0;
              ttEntry->hash_val=hash_val;
              ///bdl->board=new Position(*this);
              hash_table[hash_val]=*ttEntry;
        }
        else {
              ttEntry = &hash_table[hash_val];
              ///if(maximizingPlayer)
              {
              if (ttEntry->hash_val==hash_val && ttEntry->depth >= depth)
              {
                if (ttEntry->exact)
                    return ttEntry->Value;
                else if (ttEntry->lower)
                    alpha = max( alpha, ttEntry->Value);
                else if (ttEntry->upper)
                    beta = min( beta, ttEntry->Value);
                if (alpha >= beta)
                    return ttEntry->Value;
              }
              }
        }


		if(depth == 0)
		{
			move->eval = maximizingPlayer?Quiescence(move,maximizingPlayer, alpha, beta): -Quiescence(move,maximizingPlayer, -beta, -alpha);//way faster
			move->set_evaluated(0);
			///if(maximizingPlayer)
            {
                    if(ttEntry->depth<=depth)
                    {
                    ttEntry->Value = move->eval;
                    if (move->eval <= alpha)
                        {
                            ttEntry->upper = true;
                            ttEntry->lower = false;
                            ttEntry->exact = false;
                        }
                    else if (move->eval>= beta)
                        {
                            ttEntry->upper = false;
                            ttEntry->lower = true;
                            ttEntry->exact = false;
                        }
                    else
                    {
                            ttEntry->upper = false;
                            ttEntry->lower = false;
                            ttEntry->exact = true;
                    }
                    ttEntry->depth = depth;
                    }
            }


			return move->eval;
		}




		//if( move->children.size() == 0 )//TODO check this line and comment/uncomment

		if(depth>4 && (pieceType[ROOK] || pieceType[BISHOP] || pieceType[QUEEN] || pieceType[KNIGHT]) &&!isCheck(turn()))
        {

            if(maximizingPlayer)
            {
                Move *tmp = new Move(*move);
                set_turn(!turn());
                int enpass = en_passant_file;
                en_passant_file=-1;
                move_gen(tmp);
                 int score = alphabeta(tmp, depth-3, beta-1,beta, !maximizingPlayer);
                 set_turn(!turn());
                 en_passant_file=enpass;
                 if(score>=beta){move->eval=score;move->set_evaluated(false);return score;}
            }
            else{
                Move *tmp = new Move(*move);
                set_turn(!turn());
                int enpass = en_passant_file;
                en_passant_file=-1;
                move_gen(tmp);
                 int score = alphabeta(tmp, depth-3, alpha,alpha+1, !maximizingPlayer);
                 set_turn(!turn());
                 en_passant_file=enpass;
                 if(score<=alpha){move->eval=score;move->set_evaluated(false);return score;}
            }
        }





		if( !move->all_move())
		{
			pv = NULL;
			move_gen(move);
		}
		else
		{//for iterative deepening

		    ///NEW IDEA just get the PV from here rather than sorting

            if(maximizingPlayer) stable_sort(move->children.begin(),move->children.end(),compare);
			else stable_sort(move->children.begin(),move->children.end(),compare2);

            if(move->children.size())
			pv = move->children[0];




			//sort killers
		}




		if(move->children.size()==0)
        {

            int score = move->eval;

            if(score>HINF || score<-HINF)
            {
                score = ( (INF-dist(move)-1)*(score>0?1:-1) );
            }
            move->eval = score;
            return score;

        }

		hist_turn = maximizingPlayer;
		kill_helper = depth;

		//if()
		stable_sort(move->children.begin(),move->children.end(),histsort);


		//if (maximizingPlayer)
        stable_sort(move->children.begin(),move->children.end(),killsort);

        if(ttEntry->muci!=NULL && ttEntry->depth>=depth-3)
        {
            for(int i=0;i<move->children.size();i++)
            {
                if( move->children[i]->equal(ttEntry->muci))
                {
                    pv=move->children[i];
                    for(int j=0;j<i;j++)
                    {
                        move->children[j+1]=move->children[j];
                    }
                    move->children[0]=pv;
                    break;
                }
            }
        }
        ///NOW sort by capture and promotions
        //stable_sort(move->children.begin(),move->children.end(),capsort);
        ///now capsort is not req


		Move *killer = NULL;
		if (maximizingPlayer)
		{
		    int alphaOrig = alpha;
            int result,best_value=-DINF;
			for(int i=0;i<move->children.size();i++)
			{
				makemove(move->children[i]);


				result = alphabeta(move->children[i],depth - 1, alpha, beta, false);
				best_value = max( best_value, result );
				if(depth==MAX_DEPTH && depth>4){
				///cout<<"scored "<<result<<endl;
				if(VERBOSE){cout<<"info score cp "<<result<<" depth "<<depth<<" pv ";
				move->children[i]->print();
				cout<<endl;}
				}

				if (alpha<result)
                {
                    killer = move->children[i];
                    alpha = result;
                }
				//alpha = max(alpha, result);

				unmakemove(move->children[i]);




				if(beta <= alpha)//cut-off
                    {
                        if(killer->uci[4]<'A' && killer->capture<'A')
                        {
                            if(killer_count[depth]<KILLER_LEVELS )
                            {
                                killers[depth][killer_count[depth]++] = killer->uci;
                            }
                            else {
                                killers[depth][0] = killer->uci;
                            }
                            history[maximizingPlayer][CONV(killer->uci,0)][CONV(killer->uci,2)]+=depth*depth;
                        }
                        break;
                    }

			}

            if(ttEntry->depth<=depth)
            {
			ttEntry->Value = best_value;
			if (best_value <= alphaOrig)
                {
                    ttEntry->upper = true;
                    ttEntry->lower = false;
                    ttEntry->exact = false;
                }
            else if (best_value>= beta)
                {
                    ttEntry->upper = false;
                    ttEntry->lower = true;
                    ttEntry->exact = false;
                }
            else
            {
                    ttEntry->upper = false;
                    ttEntry->lower = false;
                    ttEntry->exact = true;
            }
            ttEntry->depth = depth;

            if(killer!=NULL)
            {
                ttEntry->muci = new char[6];
                for(int b=0;b<5;b++)ttEntry->muci[b]=killer->uci[b];
            }
            }


			move->eval = best_value; move->set_evaluated(false);



			return alpha;
		}
		else//NON MAXIMIZER
		{
		    int best_value = DINF, betaOrig=beta;

			for(int i=0;i<move->children.size();i++)
			{
				makemove(move->children[i]);
				int res = alphabeta(move->children[i],depth - 1, alpha, beta, true);


				best_value = min(res,best_value);
				if(beta>res)
                {
                    killer = move->children[i];
                    beta = res;
                }
				if(depth==MAX_DEPTH && depth>4){
				if(VERBOSE){cout<<"info score cp "<<res<<" depth "<<depth<<" pv ";
				move->children[i]->print();}
				}

				unmakemove(move->children[i]);
				if(beta <= alpha)//cut-off
                    {
                        if(killer->uci[4]<'A' && killer->capture<'A')
                        {
                            if(killer_count[depth]<KILLER_LEVELS )
                            {
                                killers[depth][killer_count[depth]++] = killer->uci;
                            }
                            else {
                                killers[depth][0] = killer->uci;
                            }

                            history[maximizingPlayer][CONV(killer->uci,0)][CONV(killer->uci,2)]+=depth*depth;
                        }

                        break;
				}
			}
			move->eval = best_value; move->set_evaluated(false);

			if(ttEntry->depth<=depth)
            {
			ttEntry->Value = best_value;
			if (best_value <= alpha)
                {
                    ttEntry->upper = true;
                    ttEntry->lower = false;
                    ttEntry->exact = false;
                }
            else if (best_value>= betaOrig)
                {
                    ttEntry->upper = false;
                    ttEntry->lower = true;
                    ttEntry->exact = false;
                }
            else
            {
                    ttEntry->upper = false;
                    ttEntry->lower = false;
                    ttEntry->exact = true;
            }
            ttEntry->depth = depth;
            if(killer!=NULL)
            {
                ttEntry->muci = new char[6];
                for(int b=0;b<5;b++)ttEntry->muci[b]=killer->uci[b];
            }
            }

			return beta;
		}
	}
	Move* best_move(Move *move,int d, int &score,int alpha=-DINF, int beta=DINF)
	{
		score = alphabeta(move,d, alpha, beta, turn()==WHITE);
		if(VERBOSE)cout<<"Score = "<<score<<endl;
		for(int i=0;i<move->children.size();i++)if(score == move->children[i]->eval)return move->children[i];
        if(move->children.size()==0)
            {
                cout<<"GAME ENDED\n";exit(0);
            }
		Move *bm = move->children[0];
        if(turn())
        {
            for(int i=1;i<move->children.size();i++)
                if(move->children[i]->eval > bm->eval )bm=move->children[i];

        }
        else{
            for(int i=1;i<move->children.size();i++)
                if(move->children[i]->eval < bm->eval )bm=move->children[i];
        }
        return bm;

	}
};

int Position::object_count = 0;
int main()
{
    srand ( time(NULL) );
    for(int i=0;i<13 ;i++)for(int j=0;j<64;j++){random_bit_string[i][j]=lrand();}
    dx[PAWN].push_back(1);
    dx[PAWN].push_back(-1);
    dx[KNIGHT].push_back(1);dy[KNIGHT].push_back(2);
    dx[KNIGHT].push_back(2);dy[KNIGHT].push_back(1);
    dx[KNIGHT].push_back(2);dy[KNIGHT].push_back(-1);
    dx[KNIGHT].push_back(1);dy[KNIGHT].push_back(-2);
    dx[KNIGHT].push_back(-1);dy[KNIGHT].push_back(-2);
    dx[KNIGHT].push_back(-2);dy[KNIGHT].push_back(-1);
    dx[KNIGHT].push_back(-2);dy[KNIGHT].push_back(1);
    dx[KNIGHT].push_back(-1);dy[KNIGHT].push_back(2);

    dx[ROOK].push_back(1);dy[ROOK].push_back(0);
    dx[ROOK].push_back(0);dy[ROOK].push_back(1);
    dx[ROOK].push_back(0);dy[ROOK].push_back(-1);
    dx[ROOK].push_back(-1);dy[ROOK].push_back(0);

    dx[BISHOP].push_back(1);dy[BISHOP].push_back(-1);
    dx[BISHOP].push_back(1);dy[BISHOP].push_back(1);
    dx[BISHOP].push_back(-1);dy[BISHOP].push_back(1);
    dx[BISHOP].push_back(-1);dy[BISHOP].push_back(-1);

    append_vector_elements(dx[QUEEN],dx[ROOK]);append_vector_elements(dy[QUEEN],dy[ROOK]);
    append_vector_elements(dx[QUEEN],dx[BISHOP]);append_vector_elements(dy[QUEEN],dy[BISHOP]);
    append_vector_elements(dx[KING],dx[QUEEN]);append_vector_elements(dy[KING],dy[QUEEN]);

	thinking = new Position();
	Position *pos=NULL;
	///pos=new Position();

	///Move *base_move;
	///base_move = new Move(pos->flags,pos->en_passant_file,-1,-1);
	///pos->print();
	Move *mv=NULL;/// = base_move;
	///char s[7];
	Move *bes =NULL;

	string Line,prevLine="Aditya";
    cout.setf (ios::unitbuf);
        if (! file) {
        cerr << "can't open output file \"" << filename << "\""
        << endl;
        exit (EXIT_FAILURE);
        }
	while (getline (cin, Line))
    {
        file << Line <<endl;

        istringstream iss(Line);
        istringstream iss2(prevLine);

        if (Line == "uci")
        {
            cout << "id name Conqueror" << endl;
            cout << "id author Aditya Pande, India" << endl;
            cout << "uciok" << endl;
            file << "id name Conqueror" << endl;
            file << "id author Aditya Pande, India" << endl;
            file << "uciok" << endl;

        }
        else if (Line == "isready")
        {
            cout << "readyok" << endl;
            file << "me:\treadyok" << endl;
        }

        else if (Line == "ucinewgame")
        {
            ///cout<<"Cleaning up\n";
            if(mv!=NULL)mv->delchild();
            max_time = 300000;
            LIMIT = max_time/NORM;
            TimeTrouble = 100;
        }

        else if(Line.substr(0,3)=="pos" && (Line.find(prevLine) != std::string::npos) )///continuing old game
        {
            ///cout<<"Lets continue\n";
            vector<string> tokens;copy(istream_iterator<string>(iss),istream_iterator<string>(),back_inserter<vector<string> >(tokens));
            vector<string> tokens2;copy(istream_iterator<string>(iss2),istream_iterator<string>(),back_inserter<vector<string> >(tokens2));
            for(int i=tokens2.size();i<tokens.size();i++)
            {
                char *s = const_cast<char*>(tokens[i].c_str());
                pos->Play(mv,s);
            }
        }

        else if (Line.substr(0,17) == "position startpos")
            {
                pos = new Position();
                mv = new Move(pos->flags,pos->en_passant_file,-1,-1);
                vector<string> tokens;copy(istream_iterator<string>(iss),istream_iterator<string>(),back_inserter<vector<string> >(tokens));
                for(int i=0;i<tokens.size();i++)
                {
                    if(tokens[i]=="moves")
                    {
                        for(int j=i+1;j<tokens.size();j++)
                        {
                            char *s = const_cast<char*>(tokens[j].c_str());
                            pos->Play(mv,s);
                        }
                        break;
                    }
                }
                pos->print();
            }

        else if (Line.substr(0,12) == "position fen")
        {
            char *fen = new char[Line.length()];
            int ll = Line.length();
            for(int i=13;i<ll;i++)
            {
                fen[i-13]=Line.at(i);
            }
            fen[ll-13]=0;
            pos = new Position(fen);
            delete fen;
            ///pos->print();
                mv = new Move(pos->flags,pos->en_passant_file,-1,-1);
                vector<string> tokens;copy(istream_iterator<string>(iss),istream_iterator<string>(),back_inserter<vector<string> >(tokens));
                for(int i=0;i<tokens.size();i++)
                {
                    if(tokens[i]=="moves")
                    {
                        for(int j=i+1;j<tokens.size();j++)
                        {
                            char *s = const_cast<char*>(tokens[j].c_str());
                            pos->Play(mv,s);
                        }
                        break;
                    }
                }
                pos->print();
        }

        else if (Line.substr (0, 2) == "go")
        {
            // "go wtime 300000 btime 300000 winc 0 binc 0"
            ///cout << "bestmove e7e5" << endl;
            ///file << "me:\tbestmove e7e5" << endl;

            vector<string> tokens;copy(istream_iterator<string>(iss),istream_iterator<string>(),back_inserter<vector<string> >(tokens));
            int result;
            if(tokens.size()>4)
            {
                stringstream(tokens[pos->turn()?2:4]) >> result;
                if(result > max_time)
                {max_time = result;
                LIMIT = max_time/NORM;
                }
                if(result < 15000)
                {
                    TimeTrouble = 4;
                }
            }
                    rqnodes=0;
                    root = mv;
                    int alpha = -DINF, beta = DINF;
                    memset (history, 0, sizeof(history));
                    MAX_DEPTH=3;
                    for(int k=MIN_DEPTH;k<=MAX_DEPTH;)
                    {
                        hash_table.clear();///VERY imp to avoid the iterative deep. bugs
                        int val;
                        if(VERBOSE)cout<<"k = "<<k<<endl;
                        bes = pos->best_move(mv,k,val,alpha,beta);
                        if(VERBOSE)bes->print();
                        for(int var = 0;var<mv->children.size();var++)if(mv->children[var]!=bes)
                        {
                            mv->children[var]->delchild();
                        }
                        if(!(-HINF<val && val < HINF))
                        {
                            MAX_DEPTH = 1;
                        }

                        else if(k==MAX_DEPTH && rqnodes<LIMIT && MAX_DEPTH<TimeTrouble)MAX_DEPTH++;
                        k++;
                        for(int zx=0;zx<100;zx++)killer_count[zx]=0;

                    }
                    pos->makemove(bes);
                    pos->print();



                    /**Move*tmp=bes;
                        for(int i=1;i<MAX_DEPTH;i++)
                        {
                            for(int j=0;j<tmp->children.size();j++)
                            if(tmp->children[j]->eval==tmp->eval){
                                tmp=tmp->children[j];
                                tmp->print();
                            }
                        }*/
                    mv = bes;


                    cout<<"bestmove ";
                    file<<"bestmove ";
                    bes->print();
                    cout<<endl;
                    file<<endl;
        }
        else if (Line.substr (0, 4) == "quit")
        {
            exit(0);
        }
        /*else
        {
            // Command not handled
            cout << "what?" << endl;
            file << "me:\twhat?" << endl;
        }*/
        prevLine.assign(Line);
    }
	return 0;
}
