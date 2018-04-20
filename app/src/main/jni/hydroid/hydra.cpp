// Hydra Slayer roguelike
// Copyright (C) 2010-2011 Zeno Rogue

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#ifdef ANDROID
#define VER "15.0"
#define VERSION 1500
#else
#define VER "15.0"
#define VERSION 1500
#endif

#include "utils.cpp"
#include "data.cpp"
#include "classes.cpp"

void achievement(const char *buf);

#ifndef ANDROID
void highscore(int kind, int val) { }
void share(const string& s) { }

void shform(const string& s, const string& am , const string& is, const string& are) {}

void shareBe(const string &s) {
  shform(s, "am ", "is ", "are ");
  }    

void shareFixed(const string &s) {
  shform(s, "", "", "");
  }    

void shareS(const string& verb, const string &s) {
  shform(s, verb, verb+"s", verb);
  }    


#endif

int loglines    = 10; // last messages in the log file
string savename  = "hydra.sav";
string backname  = "hydra-bak.sav";
const char *logname   = (char*) "hydralog.txt";
const char *scorename = (char*) "hydrascores.sav";

int twindiff() { 
  return P.active[IT_PFAST] - P.twinspd;
  }

string twinName(bool tw, int mode) {
  // mode 0: normal
  // mode 1: cap
  // mode 2: short
  string words[6] = {
    "you", "your twin", "You", "Your twin", "you", "twin"
    };
  if(P.twinsNamed) {
    string s = pinfo.twin[tw?1:0];
    if(mode == 1) s[0] = toupper(s[0]);
    return s;
    }
  else
    return words[(tw?1:0)+mode*2]; 
  }

string twinForm(string verb, string suffix) {
  if(P.twinsNamed)
    return twinName(false, 1)+" "+verb+suffix+" ";
  else
    return "You "+verb+" ";
  }

void twinswap() {
  if(!twin) return; // something wrong

  swap(twin->pos, playerpos);
  M[playerpos].h = NULL;
  M[twin->pos].h = twin;
  swap(P.curHP, twin->heads);
  swap(P.maxHP, P.twinmax);
  P.twinarms ^= (1<<P.arms) - 1;
  swap(P.twincarm, P.cArm);
  swap(P.active[IT_PFAST], P.twinspd);
  swap(P.active[IT_PSEED], P.twinseed);
  swap(pinfo.twin[0], pinfo.twin[1]);
  }

bool phaseswappable() {
  int tdiff = twindiff();
  if(tdiff <= 0) return true;
  return ((P.phase >> tdiff) << tdiff) == P.phase;
  }

void twinswap_phase() {
  if(!twin) return; // something wrong
  int tdiff = twindiff();
  if(tdiff > 0) P.phase >>= tdiff;
  if(tdiff < 0) P.phase <<= -tdiff;
  twinswap();
  }

hydra *twinAlive() {
  if(twin)
    return twin;
  for(int i=0; i<size(stairqueue); i++)
    if(stairqueue[i]->color == HC_TWIN)
      return stairqueue[i];
  return NULL;
  }

string form(hydra *who, string verb) {

  // crush -> crushes
  if(who && verb[verb.size()-1] == 'h') 
    verb += "e";
  
  if(twin && (who == twin || (!who && P.twinsNamed))) {
    return twinName(P.twinsNamed ? 0 : who,1) + " " + verb + "s";
    }
  else if(who && !who->ewpn && verb != "kill") {
    verb = who->info().hverb;
    if(who->color & HC_DRAGON)
      verb = who->info().dverb;
    return "The " + who->name() + " " + verb;
    }
  else if(who) return "The " + who->name() + " " + verb + "s";
  else return "You " + verb;
  }

void singlestep() {
  popStairQueue();
  cancelVorpal();

  int tdiff = twindiff();

  P.phase++;
  if(twin && ((tdiff < 0) || ((P.phase >> tdiff) << tdiff) == P.phase)) {
    if(P.twinmode == 0) {
      int twintimes = tdiff <= 0 ? (1 <<-tdiff) : 1;
      while(twintimes--) {
        dfs(2);
        if(twin->power()) moveHydra(twin);
        }
      }
    else {
      twinswap_phase();

      P.twinmode = 3-P.twinmode;
      if(P.twinmode == 2) {
        P.phase--;
        return;
        }
      }
    }

  while(P.active[IT_PFAST] <= 32 && P.phase >= (1<<P.active[IT_PFAST])) {
    P.phase = 0; moveHydras();
    }
  }

void cancelspeed() { 
  if(P.active[IT_PFAST]) {
    P.phase--;
    P.phase >>= 1;
    P.active[IT_PFAST]--;
    }
  singlestep();
  }

#include "level.cpp"
#include "weapons.cpp"
#include "monster.cpp"
#include "tutorial.cpp"

bool checkShadow(cell& c) {
  if(!c.h) return false;
  if(c.h->visible()) return false;
  addMessage("You miss the "+c.h->name()+".");
  cancelspeed();
  if(P.ambifresh && P.ambifresh == P.active[IT_PAMBI])
    P.ambifresh--;
  return true;
  }

string item::getname() {
  if(type < IT_SCRO) return "a vase filled with " + name();
  else return "a " + name();
  }

item *asItem(sclass *o) {
  if(!o) return NULL;
  if(o->sct() != SCT_ITEM) return NULL;
  return (item*) o;
  }

