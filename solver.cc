// '!': start
// '#': wall
// '>': stairs
// UPPERCASE: monsterr
// lowercase: obstacle
// 2,3,4,5: tools
// ^: spike
// *: stool
// -: arrow
//
// Move is {1,2,3,4,5}{L,R,U,D} i.e. tool+direction
// 1 is "boots"
// 2 is "sword"
// 3 is "bow"
// 4 is "shield"
// 5 is "glove"

#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <queue>
#include <map>
#include <cassert>
#include <set>

using namespace std;
using ll = int64_t;

struct Dir {
  enum Value {
    Up = 0,
    Right = 1,
    Down = 2,
    Left = 3
  };
  Dir(Value v_) : v(v_) {}
  Value v;
  char toChar() {
    return "URDL"[v];
  }
  int dr() { return vector<int>{-1, 0, 1, 0}[v]; }
  int dc() { return vector<int>{0, 1, 0, -1}[v]; }
  static vector<Dir> values() { return vector<Dir>{Up,Right,Down,Left}; }
};

struct State {
  ll R;
  ll C;
  ll r,c;
  vector<vector<char>> G; // map
  vector<int> T; // available tools

  bool inBounds(ll rr, ll cc) {
    return 0<=rr && rr<R && 0<=cc && cc<C;
  }
  bool isTool(ll rr, ll cc) {
    assert(inBounds(rr,cc));
    char ch = G[rr][cc];
    return ch=='2' || ch=='3' || ch=='4' || ch=='5';
  }
  bool isSpike(ll rr, ll cc) {
    assert(inBounds(rr,cc));
    return G[rr][cc]=='^';
  }
  bool isStool(ll rr, ll cc) {
    assert(inBounds(rr,cc));
    return G[rr][cc]=='*';
  }
  bool isArrow(ll rr, ll cc) {
    assert(inBounds(rr,cc));
    return G[rr][cc]=='-';
  }
  bool isEmpty(ll rr, ll cc) {
    if(!inBounds(rr,cc)) { return false; }
    return G[rr][cc]=='.';
  }
  bool isMoveable(ll rr, ll cc) {
    if(!inBounds(rr,cc)) { return false; }
    char ch = G[rr][cc];
    if(isEmpty(rr,cc)) { return true; }
    if(isTool(rr,cc)) { return true; }
    if(isSpike(rr,cc)) { return true; }
    if(isStool(rr,cc)) { return true; }
    if(isArrow(rr,cc)) { return true; }
    if(ch=='>') { return noMonsters(); }
    return false;
  }
  bool noArrow() {
    for(ll rr=0; rr<R; rr++) {
      for(ll cc=0; cc<C; cc++) {
        if(G[rr][cc]=='-') {
          return false;
        }
      }
    }
    return true;
  }
  bool noMonsters() {
    for(ll rr=0; rr<R; rr++) {
      for(ll cc=0; cc<C; cc++) {
        if('A'<=G[rr][cc] && G[rr][cc]<='Z') {
          return false;
        }
      }
    }
    return true;
  }
  bool isMonster(ll rr, ll cc) {
    if(!inBounds(rr,cc)) { return false; }
    return 'A'<=G[rr][cc] && G[rr][cc]<='Z';
  }
  void kill(ll rr, ll cc) {
    ll id = G[rr][cc];
    G[rr][cc] = '.';
    for(Dir d : Dir::values()) {
      ll rrr = rr+d.dr();
      ll ccc = cc+d.dc();
      if(inBounds(rrr,ccc) && G[rrr][ccc]==id) {
        kill(rrr,ccc);
      }
    }
  }

  bool operator<(const State& o) const {
    return make_tuple(r,c,G,T) < make_tuple(o.r,o.c,o.G,o.T);
  }
  bool operator==(const State& o) const {
    return make_tuple(r,c,G,T) == make_tuple(o.r,o.c,o.G,o.T);
  }
};
ostream& operator<<(ostream& o, const pair<int,char>& move) {
  o << move.first << move.second;
  return o;
}
ostream& operator<<(ostream& o, const State& S) {
  o << "Tools:";
  for(ll t : S.T) {
    o << " " << t;
  }
  o << endl;
  for(ll r=0; r<S.R; r++) {
    for(ll c=0; c<S.C; c++) {
      if(r==S.r && c==S.c) {
        o << "!";
      } else {
        o << S.G[r][c];
      }
    }
    o << endl;
  }
  return o;
}

