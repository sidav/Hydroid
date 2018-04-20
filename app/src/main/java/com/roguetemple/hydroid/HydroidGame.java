package com.roguetemple.hydroid;

import java.io.File;
import java.util.HashMap;

import android.content.Intent;
import android.net.Uri;
import android.view.KeyEvent;

public class HydroidGame extends Thread {
    Hydroid main;

    public HydroidGame(Hydroid m) {
      main = m;
      kqstart = 0; kqend = 0; keyQueue = new int[16];
      colat = new int [24] [80];
      charat = new char [24] [80];
      radius = 3;
      currentIC = -1;
      }
    
    // methods implemented by C++
    public native int jniMain();
    
    public native void loadMap(byte[] map);
 
    int radius;

    // methods called by C++
    synchronized int getRadius() {
      return radius;
      }
    
    synchronized void setRadius(int r) {
      radius = r;
      }
    
    HashMap<Integer,Tile> gmap;

    void drawAt(int x, int y, int xy,
      int ctype, int hc, int hcolor, int eqtype, 
      int eqcolor, int deadcolor, int flags) {
      if(gmap == null) gmap = new HashMap<Integer,Tile>();
      Tile t = new Tile();
      t.ctype = ctype;
      t.hc = hc;
      t.hcolor = hcolor;
      t.eqtype = eqtype;
      t.eqcolor = eqcolor;
      t.deadcolor = deadcolor;
      t.flags = flags;
      t.xy = xy;
      gmap.put(x+y*100, t);
      }
    
    Tile getTile(int x, int y) {
    	if(gmap == null) return null;
        return gmap.get(x+y*100);
      }

    void refreshScreen(int cic) {
      setIC(cic);
      updateGameScreen();
      Runnable r = new Runnable() {
        public void run() { 
          main.findViewById(R.id.ascv).invalidate();
          /* if(currentIC != lastIC) 
              this.invalidateOptionsMenu(); */
          }
        };
      main.runOnUiThread(r);
      }

    int getKey() {
      int key;
      if(ic() == IC_HELP) {
        openWebsite();
        return 10;
        }
      while((key = getKeyFromQueue()) == -1) {
        try { Thread.sleep(10000); } catch(InterruptedException e) {}
        }
      return key;
      }
    
    // key implementation
    public int[] keyQueue;
    public int kqstart, kqend;
    
    synchronized void addKeyToQueue(int key) {
      if((kqend+1) % 16 != kqstart) {
    	keyQueue[kqend] = key;
    	kqend = (kqend+1) % 16;
      }
    }
    
    synchronized int getKeyFromQueue() {
      if(kqstart == kqend) return -1;
      int k = keyQueue[kqstart];
  	  kqstart = (kqstart+1) % 16;
  	  return k;
      }
    
    // screen implementation
    int colat[][];
    char charat[][];

    synchronized void updateGameScreen() {
      byte[] map = new byte[80*24*2];
      loadMap(map);
      int id = 0;
      for(int y=0; y<24; y++)
        for(int x=0; x<80; x++) {
          charat[y][x] = (char) map[id++];
          colat[y][x] = map[id++];
        }
      }
    
    synchronized char displayCharAt(int x, int y) {
      return charat[y][x];
      }
    
    synchronized int displayColAt(int x, int y) {
      return colat[y][x];
      }

    int currentIC;
    
    synchronized void setIC(int ic) { currentIC = ic; }
    synchronized int ic() { return currentIC; }
    
    final static int IC_GAME     =  0;
    final static int IC_GAMETWIN =  1; // just like GAME, but playing twins
    final static int IC_YESNO    =  2; // yes/no question
    final static int IC_ASKDIR   =  3; // asking for direction

    final static int IC_QUIT     = 16; // quitting menu
    final static int IC_RACE     = 17; // race selection
    final static int IC_INV      = 18; // inventory
    final static int IC_TROLL    = 19; // troll inventory
    final static int IC_EDIT     = 20; // editString
    final static int IC_HALL     = 21; // hall of fame
    final static int IC_VIEWDESC = 22; // view description
    final static int IC_HELP     = 23; // help
    final static int IC_FULLINFO = 24; // fullinfo
    final static int IC_CALL     = 25; // call debug
    final static int IC_MYESNO   = 26; // yes/no question in menu
    
    void pushKey(int pkey) {
      addKeyToQueue(pkey);
      interrupt();
      }
    
    synchronized void panicQueue() {
      kqstart = 0; kqend = 8;
      for(int i=0; i<16; i++) keyQueue[i] = -3;
      }
    
    public void run() {
      new File("/sdcard/hydroid/").mkdir();
      main.debug("jnimain");
      int u = jniMain();
      if(u == 2) {
    	main.debug("crash detected");
        setIC(-20);
        while(true) suspend();
      }
      main.debug(u < 0 ? "game on" : "game ended");
      android.os.Process.killProcess(android.os.Process.myPid());
      }
    
    static {
      System.loadLibrary("hydroid");
      }
    
    static int DBASE = (512*6); 