void movedir(int dir) {

  if(P.curHP <= 0) {
    addMessage("You are dead! Press 'q' to proceed.");
    exploreOn = false;
    return;
    }
  
  if(P.active[IT_PAMBI] && ((P.ambiArm & ~P.twinarms) == 0)) {
    addMessage("Select one of your weapons, okay?");
    return;
    }
  
  vec2 npos = playerpos + dirs[dir];
  cell& c(M[npos]);

  if((P.flags & dfAutoON) && c.mushrooms) {
    hydra h(HC_MUSH, c.mushrooms, 1, 0);
    if(!chooseAutoAttack(&h)) return;
    }

  if((P.flags & dfAutoON) && c.h) {
    if(!chooseAutoAttack(c.h)) return;
    }

  stunnedHydra = NULL;
  weapon *W = wpn[P.cArm];

  cell *shc = &c;
  
  if((!exploreOn) && tryLineAttack(dir, !P.manualfire, false)) return;
  
  int hc = 0;
  
  int haveswipe = 0;

  if(W && (W->msl()) && (c.h || c.mushrooms)) {
    addMessage("Missile weapons have to be fired ('t')!");
    return;
    }
    
  if(W) {
    if(P.race == R_ELF && W->cuts() && !W->axe()) haveswipe = 1;
    if(W->type == WT_DANCE) haveswipe |= 2;
    if(P.active[IT_PSWIP] && W->swipable()) haveswipe |= 4;
    if(P.race == R_CENTAUR) haveswipe = haveswipe == 6 ? 4 : 0;

    if(haveswipe && W->csdx()) {
      int dd = P.cArm & 1 ? 7 : 1;
      int wstr = W->size;
      for(int d=0; d<DIRS; d++) {
        cell& c2(M[playerpos + dirs[(dir + d * dd) % DIRS]]);
        if(c2.h && c2.h->invisible()) shc = &c2;
        int hat = c2.headsAt(W);
        if(d > 0) {
          if(hc < wstr) haveswipe |= 8;
          if(hc < wstr && hc+hat > wstr) haveswipe |= 16;
          }
        hc += hat;
        }
      }
    else
      hc = c.headsAt(W);
    }

  decomposer D(W, hc);
  
  int mushlord = (W && W->type == WT_FUNG) ? W->size : 0;
  
  if(mushlord && c.mushrooms) {
    if(c.mushrooms > 2)
      addMessage("The mushrooms join their "+mushroomname(mushlord)+"!");
    else if(c.mushrooms > 1)
      addMessage("The mushrooms move in front of you!");
    else
      addMessage("The mushroom bows to you!");
    P.active[IT_PSEED] += c.mushrooms-1;

    W->sc.sc[WS_MKILL] ++;
    W->sc.sc[WS_MHEAD] += c.mushrooms;

    if(!W->sc.sc[WS_USE]) shareBe("crushing mushrooms with the " + W->name());
    if(P.active[IT_PSEED] > stats.maxseed) stats.maxseed = P.active[IT_PSEED];
    achievement("MUSHROOMSTAFF");
    c.mushrooms = 0;
    }

  if(c.h && c.h == twin) {
    M[playerpos].h = twin;
    c.h = NULL;
    swap(twin->pos, playerpos);
    singlestep();
    exploreOn = false;

    if(c.type == CT_STAIRDOWN)
      addMessage("Press 'g' to climb down the stairs.");
    else if(c.it)
      addMessage("You see "+c.it->getname()+" here.");
    }

  else if(W && W->type == WT_QUAKE && (c.type == CT_WALL || hc)) {
    weapon *w = W;
    if(w->size == 0) {
      addMessage("Club need recharging. Need Scroll of Big Stick.");
      checkShadow(*shc);
      return;
      }
    w->size--;
    string exclamation = "!";
    int i = w->color; while(i != HC_ALIEN) exclamation += "!", i--;
    if(c.type == CT_WALL)
      addMessage("The level shakes as you hit the wall"+exclamation);
    else if(c.h)
      addMessage("As you hit the "+c.h->name()+", a huge shockwave is released"+exclamation);
    else
      addMessage("The poor mushroom shakes and the ground trembles"+exclamation);
    for(int i=0; i<size(hydras); i++) {
      hydra *h (hydras[i]);
      h->sheads = h->heads;
      if(h->lowhead())
        h->stunforce += h->heads * (w->info().stunturns + 5) / 10;
      else {
        h->stunforce += quakefun(h->heads, w->color);
        }
      w->sc.sc[WS_HKILL] ++;
      w->sc.sc[WS_HHEAD] += h->heads;
      }
    if(!w->sc.sc[WS_USE]) shareBe("using the " + w->name());
    w->sc.sc[WS_USE] ++;
    achievement("HYDRAQUAKES");
    }
  
  else if(W && W->type == WT_RAND && c.h) {
    mersenneTwist(W, c.h);
    }
  
  else if(W && W->type == WT_RAND && hc) {
    hydra h(HC_MUSH, c.mushrooms, 1, 0);
    mersenneTwist(W, &h);
    c.mushrooms = h.heads;
    }
  
  else if(W && W->wand() && (c.type == CT_WALL || hc)) {
    if(c.type != CT_WALL) {
      if(!checkShadow(*shc)) 
        addMessage("Your "+W->name()+" can be used only on walls.");
      return;
      }
    tryWandUse(W, dir, false);
    }

  else if(W && W->type == WT_PICK && c.type == CT_WALL) {
    if(!inlevel(wrap(npos))) {
      addMessage("Destroying outer walls of magical dungeons is dangerous. Won't do that!");
      return;
      }
    c.type = CT_EMPTY;
    addMessage("You crush the wall with your "+W->name()+"!");
    stats.ws[MOT_PICK].sc[WS_USE]++;
    stats.ws[MOT_PICK].sc[WS_GROW]++;
    if(!W->sc.sc[WS_USE]) shareBe("crushing walls with the " + W->name());
    W->sc.sc[WS_USE]++;
    W->sc.sc[WS_GROW]++;
    if(W->size < 3) cancelspeed();
    if(W->size < 5) cancelspeed();
    }
  else if(W && W->type == WT_PICK && c.mushrooms) {
    stats.ws[MOT_PICK].sc[WS_USE]++;
    stats.ws[MOT_PICK].sc[WS_MKILL] ++;
    stats.ws[MOT_PICK].sc[WS_MHEAD] += c.mushrooms;
    if(!W->sc.sc[WS_USE]) shareBe("crushing mushrooms with the " + W->name());
    W->sc.sc[WS_USE]++;
    W->sc.sc[WS_MKILL] ++;
    W->sc.sc[WS_MHEAD] += c.mushrooms;
    c.mushrooms = 0;
    addMessage("You crush the mushrooms with your "+W->name()+"!");
    if(W->size < 2) cancelspeed();
    }
  else if(c.type == CT_WALL) {
    addMessage("A wall blocks your path!");
    }
  
  else if(!c.h && (!c.mushrooms)) {

    if(c.dead > 10 && P.active[IT_RNECR]) {
      if(c.dead-1 == HC_TWIN) {
        addMessage(twinName(0,1) + " would not do this to "+twinName()+"!");
        }
      else
        addMessage("Cannot raise special enemies as zombies.");
      }

    if(c.dead && c.dead <= 10 && P.active[IT_RNECR] && P.curHP > 1) {
      int mush = 0;
      for(int y=0; y<SY; y++) for(int x=0; x<SX; x++) if(M.m[y][x].seen) {
        mush += M.m[y][x].mushrooms;
        M.m[y][x].mushrooms = 0;
        }
      
      if(mush > 0) {
        stats.necro += mush;
        hydra *h = new hydra(c.dead-1, mush, 1, 1);
        P.maxHP--; P.curHP--;
        h->zombie = true;
        h->pos = npos; c.h = h;
        hydras.push_back(h);
        c.dead = 0; P.active[IT_RNECR]--;
        for(int i=0; i<COLORS; i++) h->res[i] = 0;
        shareS("raise", " " + h->name());
        addMessage("You raise a hydra zombie!");
        if(mush >= 200000) achievement("NECROPOWER");
        return;
        }
      }
    
    if(W && W->type == WT_SPEED) {
      for(int u=1; u<W->size; u++) {
        npos += dirs[dir];
        if(!M[npos].isPassable()) {
          if(!checkShadow(M[npos]))
            addMessage("Not enough space to fly in this direction.");
          return;
          }
        }
      int hpcost = 1;
      if(P.race == R_TROLL) hpcost = 3;
      P.curHP -= hpcost; stats.broomhp += hpcost;
      stats.broomdist += W->size;
      }
    
    if(P.active[IT_PSEED] && M[playerpos].type == CT_EMPTY) {
      int q = (P.active[IT_PSEED]+6) / 7;
      P.active[IT_PSEED]-=q;
      M[playerpos].mushrooms+=q;
      }
    playerpos = npos;
    
    if(P.race == R_NAGA) {
      nagavulnerable = true;
      singlestep();
      nagavulnerable = false;
      singlestep();
      }
    else
      singlestep();
        
    if(P.active[IT_PAMBI] > P.ambifresh) P.active[IT_PAMBI]--;
    
    if(M[npos].type == CT_STAIRDOWN)
      addMessage("Press 'g' to climb down the stairs.");
    else if(asItem(M[npos].it) && asItem(M[npos].it)->type == IT_HINT)
#ifdef ANDROID    
      addMessage("Touch MENU then 'get/use' to read");
#else
      addMessage("Press 'g' to read the information in this scroll.");
#endif
    else if(M[npos].it)
      addMessage("You see "+M[npos].it->getname()+" here.");
    }
  
  else if(c.mushrooms && P.active[IT_RGROW]) {
    hydra h(HC_MUSH, c.mushrooms, 1, 0);
    // h.sheads = h.heads;
    analyzeHydra(&h);
    stats.mushgrow++;
    if(growHeads(&h)) {
      c.mushrooms = h.heads;
      // giveHint(&h);
      addMessage("The mushroom grows!");
      }
    else addMessage("You could not even cut a mushroom with your current weapons!");
    P.active[IT_RGROW]--;
    }
  
  else if(c.mushrooms && P.active[IT_RDEAD]) {
    // h.sheads = h.heads;
    stats.ws[MOT_DECAP].sc[WS_MKILL]++;
    stats.ws[MOT_DECAP].sc[WS_MHEAD] += c.mushrooms;
    c.mushrooms = 0;
    addMessage("Your powder instantly destroys a patch of mushrooms!");
    P.active[IT_RDEAD]--;
    }
  
  else if(c.mushrooms && bitcount(P.ambiArm) > 1 && P.active[IT_PAMBI]) 
    ambiAttackFull(&c, D);
  
  else if(c.mushrooms && !W) {
    stats.ws[MOT_BARE].sc[WS_USE] ++;
    c.mushrooms--;
    if(c.mushrooms)
      addMessage("You crush one of the mushroom's heads.");
    else
      addMessage("You crush the mushroom with your hands."),
      stats.ws[MOT_BARE].sc[WS_MKILL] ++;
    cancelspeed();
    cancelspeed();
    }

  else if(c.mushrooms && W->activeonly())
    addMessage("Magical blunt weapons and axes don't work on mushrooms.");

  else if(c.h && P.active[IT_RSTUN] && c.h->sheads != c.h->heads) {
    c.h->stunforce += 5 * c.h->heads + 1000;
    stats.magestun += (c.h->heads - c.h->sheads);
    c.h->sheads = c.h->heads;
    addMessage("Sparks fly and the "+c.h->name()+" is stunned!");
    P.active[IT_RSTUN]--;
    stunnedHydra = c.h; stunnedColor = iinf[IT_RSTUN].color;
    if(c.h->color == HC_VAMPIRE) {
      stats.unhonor++;
      addMessage("You are almost stunned by the thought of a similar thing happening to you!");
      }
    }
  
  else if(c.h && P.active[IT_RDEAD] && c.h->sheads) {
    P.active[IT_RDEAD]--;
    if(c.h->isAncient()) {
      addMessage("The "+c.h->name()+" is too powerful to be affected by this puny magic!");
      return;
      }
    stats.ws[MOT_DECAP].sc[WS_HHEAD] += c.h->sheads;
    if(c.h->heads > c.h->sheads) {
      if(c.h->color == HC_VAMPIRE) {
        stats.unhonor++;
        addMessage("You look in terror! These disappearing heads could be yours as well!");
        }
      else
        addMessage("The "+c.h->name()+" looks in terror as its stunned heads disappear!");
      c.h->heads -= c.h->sheads;
      c.h->sheads = 0; c.h->stunforce = 0;
      }
    else {
      addMessage("Your death touch kills the poor sleeping "+c.h->name()+"!");
      if(c.h->color == HC_VAMPIRE) {
        stats.unhonor++;
        addMessage("You hope you won't lose your own head to such a stupid magical trick.");
        }
      c.hydraDead(NULL);
      stats.ws[MOT_DECAP].sc[WS_HKILL]++;
      }
    }

  else if(c.h && P.active[IT_RCANC]) {
    if(c.h->color == HC_ALIEN) {
      addMessage("The "+c.h->name()+" is not affected!");
    } else if(c.h->isAncient()) {
      for(int i=0; i<HCOLORS; i++) c.h->res[i] /= 2;
      addMessage("The "+c.h->name()+" partially resists your spell!");
      }
    else {
      for(int i=0; i<HCOLORS; i++) c.h->res[i] = 0;
      addMessage("The "+c.h->name()+" looks somehow normal now!");
      c.h->clearswnd(); 
      }
    P.active[IT_RCANC]--;
    }
  
  else if(c.h && P.active[IT_RGROW]) {
    string origname = c.h->name();
    analyzeHydra(c.h);
    int orig = c.h->heads;
    stats.magegrow -= c.h->heads;
    if(growHeads(c.h)) {
      addMessage("The "+origname+" suddenly grows some new heads!");
      if(c.h == vorplast) vorpalClear();
      }
    else {
      addMessage("Sorry, growing additional heads won't help you with "+origname+".");
      }
    stats.magegrow += c.h->heads;
    orig = c.h->heads - orig;
    if(orig >= 10000) achievement("GROWTHMASTER");
    if(orig >= 1000) shareS("grow", " the " + c.h->name() + " (" + its(orig)+ " new heads)");
    P.active[IT_RGROW]--;
    }
  
  else if(c.h && P.active[IT_RCONF] && !c.h->conflict) {
    bool resist = c.h->color == HC_VAMPIRE;
    if(!resist)
      c.h->conflict = true;
    addMessage(
      resist ? "The mighty "+c.h->name()+" resists your magic!" :
      c.h->isAncient() ? "Wow, the mighty "+c.h->name()+" now looks anciently confused!" :
      c.h->lowhead() ? "The "+c.h->name()+" suddenly seems to dislike hydras for some reason!" :
      c.h->power() ?  "The "+c.h->name()+" now looks confused!" : 
      "The "+c.h->name()+" should be confused once it wakes up!");
    P.active[IT_RCONF]--;
    }
  
  else if(c.h && P.active[IT_RFUNG]) {
    P.active[IT_RFUNG]--;
    if(c.h->isAncient()) {
      addMessage("The "+c.h->name()+" is too powerful to be affected by this mushroom magic!");
      return;
      }
    c.mushrooms = c.h->heads;
    if(c.h->color == HC_ALIEN)
      addMessage("The "+c.h->name()+" turns into an otherworldly mushroom!");
    if(c.h->color != HC_VAMPIRE) 
      addMessage("The "+c.h->name()+" turns into a mushroom!");
    if(c.h->color == HC_VAMPIRE) {
      stats.unhonor++;
      addMessage("Such a mighty being falling to such a stupid magical trick? You feel uneasy.");
      }
    
    c.hydraDead(NULL);
    c.dead = 0; // not allowed to raise
    }

  else if(c.h && bitcount(P.ambiArm) > 1 && P.active[IT_PAMBI]) 
    ambiAttackFull(&c, D);
  
  else if(c.h && !W) {
    if(!checkShadow(*shc))
      addMessage("With what? Your bare hands?");
    }

  else if(hc && W->type == WT_BLADE && W->size == 0) {
    // fingernail attack!
    if(c.h) shareS("attack", " " + c.h->name() + " with the " + W->name());
    c.attack(W, 0, NULL);
    cancelspeed();
    // int orig = c.h->heads;
    achievement("ZERO");
    }
  
  else if(hc) {
    if(W->xcuts() && W->type != WT_PSLAY && !W->size) {
      if(!checkShadow(*shc)) {
        addMessage("This weapon will drive you mad!!! You decide against using it.");
        achievement("NULLSECTOR");
        shareBe("too sane to use the " + W->name());
        }
      }
    else if(W->type == WT_TIME) {
      int hc = c.mushrooms, res = 0;
      if(c.h) {
        res = c.h->res[W->color];
        if(res < 0) res = -res * W->size;
        hc = c.h->heads - c.h->sheads;
        }
      if(c.mushrooms > 10)
        addMessage("Hey, stop this! This is just boring.");
      else if(hc < res || (hc == res && !c.h->sheads))
        addMessage("Not enough active heads here...");
      else {
        c.attack(W, W->size, NULL);
        W->addStat(WS_USE, 1, 0);
        cancelspeed();
        }
      }
    else if(W->csd() && hc < W->size) {
      if(!checkShadow(*shc)) 
        addMessage("Not enough "+(W->stuns()?"active ":s0)+"heads here to use your "+W->name()+"!");
      if(c.mushrooms) addMessage("You can try dropping your weapon and crushing this mushroom barehanded.");
      }
    else if(W->xcuts() && W->cutoff(hc, false) < 0) {
      if(checkShadow(*shc)) ;
      else if(W->type == WT_DECO) {
        if(c.h)
          addMessage("The "+c.h->name()+" cannot be decomposed any further.");
        else
          addMessage("The mushroom seems decomposed enough.");
        // luckily you don't lose a charge when you miss a shadow
        }
      else if((hc&1) && !(W->size & 1) && W->type == WT_DIV)
        addMessage("It would be *odd* if your "+W->name()+" could attack this, don't you think?");
      else
        addMessage("Your "+W->name()+" cannot cut "+          
          (P.active[IT_PSWIP] ? "the surrounding heads" :
           c.mushrooms?"this mushroom":c.h->name()) + "!"
           );
      }
    else {
      int acount = 0, lastkill = W->sc.sc[WS_HKILL];
      int dd = P.cArm & 1 ? 7 : 1;
      int wstr = W->size;
      
      if(W->type == WT_PSLAY && !W->size) wstr = 1;
      
      char oldtype = W->type;
      if(W->xcuts() && ((haveswipe&7) == 4)) {
        wstr = W->cutoff(hc, false);
        oldtype = W->type;
        W->type = WT_BLADE;
        }
      
      if(haveswipe == 25) {
        if(!checkShadow(*shc)) 
          addMessage("Elven fighting style does not allow that!");
        return;
        }
      
      if((haveswipe & 24) == 24) haveswipe &= ~1;
      
      bool havedouble = false;
      
      for(int d=0; d<DIRS; d++) if(wstr) {
        cell& c2(M[playerpos + dirs[(dir + d * dd) % DIRS]]);
        int hc0;
        if((hc0 = c2.headsAt(W))) {
          acount++;
          havedouble = c2.h && c2.h->res[W->color] < 0;
          if(wpn[P.cArm]->xcuts()) {
            if(d == 0) {
              c2.attack(wpn[P.cArm], wstr, NULL), acount++;
              }
            }
          else if(hc0 < wstr)
            c2.attack(W, hc0, NULL), wstr -= hc0;
          else
            c2.attack(W, wstr, NULL), wstr = 0;
          }
        }
      
      if((haveswipe&7) == 4 && (acount > 1 || oldtype != W->type))
        P.active[IT_PSWIP]--;
      lastkill = W->sc.sc[WS_HKILL] - lastkill;
      if(lastkill > stats.maxkill) stats.maxkill = lastkill;
      if(lastkill >= 8) achievement("ALLAROUND");
      if(lastkill >= 2) shareFixed("just killed "+its(lastkill)+" enemies at once with the " + W->name());
      
      attackEffect(W, havedouble);
      cancelspeed();

      W->addStat(WS_USE, 1, 0);
      if(twin) stats.twinmy++;
      
      W->type = oldtype;

      // the mushroom staff grows mushrooms on attack
      if(mushlord) for(int i=0; i<mushroomlevel(mushlord); i++) {
        int d = rand() % DIRS;
        for(int k=0; k<DIRS; k++) {
          cell& c2(M[npos+dirs[(d+k) % DIRS]]);
          if((&c2 != &M[playerpos]) && !c2.mushrooms && c2.type == CT_EMPTY) {
            c2.mushrooms++;
            break;
            }
          }
        }
      
      D.reduce();
      }
    }
  
  else {
    if(!checkShadow(*shc)) 
      addMessage("A stunned "+c.h->name()+" blocks your way.\n");
    }
  }

