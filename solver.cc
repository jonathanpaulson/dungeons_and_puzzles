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
#include <algorithm>

using namespace std;
using ll = int64_t;
using pll = pair<ll,ll>;

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
  pll add(pll p) { return make_pair(p.first+dr(), p.second+dc()); }
  static vector<Dir> values() { return vector<Dir>{Up,Right,Down,Left}; }
  Dir flip() { return Dir::Value((v+2)%4); }
};

struct State {
  ll R;
  ll C;
  ll r,c;
  vector<vector<char>> G; // map
  vector<int> T; // available tools

  pll p() { return make_pair(r,c); }

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
  bool isPushDest(ll rr, ll cc) {
    if(!inBounds(rr,cc)) { return false; }
    if(isStool(rr,cc)) { return true; }
    if(isEmpty(rr,cc)) { return true; }
    return false;
  }
  bool isPushable(pll p) {
    auto [rr,cc] = p;
    if(!inBounds(rr,cc)) { return false; }
    if(isMonster(rr,cc)) { return true; }
    if(isObstacle(rr,cc)) { return true; }
    if(isStool(rr,cc)) { return true; }
    if(isTool(rr,cc)) { return true; }
    if(isArrow(rr,cc)) { return true; }
    return false;
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
  bool isObstacle(ll rr, ll cc) {
    if(!inBounds(rr,cc)) { return false; }
    return 'a'<=G[rr][cc] && G[rr][cc]<='z';
  }

  bool push(ll rr, ll cc, Dir d) {
    if(!inBounds(rr,cc)) { return false; }
    ll id = G[rr][cc];
    bool dead = false;
    vector<pll> OLD;
    vector<pll> NEW;
    for(ll r3=0; r3<R; r3++) {
      for(ll c3=0; c3<C; c3++) {
        // stools are only 1 square big
        if(G[r3][c3]==id && (id!='*' || (r3==rr && c3==cc))) {
          ll r4 = r3+d.dr();
          ll c4 = c3+d.dc();
          if(!inBounds(r4,c4)) { return false; }
          char ch = G[r4][c4];
          if(!(ch=='.' || (ch==id && id!='*') || (ch=='^' && ('A'<=id && id<='Z')))) {
            return false;
          }
          if(G[r4][c4]=='^') { dead = true; }
          OLD.push_back(make_pair(r3,c3));
          NEW.push_back(make_pair(r4,c4));
        }
      }
    }
    for(auto [r3,c3] : OLD) {
      G[r3][c3] = '.';
    }
    if(!dead) {
      for(auto [r3,c3] : NEW) {
        G[r3][c3] = id;
      }
    }
    return true;
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
  // sort monsters and obstacles
  void normalize() {
    map<char,char> M;
    char next_monster = 'A';
    char next_obstacle = 'a';
    for(ll rr=0; rr<R; rr++) {
      for(ll cc=0; cc<C; cc++) {
        char ch = G[rr][cc];
        if('A'<=ch && ch<='Z' && M.count(ch)==0) {
          M[ch] = next_monster;
          next_monster++;
        }
        if('z'<=ch && ch<='z' && M.count(ch)==0) {
          M[ch] = next_obstacle;
          next_obstacle++;
        }
      }
    }
    for(ll rr=0; rr<R; rr++) {
      for(ll cc=0; cc<C; cc++) {
        if(M.count(G[rr][cc])==1) {
          G[rr][cc] = M[G[rr][cc]];
        }
      }
    }
  }

  bool operator<(const State& o) const {
    return make_tuple(r,c,G,T) < make_tuple(o.r,o.c,o.G,o.T);
  }
  bool operator==(const State& o) const {
    return make_tuple(r,c,G,T) == make_tuple(o.r,o.c,o.G,o.T);
  }
  bool operator!=(const State& o) const {
    return make_tuple(r,c,G,T) != make_tuple(o.r,o.c,o.G,o.T);
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
              sort(S2.T.begin(), S2.T.end());
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
      } else if(t==4) { // shield: pushes
        while(true) {
          ll rr = S2.r+d.dr();
          ll cc = S2.c+d.dc();
          if(!S2.inBounds(rr,cc)) {
            break;
          } else if(S2.isSpike(rr,cc)) {
            break;
          } else if(S2.isPushable(make_pair(rr,cc))) {
            if(S2.push(rr,cc,d)) {
              changed = true;
            }
            break;
          } else if(S2.isEmpty(rr,cc)) {
            changed = true;
            S2.r = rr;
            S2.c = cc;
          } else {
            break;
          }
        }
      } else if(t==5) { // glove: pulls
        pll dest = d.add(S2.p());
        pll src = d.flip().add(S2.p());
        if(S2.isPushable(src) && (S2.isPushDest(dest.first, dest.second))) {
          if(S2.push(src.first,src.second,d)) {
            S2.r = dest.first;
            S2.c = dest.second;
            changed = true;
          }
        }
      } else {
        assert(false);
      }
      if(changed && !dead) {
        S2.normalize();
        ans.push_back(make_pair(S2, make_pair(t, d.toChar())));
      }
    }
  }
  return ans;
}
bool done(const State& S) {
  return S.G[S.r][S.c] == '>';
}

ll path_length(State x, State start, map<State, pair<State, Move>>& PAR) {
  ll ans = 0;
  while(x!=start) {
    x = PAR[x].first;
    ans++;
  }
  return ans;
}
void show_path(State x, State start, map<State, pair<State, Move>>& PAR) {
  vector<Move> M;
  while(x!=start) {
    auto& [p, m] = PAR[x];
    M.push_back(m);
    x = p;
  }
  cout << M.size() << endl;
  for(ll i=0; i<M.size(); i++) {
    auto [t,d] = M[M.size()-1-i];
    cout << (i+1) << ": " << t << d << endl;
  }
}

int main() {
  map<State, pair<State, Move>> PAR;
  State start = readLevel();
  queue<State> Q;
  Q.push(start);
  while(!Q.empty()) {
    State x = Q.front(); Q.pop();
    //cerr << "=========================" << endl;
    //show_path(x, start, PAR);
    //cerr << x << endl;
    if(done(x)) {
      show_path(x, start, PAR);
      return 0;
    }
    for(auto& [y,m] : moves(x)) {
      //cerr << m << endl << y << endl;
      if(PAR.count(y)==0) {
        PAR[y] = make_pair(x, m);
        if(PAR.size()%10000==0) {
          cerr << PAR.size() << " " << path_length(y, start, PAR) << endl;
        }
        Q.push(y);
      }
    }
    //cerr << "=========================" << endl;
  }
}