    static int D_RIGHT = DBASE+0;
    static int D_UP    = DBASE+2;
    static int D_LEFT  = DBASE+4;
    static int D_DOWN  = DBASE+6;
    static int D_PGUP  = DBASE+1;
    static int D_PGDN  = DBASE+7;
    static int D_HOME  = DBASE+3;
    static int D_END   = DBASE+5;
    static int D_CTR   = DBASE+8;
    // static int ESC     = 27; 

    public boolean onMap() {
        return ic() < 16;
        }
        
    public boolean inQuestion() {
        if(ic() == IC_YESNO) return true;
        if(ic() == IC_MYESNO) return true;
        return false;
        }
        
    int centerkey() {
      return inQuestion() ? 'y' : onMap() ? ' ' : 10;
      }
    
    public boolean handleKey(int keyCode, KeyEvent msg) {

      if (keyCode == KeyEvent.KEYCODE_DPAD_UP && main.av.subscreen == 2) {
          main.av.hly = (main.av.hly + main.av.numy - 1) % main.av.numy;
          main.av.invalidate();
      }
          
     
      else if (keyCode == KeyEvent.KEYCODE_DPAD_DOWN && main.av.subscreen == 2) {
    	  main.av.hly = (main.av.hly + 1) % main.av.numy;
          main.av.invalidate();
      }
    
      else if (keyCode == KeyEvent.KEYCODE_DPAD_LEFT && main.av.subscreen == 2) {
    	  main.av.hlx = (main.av.hlx + main.av.numx - 1) % main.av.numx;
          main.av.invalidate();
      }

      else if (keyCode == KeyEvent.KEYCODE_DPAD_RIGHT && main.av.subscreen == 2) {
    	  main.av.hlx = (main.av.hlx + 1) % main.av.numx;
          main.av.invalidate();
      }
          
      else if (keyCode == KeyEvent.KEYCODE_DPAD_CENTER && main.av.subscreen == 2) {
    	  main.av.bmbSend(main.av.hlx, main.av.hly);
    	  }
          
      else if (keyCode == KeyEvent.KEYCODE_DPAD_CENTER && main.av.subscreen == 1) {
    	  main.av.subscreen = 0;
    	  main.av.invalidate();
    	  }
          
      else if (keyCode == KeyEvent.KEYCODE_DPAD_RIGHT && main.av.subscreen == 1)
          pushKey(']');
          
      else if (keyCode == KeyEvent.KEYCODE_DPAD_LEFT && main.av.subscreen == 1)
          pushKey('[');
          
      else if (keyCode == KeyEvent.KEYCODE_DPAD_UP && main.av.subscreen == 1)
          pushKey('[');
          
      else if (keyCode == KeyEvent.KEYCODE_DPAD_DOWN && main.av.subscreen == 1)
          pushKey(']');
          
      else if (keyCode == KeyEvent.KEYCODE_DPAD_UP)
          pushKey(D_UP);
   
      else if (keyCode == KeyEvent.KEYCODE_DPAD_DOWN)
          pushKey(D_DOWN);
  
      else if (keyCode == KeyEvent.KEYCODE_DPAD_LEFT)
          pushKey(D_LEFT);
       
      else if (keyCode == KeyEvent.KEYCODE_DPAD_RIGHT)
          pushKey(D_RIGHT);
        
      else if (keyCode == KeyEvent.KEYCODE_CALL)
          pushKey(inQuestion() ? 'y' : onMap() ? 'g' : 10);
            
      else if (keyCode == KeyEvent.KEYCODE_DPAD_CENTER)
          pushKey(centerkey());
            
      else if (keyCode == KeyEvent.KEYCODE_SEARCH)
          pushKey('i');
              
      else if (keyCode == KeyEvent.KEYCODE_VOLUME_UP)
          pushKey(inQuestion() ? 'y' : onMap() ? '[' : ' ');
                
      else if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN)
          pushKey(inQuestion() ? 'n' : onMap() ? ']' : 'h');
       
      else if (keyCode == KeyEvent.KEYCODE_ENDCALL)
          pushKey(inQuestion() ? 'n' : onMap() ? ']' : ' ');
       
      else if (keyCode == KeyEvent.KEYCODE_DEL)
          pushKey(8);
       
      else if (keyCode == KeyEvent.KEYCODE_BACK && main.av.subscreen > 0) {
        main.av.subscreen = 0;
        main.av.invalidate();
        }
          
      else if (keyCode == KeyEvent.KEYCODE_BACK && !onMap() && ic() != IC_RACE)
        pushKey(
          inQuestion() ? 'n' :
          ic() == IC_QUIT ? 'z' : 
          ic() == IC_EDIT ? 9 : ' '
          );
       
      else {
          
        int chr = msg.getUnicodeChar(msg.getMetaState()); 
        
        if(chr > 0) pushKey(chr);
        else return false;
        }
      
      return true;
      }
    
    void openWebsite() {
      Runnable r = new Runnable() {
        public void run() {
          String url = "http://roguetemple.com/z/hydroid.php";
          Intent i = new Intent(Intent.ACTION_VIEW);
          i.setData(Uri.parse(url));
          main.startActivity(i);
          }
        };
      main.runOnUiThread(r);      
      }

    }