bool useup(int ii) {
  /* if(ii < IT_SXMUT && P.race == R_ELF) {
    ii = IT_SXMUT;
    addMessage("You convert the Rune to a Scroll of Transmutation.");
    } */
  if((ii == IT_SGROW || ii == IT_SXMUT) && P.active[IT_PAMBI] && bitcount(P.ambiArm) > 1) {
    if(P.race == R_NAGA ? P.active[IT_PAMBI] > 1 : P.ambifresh) {
      stats.ambiscroll++;
      int c = P.cArm, aa = P.ambiArm;
      for(int i=0; i<P.arms; i++) if(havebit(aa, i)) {
        P.cArm = i; P.ambiArm = 0;
        useup(ii);
        }
      P.cArm = c; P.ambiArm = aa;
      P.active[IT_PAMBI]--; P.ambifresh--;
      return true;
      }
    }
  switch(ii) {
    case IT_SXMUT: {
      if(!transmute(wpn[P.cArm])) return false;
      break;
      }      
    
    case IT_SGROW: {
      if(wpn[P.cArm]) wpn[P.cArm]->grow();
      else {
        wpn[P.cArm] = new weapon(0, 8, WT_BLADE);
        wpn[P.cArm]->level = P.curlevel+1;
        addMessage("Your fingernail suddenly grows and falls off!");
        }
      wpnset++;
      break;
      }
    
    case IT_SPART: {
    
      if(P.active[IT_PAMBI] && bitcount(P.ambiArm) > 1) {
        bool error = false;
        if(P.race == R_NAGA ? P.active[IT_PAMBI] == 1 : !P.ambifresh)
          error = true;
        int bigsize = -1, bigat = -1;
        for(int i=0; i<P.arms; i++) if(havebit(P.ambiArm, i)) {
          if(!wpn[i]) error = true;
          else if(wpn[i]->size > bigsize) bigsize = wpn[i]->size, bigat = i;
          }
        if(!error)
          for(int i=0; i<P.arms; i++) if(havebit(P.ambiArm, i)) if(i != bigat) {
            if(wpn[i]->size & 1) error = true;
            if(wpn[i]->size >= bigsize) error = true;
            if(wpn[i]->color != wpn[bigat]->color) error = true;
            }
        if(error) {
          addMessage("For some reason, this does not seem to work.");
          return false;
          }
        addMessage("The "+wpn[bigat]->name()+" is reforged!");
        stats.ambiforge++;
        for(int i=0; i<P.arms; i++) if(havebit(P.ambiArm, i)) if(i != bigat) {
          wpn[i]->size /= 2;
          wpn[bigat]->size += wpn[i]->size;
          stats.forgegrow += wpn[i]->size;
          }
        P.active[IT_PAMBI]--; P.ambifresh--;
        wpnset++;
        return true;
        }
    
      if(!wpn[P.cArm]) return false;

      if(wpn[P.cArm]->type == WT_RAND && stats.backupsaves) {
        addMessage("Your weapon shouts about responsibility! You cannot reforge it...");
        return false;
        }

      if(M[playerpos].it && P.race != R_TROLL) {
        addMessage("You can only do this on an empty spot.");
        return false;
        }

      weapon *w = wpn[P.cArm]->reduce();

      if(w == NULL) {
        addMessage("The "+wpn[P.cArm]->name()+" is too small to be further reduced.");
        return false;
        }

      w->osize = -1;

      addMessage("A part of "+wpn[P.cArm]->name()+" falls off!");
      M[playerpos].it = w;
      wpn[P.cArm]->size -= w->size;

      wpnset++;
      break;
      }
    
    case IT_PKNOW: {
      for(int i=0; i<size(hydras); i++) if(hydras[i]->aware() && hydras[i]->dirty) {
        hydras[i]->dirty = 0;
        addMessage("You recognize the " + hydras[i]->name()+" under the blood.");
        return true;
        }
      for(int i=0; i<size(hydras); i++) 
        if(hydras[i]->aware()) {
          giveHint(hydras[i]);
          }
      addMessage("All Hydras in sight analyzed.");
      break;
      }
    
    case IT_PLIFE: {
      if(P.race == R_TWIN && !twinAlive()) {
        vec2 twinpos;
        int i = 0;
        for(i=0; i<DIRS; i++) {
          twinpos = playerpos + dirs[i];
          cell& c(M[twinpos]);
          if(c.isPassable()) break;
          }
        if(i == DIRS && wrap(playerpos) != wrap(stairpos)) {
          addMessage("No room here to save "+twinName()+".");
          return false;
          }
        else if(wrap(playerpos) == wrap(stairpos)) {
          stairqueue.push_back(new hydra(HC_TWIN, 1, 1, 0));
          }
        else {
          (new hydra(HC_TWIN, 1, 1, 0))->putOn(twinpos);
          }
        for(int y=0; y<SY; y++) for(int x=0; x<SX; x++) {
          cell& c(M.m[y][x]);
          if(c.dead == HC_TWIN+1) c.dead = 0;
          }
        if(P.twinsNamed)
          addMessage(twinForm("save","s") + twinName()+"!");
        twinswap();
        achievement("LIFESAVER");
        }
      addMessage("You feel full of life!");
      P.maxHP *= 2;
      P.curHP = P.maxHP;
      /*P.twinmax *= 2;
      if(twinAlive()) {
        twinAlive()->heads = P.twinmax;
        } */
      break;
      }
    
    case IT_PSEED:
      P.active[IT_PSEED] += 7;
      if(wpn[P.cArm] && wpn[P.cArm]->type == WT_FUNG) {
        P.active[IT_PSEED] += 7 * wpn[P.cArm]->size;
        addMessage("You feel the power of the Mushroom "+mushroomname(wpn[P.cArm]->size)+" inside you!");
        }
      else 
        addMessage("You feel like a source of mushroom life!");
      if(P.active[IT_PSEED] > stats.maxseed) stats.maxseed = P.active[IT_PSEED];
      if(stats.maxseed >= 50) achievement("SEEDMASTER");
      break;

    case IT_PARMS:
      P.arms++;
      addMessage(P.arms & 1 ? "You suddenly grow a new right arm!" : "You suddenly grow a new left arm!");
      if(stats.powerignore > P.inv[IT_PARMS]-1 && P.race != R_TROLL)
        stats.powerignore = P.inv[IT_PARMS] - 1;
      break;
    
    default:
      if(ii == IT_PCHRG && P.race == R_NAGA) {
        if(P.active[IT_PFAST])
          P.active[IT_PFAST]--;
        else {
          addMessage("Children of Echidna are too slow to charge! Drink a Potion of Speed first.");
          return false;
          }
        }
      
      if(ii == IT_PAMBI && P.race == R_TWIN) {
        if(!twin) {
          if(twinAlive())
            addMessage("No ambidexterity alone!");
          else
            addMessage("Respect your dead twin! You would never drink this alone!");
          return false;
          }
        if(P.inv[IT_PAMBI] == 1) {
          addMessage("You have only one of these! What about "+twinName()+"?");
          return false;
          }
        P.inv[ii]--, stats.usedup[ii]++;
        if(!stats.woundwin) stats.usedb[ii]++;
        }

      P.active[ii]++;
      if(ii >= IT_POTS)
        addMessage("You drink the "+iinf[ii].name+"!");
      else 
        addMessage("You prepare the "+iinf[ii].name+"!");

      if(ii == IT_PAMBI && P.race == R_ELF) {
        addMessage("Elves' superior fighting style does not work with this potion, but...");
        }
      
      if(ii == IT_PAMBI && P.arms == 2) {
        P.ambiArm = 3;
        }
      
      if(ii == IT_PAMBI && P.race == R_NAGA) {
        /*
        if(P.curlevel < 50)
          P.curHP += P.curlevel + 1;
        else
          P.curHP += 250 / (P.curlevel - 45);
        if(P.curHP > P.maxHP) P.curHP = P.maxHP;
        addMessage("Ahh, that feels good.");
        return true;
        */
        addMessage("Your scrolls are now able to affect multiple weapons at once!");
        }
      
      if(ii == IT_PSWIP && P.race == R_ELF) {
        addMessage("Your Elven skill of attacking multiple targets feels more precise.");
        }
      
      if(ii == IT_PSWIP && P.race == R_CENTAUR) {
        addMessage("Not very useful for a Centaur...");
        }
      
      if(ii == IT_PAMBI && P.race == R_ELF) {
        addMessage("Elves' superior fighting style does not work with this potion, but...");
        }
      
      if(ii == IT_PAMBI && P.race == R_TWIN) {
        addMessage("Note: twins can decide to have a joint ambidextrous attack.");
        addMessage("Note: pressing 'c' trades weapons instead of giving control to AI.");
        if(P.twinmode == 0) P.twinmode = 1;
        }
      
      if(P.active[IT_PFAST] > stats.maxspeed) stats.maxspeed = P.active[IT_PFAST];
      if(P.active[IT_PFAST] > 10) achievement("SPEEDOFLIGHT");
      if(P.active[IT_PFAST] > 4)
        shareS("drink", " "+its(P.active[IT_PFAST])+" Potions of Speed at once");
      
      if(ii == IT_PFAST) P.phase *= 2;
      if(ii == IT_PAMBI) P.ambifresh++;
      
      int arune = 0;
      for(int i=0; i<IT_POTS; i++) arune += P.active[i];
      if(arune > stats.maxrune) stats.maxrune = arune;
      if(arune >= 5) achievement("RUNEMASTER");
      break;
    }
  
  return true;
  }