using Move = pair<int, char>;

State readLevel() {
  ll r = -1;
  ll c = -1;
  vector<vector<char>> G;
  while(true) {
    string S;
    cin >> S;
    vector<char> ROW;
    for(ll i=0; i<S.size(); i++) {
      if(S[i]=='!') {
        r = G.size();
        c = i;
        ROW.push_back('.');
      } else {
        ROW.push_back(S[i]);
      }
    }
    if(ROW.size() > 0) {
      G.push_back(ROW);
    }

    if(cin.eof()) {
      assert(G.size() > 0);
      ll R = static_cast<ll>(G.size());
      ll C = static_cast<ll>(G[0].size());
      assert(0<=r && r<R && 0<=c && c<C);
      for(auto& row : G) {
        assert(row.size() == C);
      }
      return State{R, C, r, c, G, vector<int>{1}};
    }
  }
}
vector<pair<State, Move>> moves(const State& S) {
  vector<pair<State, Move>> ans;
  for(ll t : S.T) {
    for(Dir d : vector<Dir::Value>{Dir::Up, Dir::Right, Dir::Down, Dir::Left}) {
      bool dead = false;
      bool changed = false;
      State S2(S);
      S2.G = vector<vector<char>>(S.G);
      S2.T = vector<int>(S.T);
      if(t==1) {
        // Walk in the direction until you hit something
        // tool -> pick it up and stop
        // stool -> stop
        // spike -> die
        while(true) {
          ll rr = S2.r+d.dr();
          ll cc = S2.c+d.dc();
          if(S2.isMoveable(rr,cc)) {
            changed = true;
            S2.r = rr;
            S2.c = cc;
            if(S2.isTool(rr,cc)) {
              S2.T.push_back(S2.G[rr][cc]-'0');
              S2.G[rr][cc] = '.';
              break;
            } else if(S2.isSpike(rr,cc)) {
              dead = true;
              break;
            } else if(S2.isStool(rr,cc)) {
              break;
            } else if(S2.isArrow(rr,cc)) {
              S2.G[rr][cc] = '.';
              break;
            }
          } else {
            break;
          }
        }
      } else if(t==2) { // sword
        ll rr = S2.r+d.dr();
        ll cc = S2.c+d.dc();
        if(S2.isMonster(rr,cc)) {
          changed = true;
          S2.kill(rr,cc);
        }
      } else if(t==3) { // bow
        if(S2.noArrow()) {
          ll ar = S2.r;
          ll ac = S2.c;
          while(true) {
            ll rr = ar+d.dr();
            ll cc = ac+d.dc();
            if(S2.isEmpty(rr,cc)) {
              ar = rr;
              ac = cc;
            } else if(S2.isMonster(rr,cc)) {
              ar = rr;
              ac = cc;
              S2.kill(rr,cc);
              break;
            } else {
              break;
            }
          }
          if(ar!=S2.r || ac!=S2.c) {
            S2.G[ar][ac] = '-';
            changed = true;
          }
        }
      } else {
        assert(false);
      }
      if(changed && !dead) {
        ans.push_back(make_pair(S2, make_pair(t, d.toChar())));
      }
    }
  }
  return ans;
}
bool done(const State& S) {
  return S.G[S.r][S.c] == '>';
}

int main() {
  map<State, pair<State, Move>> PAR;
  State start = readLevel();
  queue<State> Q;
  Q.push(start);
  while(!Q.empty()) {
    State x = Q.front(); Q.pop();
    //cerr << x << endl;
    if(done(x)) {
      vector<Move> M;
      while(true) {
        auto& [p, m] = PAR[x];
        M.push_back(m);
        x = p;
        if(x==start) { break; }
      }
      cout << M.size() << endl;
      for(ll i=0; i<M.size(); i++) {
        auto [t,d] = M[M.size()-1-i];
        cout << (i+1) << ": " << t << d << endl;
      }
      return 0;
    }
    for(auto& [y,m] : moves(x)) {
      //cerr << m << endl << y << endl;
      if(PAR.count(y)==0) {
        PAR[y] = make_pair(x, m);
        Q.push(y);
      }
    }
  }
}
