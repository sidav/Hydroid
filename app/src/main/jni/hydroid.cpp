#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

bool panic = false;     // set when we have to quit the game quickly

#define CRASHFILE "/sdcard/hydroid/crash.txt"

// algorithm

int max(int a, int b) { return a>b?a:b; }
int min(int a, int b) { return a<b?a:b; }
template<class T> void swap(T& x, T& y) { T z; z=x; x=y; y=z; }
template<class T> void stable_sort(T* a, T* b, bool less(const T& x, const T& y)) {
  T *p = a;
  while(p < b) {
    if(p == a) p++;
    else if(less(p[0], p[-1])) { swap(p[0], p[-1]); p--; }
    else p++;
    }
  }

// string

template<int STRSIZE> struct sstring {
  static const int STRSIZ = STRSIZE-1;
  static const int npos = 999;
  char b[STRSIZE];
  char& operator [] (int i) { return b[i]; }
  const char& operator [] (int i) const { return b[i]; }
  char* c_str() { return b; }
  int size() const { for(int k=0; k<STRSIZE; k++) if(b[k] == 0) return k; return STRSIZE; }
  void operator = (const char* x) {
    int i = 0;
    while(i < STRSIZ && *x) { b[i] = *x; i++; x++; }
    b[i] = 0;
    }
  sstring () { b[0] = 0; }
  sstring (const char* x) {
    int i = 0;
    while(i < STRSIZ && *x) { b[i] = *x; i++; x++; }
    b[i] = 0;
    }
  sstring (char x) { b[0] = x; b[1] = 0; }
  sstring (int qty, char x) { if(qty>STRSIZ) qty=STRSIZ; for(int i=0; i<qty; i++) b[i] = x; b[qty] = 0; }
  sstring substr(int from, int qty = npos) {
    sstring res;
    if(from+qty > STRSIZ) qty = STRSIZ-from;
    for(int k=0; k<qty; k++) res[k] = b[from+k];
    res[qty] = 0;
    return res;
    }
  int find(const sstring& s) { return npos; }
  void replace(int at, int qty, const sstring& s) { }
  };

template<int STRSIZE> bool operator == (sstring<STRSIZE>&a, sstring<STRSIZE>& b) {
  for(int k=0; k<STRSIZE; k++) {
    if(a[k] != b[k]) return false;
    if(!a[k]) return true;
    }
  return true;
  };

template<int STRSIZE> bool operator == (sstring<STRSIZE>&a, const char *b) {
  for(int k=0; k<STRSIZE; k++) {
    if(a[k] != b[k]) return false;
    if(!a[k]) return true;
    }
  return true;
  };

template<int X, int Y> void operator+= (sstring<X>& a, const sstring<Y>& b) {
  int pa=0, pb=0;
  while(a[pa]) pa++;
  while(b[pb] && pa < a.STRSIZ)
    a[pa] = b[pb], pa++, pb++;
  a[pa] = 0;
  }

template<int X> void operator+= (sstring<X>& a, const char *b) { 
  a += sstring<X> (b);
  }
  
template<int X> void operator+= (sstring<X>& a, char b) { 
  a += sstring<X> (b);
  }
  
template<int X, int Y> sstring<X> operator+ (const sstring<X>& a, const sstring<Y>& b) {
  sstring<X> res;
  int pa=0, pb=0;
  while(a[pa]) res[pa] = a[pa], pa++;
  while(b[pb] && pa < a.STRSIZ) res[pa] = b[pb], pa++, pb++;
  res[pa] = 0;
  return res;
  }

template<int X> sstring<X> operator+ (const sstring<X>& a, const char *b) {
  sstring<X> res;
  int pa=0;
  while(a[pa]) res[pa] = a[pa], pa++;
  while(*b && pa < a.STRSIZ) res[pa] = b[0], pa++, b++;
  res[pa] = 0;
  return res;
  }

template<int X> sstring<X> operator+ (const sstring<X>& a, char b) {
  sstring<X> res;
  int pa=0;
  while(a[pa]) res[pa] = a[pa], pa++;
  if(pa < a.STRSIZ) res[pa] = b, pa++;
  res[pa] = 0;
  return res;
  }

template<int X> sstring<X> operator+ (const char *a, const sstring<X>& b) {
  return sstring<X>(a) + b;
  }

template<int X> sstring<X> operator+ (const char x, const sstring<X>& b) {
  return sstring<X>(x) + b;
  }