bool canGoDown() {
  
  for(int i=0; i<size(hydras); i++)
    if(!hydras[i]->lowhead() && !hydras[i]->zombie)
      return false;

  return true;
  }

void totalKnowledge() {
  addMessage("You have a dream about killing 100 hydras on the next 10 levels...");
  drawScreen(); refresh(IC_VIEWDESC);
  int killed = 0, wounds = 0;
  
  if(P.curlevel+1 < LEVELS) {
    addstri("This feature works only below the boss level.");
    return;
    }
  
  int ocl = P.curlevel;
  
  for(int i=0; i<100; i++) {
    P.curlevel += 1 + (i/10);
    hydra* h = new hydra(i % HCOLORS, rand() % (200 + 5 * P.curlevel) + 1, 13, 240/(P.curlevel-7));
    
    analyzeHydra(h);
    int spos; encode(h->heads, h->sheads, spos);
    
    if(h->heads < AMAXS && wnd[spos] < WMAX) {
      killed++;
      wounds += wnd[spos];
      }
    delete h;
    }
  P.curlevel = ocl;
  addMessage("You have killed "+its(killed)+" of them taking "+its(wounds)+" wounds.");
  }

#include "save.cpp"
#include "achieve.cpp"
#include "mainmenu.cpp"
#include "ui.cpp"

void initGame() {

  if(!stats.savecount) {
    P.gameseed = time(NULL);
    }

  generateGame();
  
  stats.tstart = time(NULL) - stats.tstart;
  
  if(!stats.savecount) {
    P.gameseed = time(NULL);
    for(int ii=0; ii<ITEMS; ii++) P.inv[ii] = 0;
    P.arms = 2; P.cArm = 0; P.ambiArm = 3;
    P.maxHP = 24; P.curHP = 24;
    P.version = VERSION;
    stats.powerignore = 100;
    stats.gamestart = time(NULL);
    
    P.arms = 0;
        
    share("I just started playing Hydra Slayer!");

    shareS("enter", " the Hydras Nest");
    P.arms = 2;

    generateLevel();
    if(P.race == R_ELF || P.race == R_CENTAUR) {
      delete(wpn[0]);
      wpn[0] = new weapon(HC_ALIEN, 1, WT_BOW);
      P.cArm = 1;
      }

    int it3[] = {IT_PFAST, IT_PSEED, IT_RCANC};

    if(P.flags & dfTutorial) {
      // tutorial starting items
      P.inv[IT_RSTUN]+=2;
      P.inv[IT_RDEAD]+=2;
      }

    else if(P.race != R_TROLL) {
      P.inv[IT_RSTUN]++;
      P.inv[IT_RDEAD]++;        
      P.inv[it3[rand() % 3]]++;
      }

    else {
      // generate a size 3 stunner
      pinfo.trollwpn.push_back(new weapon(HC_ANCIENT, 3, WT_BLUNT));
      pinfo.trollkey.push_back('h');
      }
    
    if(P.race == R_TWIN) {
      stairqueue.push_back(new hydra(HC_TWIN, 12, 1, 0));
      P.maxHP = 12; P.curHP = 12;
      P.twinarms = 2; P.twinmax = 12; P.twincarm = 1;
      }

    if(debugon()) {
      for(int ii=0; ii<ITEMS; ii++) P.inv[ii] = 666;
      pinfo.trollwpn.push_back(new weapon(0, 2, WT_SHLD));
      pinfo.trollwpn.push_back(new weapon(1, 0, WT_DIV));
      pinfo.trollwpn.push_back(new weapon(2, 2, WT_DIV));
      pinfo.trollwpn.push_back(new weapon(3, 3, WT_DIV));
      pinfo.trollwpn.push_back(new weapon(4, 5, WT_DIV));
      pinfo.trollwpn.push_back(new weapon(5, 10, WT_DIV));
      pinfo.trollwpn.push_back(new weapon(6, 20, WT_BLADE));
      pinfo.trollwpn.push_back(new weapon(7, 2, WT_ROOT));
      pinfo.trollwpn.push_back(new weapon(8, 4, WT_MSL));
      pinfo.trollwpn.push_back(new weapon(9, 15, WT_MSL));
      pinfo.trollwpn.push_back(new weapon(0, 999, WT_BLADE));
      pinfo.trollwpn.push_back(new weapon(1, 999, WT_MSL));
      pinfo.trollwpn.push_back(new weapon(2, 3, WT_DANCE));
      pinfo.trollwpn.push_back(new weapon(3, 30, WT_DANCE));
      pinfo.trollwpn.push_back(new weapon(4, 1, WT_VORP));
      pinfo.trollwpn.push_back(new weapon(2, 2, WT_PREC));
      pinfo.trollwpn.push_back(new weapon(HC_OBSID, 1, WT_PREC));
      pinfo.trollwpn.push_back(new weapon(HCOLORS, 1, WT_PICK));
      pinfo.trollwpn.push_back(new weapon(4, 40, WT_DECO));
      pinfo.trollwpn.push_back(new weapon(HC_OBSID, 10, WT_DECO));
      pinfo.trollwpn.push_back(new weapon(HC_OBSID, 1, WT_FUNG));
      pinfo.trollwpn.push_back(new weapon(5, 2, WT_LOG));
      pinfo.trollwpn.push_back(new weapon(HCOLORS, 13, WT_BLUNT));
      pinfo.trollwpn.push_back(new weapon(HCOLORS+1, 1, WT_BLUNT));
      pinfo.trollwpn.push_back(new weapon(HC_ANCIENT, 1, WT_SHLD));
      pinfo.trollwpn.push_back(new weapon(HC_OBSID, 1, WT_BLADE));
      pinfo.trollwpn.push_back(new weapon(HC_OBSID, 2, WT_BLADE));
      pinfo.trollwpn.push_back(new weapon(HC_OBSID, 3, WT_BLADE));
      pinfo.trollwpn.push_back(new weapon(HC_ALIEN, 2, WT_QUAKE));
      pinfo.trollwpn.push_back(new weapon(6, 0, WT_PSLAY));
      pinfo.trollwpn.push_back(new weapon(12, 1, WT_SPEAR));
      pinfo.trollwpn.push_back(new weapon(8, 1, WT_AXE));
      pinfo.trollwpn.push_back(new weapon(9, 1, WT_DISK));
      pinfo.trollwpn.push_back(new weapon(10, 1, WT_STONE));
      pinfo.trollwpn.push_back(new weapon(11, 200, WT_PHASE));
      pinfo.trollwpn.push_back(new weapon(10, 1, WT_BOW));
      pinfo.trollwpn.push_back(new weapon(7, 3, WT_SUBD));
      pinfo.trollwpn.push_back(new weapon(8, 3, WT_QUI));
      pinfo.trollwpn.push_back(new weapon(9, 1, WT_GOLD));
      pinfo.trollwpn.push_back(new weapon(10, 2, WT_SPEED));
      pinfo.trollwpn.push_back(new weapon(4, 1, WT_TIME));
      pinfo.trollwpn.push_back(new weapon(5, 10, WT_RAIN));
      pinfo.trollwpn.push_back(new weapon(1, 1, WT_RAND));
      pinfo.trollkey.clear();
      for(int i=0; i<size(pinfo.trollwpn); i++)
        pinfo.trollkey.push_back(i<26 ? 'a'+i : 'A'+(i-26));
      }    

    if(P.flags & dfTutorial)
      addMessage("Welcome to the Tutorial!");
    
    else
      addMessage("Welcome to Hydra Slayer v"VER"!");
    }
  else {
    addMessage("Welcome back to Hydra Slayer!");
    shareS("continue", " the adventure");
    }
  addMessage("Press F1 or ? to get help.");  
  
  fixTheSavefile();
  
  /*for(int i=0; i<10; i++) {
    P.curlevel++;
    generateLevel();
    curHP += 100; maxHP += 100;
    } */
  
  halfdelay(1);
  }
  