template<int X, int Y> bool operator== (const sstring<X>& a, const sstring<Y>& b) {
  int i = 0;
  while(a[i] && a[i] == b[i]) i++;
  return a[i] == b[i];
  }
  
template<int X, int Y> bool operator!= (const sstring<X>& a, const sstring<Y>& b) {
  int i = 0;
  while(a[i] && a[i] == b[i]) i++;
  return a[i] != b[i];
  }

template<int X> bool operator!= (const sstring<X>& a, const char *b) {
  int i = 0;
  while(a[i] && a[i] == b[i]) i++;
  return a[i] != b[i];
  }

typedef sstring<2000> string;

// vector

template<class T> struct vector {
  int qsize, capacity;
  T* arr;
  vector() { qsize = 0; capacity = 16; arr = new T[16]; }
  vector(const vector& w) { 
    qsize = w.qsize; capacity = w.capacity; arr = new T[w.capacity]; 
    for(int i=0; i<qsize; i++) arr[i] = w.arr[i];
    }
  void operator = (const vector& w) {
    qsize = w.qsize; capacity = w.capacity; arr = new T[w.capacity]; 
    for(int i=0; i<qsize; i++) arr[i] = w.arr[i];
    }
  ~vector() { delete [] arr; }
  T& operator[] (int i) { return arr[i]; }
  const T& operator[] (int i) const { return arr[i]; }
  void reserve(int newcap) {
    if(newcap <= capacity) return;
    while((newcap & (newcap-1))) newcap &= (newcap-1);
    newcap *= 2;
    if(newcap < 16) newcap = 16;
    T* narr = new T[newcap];
    for(int i=0; i<qsize; i++) narr[i] = arr[i];
    delete[] arr;
    arr = narr;
    capacity = newcap;
    }
  void resize(int newqsize) { 
    reserve(newqsize); qsize = newqsize;
    }
  void push_back(const T& x) {
    resize(qsize+1);
    arr[qsize-1] = x;
    }
  T* begin() { return arr; }
  T* end() { return arr+qsize; }
  void insert(T* where, const T& what) {
    T* w2 = end();
    resize(qsize+1);
    while(w2 != where) {w2--; w2[1] = w2[0]; }
    where[0] = what;
    }
  void clear() { qsize = 0; }
  bool empty() { return qsize == 0; }
  int size() const { return qsize; }
  };
                                    
template<class T> void swap(vector<T>& x, vector<T>& y) {
  printf("Swapped!\n");
  swap(x.qsize, y.qsize);
  swap(x.capacity, y.capacity);
  swap(x.arr, y.arr);
  }

// complex

template<class T> struct complex {
  T r, i;
  complex () { r=0; i=0; }
  complex (T R) { r=R; i=0; }
  complex (T R, T I) { r=R; i=I; }
  };

template <class T> T abs(complex<T> z) { return sqrt(z.r*z.r+z.i*z.i); }

template <class T> complex<T> operator * (const complex<T>& a, const complex<T>& b) {
  return complex<T> (a.r*b.r-a.i*b.i, a.r*b.i+a.i*b.r);
  }

template <class T> complex<T> operator + (const complex<T>& a, const complex<T>& b) {
  return complex<T> (a.r+b.r, a.i+b.i);
  }

// Curses replacement

int ccol = 7, curx, cury;

int colat[24][80];
char charat[24][80];

#ifdef FAKE
#include <curses.h>
#endif

void col(int x) {
  ccol = x; 
  }

void hydroid_addch(char ch) {
  if(curx < 80)
    colat[cury][curx] = ccol, charat[cury][curx] = ch, curx++;
  }
void hydroid_addstr(const char *buf) { 
  while(*buf) hydroid_addch(*buf), buf++;
  }

void hydroid_endwin() { }
void hydroid_halfdelay(int i) { }

void hydroid_erase() { 
  for(int y=0; y<24; y++) for(int x=0; x<80; x++) 
    colat[y][x] = ccol, charat[y][x] = ' ';
  curx=0; cury=0;
  }
void hydroid_move(int y, int x) {
  curx=x; cury=y;
  if(curx<0) curx=0;
  if(curx>79) curx=79;
  if(cury<0) cury=0;
  if(cury>23) cury=23;
  }
void hydroid_clrtoeol() { 
  if(cury<24)
  for(int x=curx; x<80; x++)
    colat[cury][x] = ccol, charat[cury][x] = ' ';
  }

void hydroid_refresh(int);
int ghch(int);