char *readArg(char **argv, int& i, int argc) {
  if(argv[i][0] == '-') {
    if(argv[i][2] == 0) {
      if(i == argc-1) return (char*) "";
      else return argv[++i];
      }
    return argv[i]+2;
    }
  if(argv[i][1] == 0)
    return i < argc-1 ? argv[++i] : argv[i]+1;
  return argv[i]+1;
  }

void runTests() {
  initScreen();
  P.arms = 6;
    
  // TEST 0: vampire hydra, 'wait until awakened', 'let yourself hit'
  
  wpn[0] = new weapon(7, 95, WT_AXE);
  wpn[1] = new weapon(8, 19, WT_AXE);
  hydra *h = new hydra(HC_VAMPIRE, 100, 1, 9);
  h->res[7] = 5; h->sheads = 90;
  giveHint(h);

  // TEST 1: ambidexterity
  wpn[0] = new weapon(HC_OBSID, 1, WT_PREC);
  wpn[1] = new weapon(HC_OBSID, 8, WT_BLADE);
  wpn[2] = new weapon(7, 12, WT_BLADE);
  wpn[3] = new weapon(8, 3, WT_DIV);
  wpn[4] = new weapon(9, 10, WT_DIV);
  wpn[5] = new weapon(6, 10, WT_SHLD);
  h = new hydra(6, 15678, 1, 28);
  P.active[IT_PAMBI] = 1;

  h->pos = vec2(5,5); M[h->pos].h = h; // we need this to avoid segfault
  giveHint(h);  

  P.active[IT_PAMBI] = 0; h->heal = 54;
  giveHint(h);  
  
  h->color = HC_GROW; h->heal = 75;
  giveHint(h); 
  
  h->color = HC_GROW; h->heal = 77; h->heads = 619;
  giveHint(h); 

  h->color = HC_WIZARD; h->heal = 15; h->heads = 123;
  giveHint(h);
  
  endwin();
  }

int main(int argc, char **argv) {
  
  P.curlevel = 0; 
  P.gameseed = time(NULL);
  
  pinfo.twin[0] = "Castor";
  pinfo.twin[1] = "Pollux";

  #ifdef ANDROID
  P.geometry = 4;
  pinfo.username = "Hydroid";
  pinfo.charname = "Hydroid";
  
  #else
  
  P.geometry = 4;
  char *user = getenv("USER");
  if(user == NULL) user = getenv("USERNAME");
  if(user == NULL) user = (char*) "Heracles";
  pinfo.username = user;
  pinfo.charname = pinfo.username;
  pinfo.charname[0] = toupper(pinfo.charname[0]);

  for(int i=1; i<argc; i++)
    switch(argv[i][0] == '-' ? argv[i][1] : argv[i][0]) {
    case 's':
      P.gameseed = atoi(readArg(argv, i, argc));
      break;
    
    case 'f':
      savename = readArg(argv, i, argc);
      break;

    case 'b':
      backname = readArg(argv, i, argc);
      break;

    case 't':
      logname = readArg(argv, i, argc);
      break;
    
    case 'g':
      scorename = readArg(argv, i, argc);
      break;
    
    case 'l':
      loglines = atoi(readArg(argv, i, argc));
      break;

#ifdef NOTEYE    
    case 'N':
      // all the following arguments are for NotEye
      i = argc;
      break;
#endif

    case 'd':
      P.flags |= dfDebug;
      P.curlevel = atoi(readArg(argv, i, argc))-1;
      if(P.curlevel < 0) P.curlevel = 0;
      break;
    
    case 'm':
      DIRS = atoi(readArg(argv, i, argc));
      if(DIRS == 4 || DIRS == 6 || DIRS == 8 || DIRS == 16 || DIRS == 3)
        P.geometry = DIRS;
      break;
      
    case 'c':
      pinfo.charname = readArg(argv, i, argc);
      break;
    
    case 'u':
      pinfo.username = readArg(argv, i, argc);
      break;
    
    case 'v':
      initScreen();
      viewHall(false);
      endwin();
      return 0;
    
    case 'T':
      runTests();
      return 0;

    default:
      printf("Usage:\n");
      printf("hydra -s 600 - start a new game with random seed set to 600\n");
      printf("hydra -f hydra2.sav - use hydra2.sav as filename for saving/loading\n");
      printf("hydra -b hydra2.sav - use hydra2.sav as filename for backup cheat\n");
      printf("hydra -t hydralog.txt - use hydralog.txt to save log files\n");
      printf("hydra -g hydrascore.sav - use hydrascore.sav as a scoretable\n");
      printf("hydra -c Heracles - use Heracles as the character name\n");
      printf("hydra -u Zeno - use Zeno as the username\n");
      printf("hydra -v - just view the Hall of Fame without playing the game\n");
      printf("hydra -l 10 - output 10 last message in log files\n");
      printf("hydra -d 20 - start in debug mode, from level 20\n");
      printf("hydra -m 6 - use hex movement\n");
      exit(0);    
    }
  
  #endif
  
  initScreen();
  loadGame();
  if(gameExists) initGame(), mainloop();
  mainmenu();
  clearGame();
  quitgame = false;
  endwin();

  return 0;
  }