#ifdef FAKE
void hydroid_refresh(int context) {
  for(int y=0; y<24; y++) for(int x=0; x<80; x++) {
    move(y,x);
    int co = colat[y][x];
    if(co < 8) attrset(COLOR_PAIR(co));
    else if(co == 8) attrset(COLOR_PAIR(co) | A_BOLD);
    else attrset(COLOR_PAIR(co-8) | A_BOLD);
    addch(charat[y][x]);
    }
  refresh();
  }
#endif

#define addstr    hydroid_addstr
#define addch     hydroid_addch
#define refresh   hydroid_refresh
#define clrtoeol  hydroid_clrtoeol
#define move      hydroid_move
#define erase     hydroid_erase
#define endwin    hydroid_endwin
#define halfdelay hydroid_halfdelay

int hydroid_main(int argc, char **argv);

#ifdef FAKE
#define ANDROID

void initScreen() { }

void highscore(int kind, int val) {}
// void achievement(const char *buf) {}
void share(const string& s) {}

void shareBe(const string &s) {
  }    

void shareFixed(const string &s) {
  }    

void shareS(const string& verb, const string &s) {
  }    

int main(int argc, char ** argv) {
  initscr(); noecho(); keypad(stdscr, true); 
  start_color(); use_default_colors();

  #define COLOR_DEFAULT -1
  init_pair(0, COLOR_BLACK,   COLOR_DEFAULT);
  init_pair(1, COLOR_BLUE,    COLOR_DEFAULT);
  init_pair(2, COLOR_GREEN,   COLOR_DEFAULT);
  init_pair(3, COLOR_CYAN,    COLOR_DEFAULT);
  init_pair(4, COLOR_RED,     COLOR_DEFAULT);
  init_pair(5, COLOR_MAGENTA, COLOR_DEFAULT);
  init_pair(6, COLOR_YELLOW,  COLOR_DEFAULT);
  init_pair(7, COLOR_WHITE,   COLOR_DEFAULT);
  init_pair(8, COLOR_BLACK,   COLOR_DEFAULT);
  
  // hydroiddebug = fopen("hydradebug.txt", "wt");

  hydroid_main(argc, argv);
  }

#define main hydroid_main
#include "hydroid/hydra.cpp"

int ghch(int context) { 
  hydroid_refresh(context); return getch();
  }


#endif

#ifndef FAKE

void cbreak() { }
void nocbreak() { }

// MAIN

#include <jni.h>
#define KEY_F0 1024

int initScreen() {}

void highscore(int kind, int val) {}
//void achievement(const char *buf) {}
void share(const string& s) {}
void shareBe(const string &s) {}    
void shareFixed(const string &s) {}    
void shareS(const string& verb, const string &s) {}    

#define main hydroid_main
#define NOCURSES
#include "hydroid/hydra.cpp"

#include <string.h>
#include <jni.h>

JNIEnv *env;
jobject thiz;

extern "C" 
int
Java_com_roguetemple_hydroid_HydroidGame_jniMain( JNIEnv* _env,
                                                  jobject _thiz )
{
    env = _env;
    thiz = env->NewGlobalRef(_thiz);
    
    savename  = "/sdcard/hydroid/hydra.sav";
    logname   = "/sdcard/hydroid/hydralog.txt";
    scorename = "/sdcard/hydroid/hydrascores.sav";
    // hydroiddebug = fopen("/sdcard/hydroid/hydradebug.txt", "wt");

    if(gameExists) {
      erase();
      move(1,1);
      col(15);
      addstr("GAME EXISTS!");
      hydroid_refresh(-3);
      return 1;
      }
    
    FILE *f = fopen(CRASHFILE, "rt");
    if(f) {
      unlink(CRASHFILE);
      fclose(f);
      return 2;
      }
    
    f = fopen(CRASHFILE, "wt"); fclose(f);

    hydroid_main(0, NULL);

    env->DeleteGlobalRef(thiz);
    
    /* hydra * h= new hydra(HC_VAMPIRE, 666, 10, 20);
    string s = h->name();
    return env->NewStringUTF(s.c_str());
    */
    
    /* initGame();
    createLog(true);
    string s;
    for(int i=0; i<size(glog); i++) s += glog[i]; */

    /* initGame();
    los();
    drawScreen();
    hydroid_refresh(); */
    
    /* return env->NewStringUTF(s.c_str()); */
}

extern "C" 
void
Java_com_roguetemple_hydroid_HydroidGame_loadMap( JNIEnv* env,
                                                  jobject _thiz, jbyteArray _array)
{
    jbyte* data = env->GetByteArrayElements(_array, NULL);
    if(data != NULL) {
      int id = 0;
      for(int y=0; y<24; y++) for(int x=0; x<80; x++) {
        data[id++] = charat[y][x];
        data[id++] = colat[y][x];
        }
      env->ReleaseByteArrayElements(_array, data, JNI_ABORT);
      }
    env->DeleteLocalRef(_thiz);
    env->DeleteLocalRef(_array);
}

/*
extern "C" 
jchar
Java_com_roguetemple_hydroid_HydroidGame_mapCharAt( JNIEnv* env,
                                                  jobject _thiz, jint x, jint y )
{
    env->DeleteLocalRef(_thiz);
    return charat[y][x];
}

extern "C" 
jchar
Java_com_roguetemple_hydroid_HydroidGame_mapColAt( JNIEnv* env,
                                                  jobject _thiz, jint x, jint y )
{
    env->DeleteLocalRef(_thiz);
    return colat[y][x];
}
*/

int ghch(int context) { 
  unlink(CRASHFILE);
  if(panic) return PANIC;
  if(gameExists) emSaveGame();
  jclass cls = env->GetObjectClass(thiz);
  jmethodID mid = env->GetMethodID(cls, "getKey", "()I");
  /*drawScreen();*/ hydroid_refresh(context);
  jint i = env->CallIntMethod(thiz, mid);
  env->DeleteLocalRef(cls);
  if(i == -2) {
    if(gameExists) saveGame();
    panic = true;
    return PANIC;
    }
  // P.debugmode = true;
  return i;
  }

void hydroid_refresh(int context) {
  if(panic) return;
  jclass cls = env->GetObjectClass(thiz);
  
  if(context < 16) {
  jmethodID midRadius = env->GetMethodID(cls, "getRadius", "()I");
  int radius = midRadius ? env->CallIntMethod(thiz, midRadius) : -1;

  jmethodID midDraw = env->GetMethodID(cls, "drawAt", "(IIIIIIIIII)V");
  
  if(!midDraw) radius = -1;
  
  for(int x=-radius; x<=radius; x++)
  for(int y=-radius; y<=radius; y++) {
    cell& c(M[playerpos + vec2(x,y)]);
    int xy = (playerpos + vec2(x,y)).x - 2 * (playerpos + vec2(x,y)).y;
    xy %= 5; xy += 5; xy %= 5;
    int hc;
    if(c.h) hc = c.h->heads;
    else hc = c.mushrooms;
    int hcolor;
    if(c.h) {
      if(c.h->color == HC_ETTIN) hcolor = -2;
      else if(c.h->color == HC_MONKEY) hcolor = -2;
      else if(c.h->color == HC_TWIN || c.h->color == HC_TWIN_R) hcolor = -3;
      else if(c.h->color == HC_SHADOW) hcolor = 0, hc = 0;
      else hcolor = c.h->gcolor();
      }
    else hcolor = -1;
    int ctype = c.type;
    int eqtype = 0, eqcolor = 0;
    if(c.it && c.it->asWpn()) {
      weapon *w = c.it->asWpn();
      eqtype = w->type;
      eqcolor = w->gcolor();
      }
    if(c.it && c.it->asItem()) {
      item *it = c.it->asItem();
      eqtype = it->type;
      if(eqtype < 7) eqtype = -1;
      else if(eqtype < 10 || eqtype == IT_HINT) eqtype = -2;
      else eqtype = -3;    
      eqcolor = it->gcolor();
      }
    int deadcolor = 0;
    if(c.dead) deadcolor = hyinf[c.dead-1].color;
    int flags = 0;
    if(c.seen) flags |= 1; 
    if(c.explored) flags |= 2;
    if(c.ontarget) flags |= 4;
    if(&c == &M.out) {
      flags = 3;
      if(DIRS == 6 && ((x+y)&1)) flags = 0, ctype = CT_HEXOUT;
      }
    if(P.curHP <= 0) flags |= 8;
    if(P.race == R_CENTAUR) flags |= 32;
    if(P.race == R_NAGA) flags |= 64;
    env->CallVoidMethod(thiz, midDraw, x, y, xy, ctype, hc, hcolor, eqtype, eqcolor, deadcolor, flags);
    }
  }
  
  jmethodID mid = env->GetMethodID(cls, "refreshScreen", "(I)V");
  env->CallVoidMethod(thiz, mid, context);

  env->DeleteLocalRef(cls);
  }

#endif
