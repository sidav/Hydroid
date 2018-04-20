package com.roguetemple.hydroid;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.graphics.Typeface;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;

public class AsciiView extends View {

    HydroidGame game;
    
    boolean wantgraph;

    int subscreen;

    public AsciiView(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initView();
        }

    public AsciiView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initView();
        havegraph = false;
        wantgraph = true;
        subscreen = 0;
        }

/*    final static int vga[] = {
      0x000000, 0x0000aa, 0x00aa00, 0x00aaaa, 0xaa0000, 0xaa00aa, 0xaa5500, 0xaaaaaa,
      0x555555, 0x5555ff, 0x55ff55, 0x55ffff, 0xff5555, 0xff55ff, 0xffff55, 0xffffff
      }; */

    final static int vga[] = {
      0x000000, 
      0x0F52BA, 0x3C965A, 0x188484, 0x841B2D, 0x664488, 
      0x964B00, 0x8C8C8C, 0x51484F, 0x71A6D2, 0x32CD32, 
      0x7FFFD4, 0xFF0000, 0xE164D1, 0xFFD700, 0xC0C0C0
      };

    int width, height;
    Canvas dc;
    Paint ptext;
    Paint.FontMetrics fm;
    boolean havegraph;
    int scry, scrx;
    
    void drawChar(int x, int y, int dx, int dy) {
      char c = game.displayCharAt(dx,dy);
      if(c == ' ') return;
      int col = game.displayColAt(dx,dy);
      
      // apparently sometimes wrong colors are given, which
      // leads to a crash with vga[col]; this should fix that
      if(col < 0 || col > 15) col = 15;
      
      String s = Character.toString(c);
      float gy = (y+.5f) * height / scry + (-fm.top-fm.bottom)/2;
      float gx = (x+.5f) * width / scrx;
      if(havegraph) {
        ptext.setColor(0xFF000000);
        dc.drawText(s, gx-1, gy, ptext);
        dc.drawText(s, gx+1, gy, ptext);
        dc.drawText(s, gx, gy-1, ptext);
        dc.drawText(s, gx, gy+1, ptext);                 
        dc.drawText(s, gx-1, gy-2, ptext);
        dc.drawText(s, gx+1, gy-2, ptext);
        dc.drawText(s, gx-1, gy+2, ptext);
        dc.drawText(s, gx+1, gy+2, ptext);               
        }
      ptext.setColor(vga[col] | 0xFF000000);
      dc.drawText(s, gx, gy, ptext); 
      }
      
    void normalView() {             
      for(int x=0; x<80; x++) for(int y=0; y<24; y++) {
        if(havegraph && x<50 && y<22) continue;
        drawChar(x,y,x,y);
        }
      }  
    
    void invView() {             
        for(int x=0; x<scrx; x++) for(int y=0; y<scry; y++) {
          drawChar(x,y,50+x,y);
          }
        }  
      
    void fixedMapView() {
      ptext.setFakeBoldText(true);
      if(!havegraph) for(int x=0; x<50; x++) for(int y=0; y<22; y++)
        drawChar(x,y+10,x,y);
      ptext.setFakeBoldText(false);
      for(int x=0; x<50; x++) for(int y=22; y<24; y++)
        drawChar(x,y+10,x,y);
      scrx = 59;
      for(int x=0; x<29; x++) for(int y=0; y<10; y++) {
        drawChar(x, y, 51+x, y);
        drawChar(30+x, y, 51+x, y+10);
        }
      scrx = 50;
      }

    void fixedOtherView() {
      int lines = 0;
      for(int y=0; y<24; y++) for(int x=0; x<80; x+=40) {
        for(int dx=0; dx<40; dx++)
          drawChar(dx, lines, x+dx, y);
        lines++;
        }
      }

    void subscrView() {
      for(int y=0; y<10; y++) {
        int ay = y/2, u = (y&1), ax = (1-u) * 5;
        drawChar(ax+u, ay, 51, y);
        drawChar(ax+1-u, ay, 52, y);
        drawChar(ax+2, ay, 77, y);
        drawChar(ax+3, ay, 78, y);
        drawChar(ax+4, ay, 79, y);
        }
      }

    void setScrDim() {
      if(subscreen == 1) {
        scrx = 10; scry = 5;
        }
      else if(game.ic() == HydroidGame.IC_INV) {
    	scrx = 30; scry = 24;
      }
      else if(height < width) {
        scrx = 80; scry = 24; 
        }
      else if(game.onMap()) {
        scrx = 50; scry = 34;
        }
      else {
        scrx = 40; scry = 48;
        }
      }

    void drawAscii() {
      ptext = new Paint();
      ptext.setAntiAlias(true);        
      ptext.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC));
      ptext.setTextSize(8);
      ptext.setTextAlign(Paint.Align.CENTER);            
      ptext.setTypeface(Typeface.MONOSPACE);
      // ptext.setTextScaleX(1.25f);
      
      setScrDim();

      float charWidth  = width / (scrx+.0f);
      float curw = ptext.measureText("M");
      
      ptext.setTextSize(8 * charWidth/curw);

      // float charHeight  = (float) (height / (scry+.0));
      // float curh = fm.top + fm.bottom;      
      
      fm = ptext.getFontMetrics();
            
      if(scrx == 40) fixedOtherView();
      if(scrx == 50) fixedMapView();
      if(scrx == 80) normalView();
      if(scrx == 10) subscrView();
      if(scrx == 30) invView();
      
      // ptext.setTextSize(20); ptext.setColor(0x7FE0E0E0);
      // String s = "TOP "+Float.toString(fm.top)+" BOTTOM "+Float.toString(fm.bottom)+" SIZE "+Float.toString(height/scry); 
      // dc.drawText(s, width/2, height/2, ptext);
      }

    void drawGraphicalMap() {
      int wi = floor.getHeight();
      
      int radiusX = (width/2 - (wi/2)) / wi + 1;
      int radiusY = (height/2 - (wi/2)) / wi + 1;
      boolean hexgame = game.getTile(0,1) != null && game.getTile(0,1).ctype == 4;
      if(hexgame) radiusX = 2 * radiusX;
      int radius = Math.max(radiusX, radiusY);      
      
      game.setRadius(radius);

      Paint p = new Paint();
      p.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC));
      
      Paint pon = new Paint();
      pon.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC_ATOP));
      
      Paint pshade = new Paint();
      pshade.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.MULTIPLY));
      pshade.setColor(0xFF404040);
      
      Paint precolor = new Paint();
      precolor.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.MULTIPLY));
      
      Bitmap recolorer = 
          Bitmap.createBitmap(wi, wi, Bitmap.Config.ARGB_8888);
      Canvas recc = new Canvas(recolorer);

      havegraph = radius > 0;
      
      if(hexgame)
      for(int y=-radiusY; y<=radiusY; y++)
      for(int x=-radiusX; x<=radiusX; x++) {
        Tile t = game.getTile(x,y);
        if(t == null) continue;
        boolean explored = (t.flags & 2) > 0;
        int sx = (width-wi)/2 + (wi/2) * x;
        int sy = (height-wi)/2 + wi * y;
        if(t.ctype == 4) continue;
        if(explored) dc.drawBitmap(t.ctype == 1 ? wallhex : floorhex, sx-wi/4, sy-wi/4, pon);
        }
      
      for(int y=-radiusY; y<=radiusY; y++)
      for(int x=-radiusX; x<=radiusX; x++) {

        Tile t = game.getTile(x,y);
        if(t == null) continue;
        boolean seen = (t.flags & 1) > 0;
        boolean explored = (t.flags & 2) > 0;
        boolean ontarget = (t.flags & 4) > 0;
        int sx = (width-wi)/2 + wi * x;
        int sy = (height-wi)/2 + wi * y;
        if(hexgame) sx = (width-wi)/2 + (wi/2) * x; 
        if(t.ctype == 4) continue;

        if(explored) {
          // base terrain
          if(!hexgame) {
        	Rect dst = new Rect(sx, sy, sx+wi, sy+wi);
        	int id = t.xy;
        	Rect src = new Rect(wi*id, 0, wi*(id+1), wi);
            dc.drawBitmap(t.ctype == 1 ? wall : floor, src, dst, p);
            }
            
          // additional terrain
          if(t.ctype == 2)
              dc.drawBitmap(stairup, sx, sy, pon);
          if(t.ctype == 3)
              dc.drawBitmap(stairdown, sx, sy, pon);
            
          // corpses
          if(t.deadcolor > 0) {
            dc.drawBitmap(blood, sx, sy, pon);
            recc.drawBitmap(hydra[0], 0, 0, p); 
            precolor.setColor(0xFF000000 | vga[t.deadcolor]);
            recc.drawRect(0,0,wi,wi,precolor);
            dc.drawBitmap(recolorer, sx, sy, pon);
            }
          
          // equipment
          if(t.eqtype != 0) {
            Bitmap b = blade;
            if(t.eqtype == -1) b = rune;
            if(t.eqtype == -2) b = scroll;
            if(t.eqtype == -3) b = potion;               
  
            if(t.eqtype == 'S') b = club;
            if(t.eqtype == '/') b = divisor;
            if(t.eqtype == 'M') b = star;
            if(t.eqtype == 'P') b = shield;
            if(t.eqtype == 'R') b = erado;
            if(t.eqtype == 'A') b = axe;
            if(t.eqtype == 'F') b = club;
            if(t.eqtype == 'W') b = pickaxe;
            if(t.eqtype == 'D') b = dblade;
            if(t.eqtype == 'V') b = dblade;
            if(t.eqtype == '\\') b = decomp;
            if(t.eqtype == 'L') b = logaems;
            if(t.eqtype == 'Q') b = trollclub;
            if(t.eqtype == 'B') b = bow;
            if(t.eqtype == '(') b = qblade;
            if(t.eqtype == 'p') b = decomp;
            if(t.eqtype == 'A') b = axe;
            if(t.eqtype == 'K') b = gem;
            if(t.eqtype == 'C') b = bldisk;
            if(t.eqtype == 'I') b = spear;
            if(t.eqtype == 'T') b = wand;
            if(t.eqtype == 'G') b = decomp;
            if(t.eqtype == 'O') b = decomp;
            if(t.eqtype == '!') b = wand;
                                          
            recc.drawBitmap(b, 0, 0, p); 
            precolor.setColor(0xFF000000 | vga[t.eqcolor]);
            recc.drawRect(0,0,wi,wi,precolor);
            dc.drawBitmap(recolorer, sx, sy, pon);
            }
            
          // mushrooms and creatures
          Bitmap slayerb =
            ((t.flags & 8) > 0) ? slayerDead :
            ((t.flags & 32) > 0) ? slayerCentaur :
            ((t.flags & 64) > 0) ? slayerNaga :
            slayer;
          if(x == 0 && y == 0)
            dc.drawBitmap(slayerb, sx, sy, pon);            
            
          if(t.hc > 0 && t.hcolor > 0 && seen) {
            int hc = t.hc;
            if(hc >= 1000) hc = 35;
            else if(hc >= 100) hc = 34;
            else if(hc >= 33) hc = 33;
            recc.drawBitmap(hydra[hc], 0, 0, p);
            precolor.setColor(0xFF000000 | vga[t.hcolor]);
            recc.drawRect(0,0,wi,wi,precolor);
            dc.drawBitmap(recolorer, sx, sy, pon);
            }
          
          if(t.hc > 0 && t.hcolor == -3)
            dc.drawBitmap(twin, sx, sy, pon);
          if(t.hc > 0 && t.hcolor == -2 && seen)
            dc.drawBitmap(giant[Math.min(t.hc,3)], sx, sy, pon);
          if(t.hc > 0 && t.hcolor == -1)
            dc.drawBitmap(mushroom[Math.min(t.hc,9)], sx, sy, pon);
            
          // darkening
          if(!seen) {
            dc.drawRect(sx,sy,sx+wi,sy+wi,pshade);
            }
          }
      
        if(ontarget) {
          p.setAlpha(255);
          dc.drawBitmap(target, sx, sy, pon);
          }
        }
     
      }

    static final String bmbNames[] = {
      "Explore", "Look", "Gfx/map", 
      "Get/use", "Items", "Drop", 
      "Help", "Main menu", "Manual target",
      "Twin", "Swap", "Target"
      };
    
    Bitmap bmbIcon(int id) {
      switch(id) {
        case 0:  return wand;
        case 1:  return hydra[7];
        case 2:  return rune;
        case 3:  return stairdown;
        case 4:  return potion;
        case 5:  return hydra[0];
        case 6:  return scroll;
        case 7:  return star;
        case 8:  return target;
        case 9:  return slayer;
        case 10: return twin;
        case 11: return target;
        }
      return blood;
      }

    static final String bmbAction = "ofGgid?qTcst";

    int numx , numy, hlx, hly;

    public void drawBigMenu() {
      Paint pon = new Paint();
      pon.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC_ATOP));
      pon.setColor(0xC0C0C0C0);
      pon.setAntiAlias(true);        
      dc.drawRect(0,0,width,height,pon);
      
      if(height > width) { numx = 2; numy = 6; }
      else { numx = 3; numy = 4; }

      pon.setColor(0xC0FFFFFF);
      dc.drawRect(hlx*width/numx, hly*height/numy, (hlx+1)*width/numx, (hly+1)*height/numy, pon);
      
      pon.setColor(0xC0000000);
      for(int i=0; i<=numx; i++)
        dc.drawLine(i*width/numx,0,i*width/numx,height,pon);
      for(int i=0; i<=numy; i++)
        dc.drawLine(0,i*height/numy,width,i*height/numy,pon);

      pon.setTextSize(10);
      pon.setTextAlign(Paint.Align.CENTER);
      
      int id = 0;
      
      for(int y=0; y<numy; y++)
      for(int x=0; x<numx; x++) {
        dc.drawText(
          bmbNames[id],
          (x + .5f) * width / numx,
          (y + .9f) * height / numy,
          pon
          );
        dc.drawBitmap(
          bmbIcon(id), 
          (x + .5f) * width / numx - wall.getHeight()/2, 
          (y + .1f) * height / numy, 
          pon
          );
        id++;
        }
      }

    public void removeSaveDialog() {
      game.main.showDialog(1);
      }
        
    @Override
    public void onDraw(final Canvas canvas) {
      super.onDraw(canvas);
      dc = canvas;
      
      if(game == null) return;
      
      if(game.ic() < -1) {
    	  game.main.debug("ondraw ic="+Integer.toString(game.ic()));
    	  if(game.ic() == -20) 
    		  removeSaveDialog();
    	  else android.os.Process.killProcess(android.os.Process.myPid());
    	// removeSaveDialog();
        // game.main.finish();
    	  
    	return;
        }
        
      width = getWidth();
      height = getHeight();
      
      havegraph = false;
      if(wantgraph && game.onMap()) drawGraphicalMap();
      if(wantgraph && !game.onMap()) {
      	  Rect dst = new Rect(0, 0, width, height);
    	  Rect src = new Rect(0, 0, titlescreen.getWidth(), titlescreen.getHeight());
          dc.drawBitmap(titlescreen, src, dst, null);
    	
      }
      drawAscii();
      if(subscreen == 2) drawBigMenu();
      }

    public void bmbSend(int x, int y) {
    	char ch = bmbAction.charAt(x + numx * y);
        
        if(ch == 'W')
          game.openWebsite();
        else if(ch == 'G') {
          wantgraph = !wantgraph;
          invalidate();
          }
        else game.pushKey(ch);
        subscreen = 0;    	
    }
    
    @Override
    public boolean onTouchEvent (MotionEvent event) {
      if(event.getAction() == MotionEvent.ACTION_DOWN) {
    	float x = event.getX();
    	float y = event.getY();
    	
    	float width = getWidth();
    	float height = getHeight();
    	
    	setScrDim();

        int dx = (int) (x * scrx / width);
        int dy = (int) (y * scry / height);
        
        if(game.inQuestion()) {
          if(y <   height / 4) game.pushKey('y');
          if(y > 3*height / 4) game.pushKey('n');
          return true;
          }
        
        if(subscreen == 1) {
          game.pushKey("2143658709".charAt(dy*2 + (dx/5)));
          subscreen = 0;
          return true;
          }
          
        if(subscreen == 2) {
          bmbSend((int) (x * numx / width), (int) (y * numy / height));
          return true;
          }
          
        if((scrx == 50 && dy > 30) || (scrx == 80 && dy > 20)) {
              game.pushKey('m');
              return true;
          }
       
        if(scrx == 50 && dy < 5) {
              subscreen = dx < 25 ? 1 : 2;
              hlx = 0; hly = 0;
              invalidate();
              return true;
          }
       
        if(scrx == 80 && dx > 68) {
              subscreen = dy < 10 ? 1 : 2;
              hlx = 0; hly = 0;
              invalidate();
              return true;
          }
                
        if(game.onMap() || game.ic() == HydroidGame.IC_TROLL) {
          int id=0;
          if(x > 2*width /3) id++;  
          if(x > 1*width /3) id++;  
          if(y > 2*height/3) id+=3;  
          if(y > 1*height/3) id+=3;
          
          int keys[] = { 
            3, 2, 1,
            4, -1, 0,
            5, 6, 7
            };
          
          int key = keys[id];
          
          if(key >= 0) game.pushKey(HydroidGame.DBASE + key);
          if(key < 0) game.pushKey(game.centerkey());
          return true;
          }

        if(scrx == 40) { if((dy&1) == 1) dx += 40; dy /= 2; }

        if(game.ic() == HydroidGame.IC_INV) {
          //game.pushKey("hhsdcgofntbrpwekjmaliiiiiiii".charAt(dy));
          game.pushKey(game.displayCharAt(51,dy));
          return true;
          }
        
        if(game.ic() == HydroidGame.IC_QUIT) {
          if(game.displayCharAt(1,4) == 'T')
        	  game.pushKey("tttttnssdaaoobzzttttttzzzzzz".charAt(dy));
          else
          game.pushKey("zzzzzsqxdaaoobzzttttttzzzzzz".charAt(dy));
          return true;
          }

        if(game.ic() == HydroidGame.IC_FULLINFO) {
          game.pushKey("  abcdefghijklmnopqrstuvwxyz".charAt(dy));
          return true;
          }

        if(game.ic() == HydroidGame.IC_RACE) {
          if(dy < 13) { 
            if(dx % 40 < 8) game.pushKey('h');
            else if(dx % 40 < 16) game.pushKey('e');
            else if(dx % 40 < 24) game.pushKey('t');
            else if(dx % 40 < 32) game.pushKey('w');
            else game.pushKey('c');
            }
          else if(dy < 20) game.pushKey(27);
          else if(dy >= 22) game.pushKey(10);
          else if(dy >= 20) game.pushKey('d');
          return true;
          }
    	
        if(game.ic() == HydroidGame.IC_VIEWDESC) {
          game.pushKey(' ');
          return true;
          }

        if(game.ic() == HydroidGame.IC_HELP) {
          if(x < width/3) game.pushKey(HydroidGame.DBASE + 4);
          else if(x > 2*width/3) game.pushKey(HydroidGame.DBASE + 0);
          else game.pushKey(' ');
          return true;
          }

        }
    	    
      return false;
      }
    
    public Bitmap loadTile(Drawable d) {
      return ((BitmapDrawable)d).getBitmap();
      /* if(d == null) return null;
      Bitmap bitmap = 
        Bitmap.createBitmap(32, 32, Bitmap.Config.ARGB_8888);
      Canvas canvas = new Canvas(bitmap);
      d.setBounds(0, 0, 32, 32);
      
      d.draw(canvas);
      
      return bitmap; */
      }
    
    Bitmap 
      floor, wall, floorhex, wallhex, stairup, stairdown, target, blood,  
      
      slayer, hydra[], giant[], mushroom[], twin, 
      
      rune, scroll, potion, 
      
      axe, blade, bldisk, bow, club, dblade, decomp, divisor,
      erado, gem, logaems, pickaxe, qblade, shield, spear, star, trollclub, wand,
      titlescreen,
      slayerDead, slayerNaga, slayerCentaur;
    
    public void initView() {
      setFocusable(true);
      setFocusableInTouchMode(true);

      Resources r = this.getContext().getResources();
      floor = loadTile(r.getDrawable(R.drawable.floor));
      wall = loadTile(r.getDrawable(R.drawable.wall));
      floorhex = loadTile(r.getDrawable(R.drawable.hexfloor));
      wallhex = loadTile(r.getDrawable(R.drawable.hexwall));
      stairup = loadTile(r.getDrawable(R.drawable.up)); 
      stairdown = loadTile(r.getDrawable(R.drawable.down)); 
      target = loadTile(r.getDrawable(R.drawable.target));
      blood = loadTile(r.getDrawable(R.drawable.blood));
      titlescreen = loadTile(r.getDrawable(R.drawable.titlescreen)); 

      rune = loadTile(r.getDrawable(R.drawable.rune)); 
      scroll = loadTile(r.getDrawable(R.drawable.scroll)); 
      potion = loadTile(r.getDrawable(R.drawable.potion)); 
      
      axe = loadTile(r.getDrawable(R.drawable.axe)); 
      blade = loadTile(r.getDrawable(R.drawable.blade)); 
      bldisk = loadTile(r.getDrawable(R.drawable.bldisk)); 
      bow = loadTile(r.getDrawable(R.drawable.bow)); 
      club = loadTile(r.getDrawable(R.drawable.club)); 
      dblade = loadTile(r.getDrawable(R.drawable.dblade)); 
      decomp = loadTile(r.getDrawable(R.drawable.decomp)); 
      divisor = loadTile(r.getDrawable(R.drawable.divisor));
      erado = loadTile(r.getDrawable(R.drawable.erado));
      gem = loadTile(r.getDrawable(R.drawable.gem));
      logaems = loadTile(r.getDrawable(R.drawable.logaems));
      pickaxe = loadTile(r.getDrawable(R.drawable.pickaxe));
      qblade = loadTile(r.getDrawable(R.drawable.qblade));
      shield = loadTile(r.getDrawable(R.drawable.shield));
      spear = loadTile(r.getDrawable(R.drawable.spear));
      star = loadTile(r.getDrawable(R.drawable.star));
      trollclub = loadTile(r.getDrawable(R.drawable.trollclub));
      wand = loadTile(r.getDrawable(R.drawable.wand));      
                  
      slayer = loadTile(r.getDrawable(R.drawable.slayer));
      slayerDead = loadTile(r.getDrawable(R.drawable.slayerdead));
      slayerNaga = loadTile(r.getDrawable(R.drawable.slayernaga));
      slayerCentaur = loadTile(r.getDrawable(R.drawable.slayercentaur));
      twin = loadTile(r.getDrawable(R.drawable.twin));
      
      
      hydra = new Bitmap[36];
      hydra[0] = loadTile(r.getDrawable(R.drawable.corpse));
      hydra[1] = loadTile(r.getDrawable(R.drawable.hydra1));
      hydra[2] = loadTile(r.getDrawable(R.drawable.hydra2));
      hydra[3] = loadTile(r.getDrawable(R.drawable.hydra3));
      hydra[4] = loadTile(r.getDrawable(R.drawable.hydra4));
      hydra[5] = loadTile(r.getDrawable(R.drawable.hydra5));
      hydra[6] = loadTile(r.getDrawable(R.drawable.hydra6));
      hydra[7] = loadTile(r.getDrawable(R.drawable.hydra7));
      hydra[8] = loadTile(r.getDrawable(R.drawable.hydra8));
      hydra[9] = loadTile(r.getDrawable(R.drawable.hydra9));
      hydra[10] = loadTile(r.getDrawable(R.drawable.hydra10));
      hydra[11] = loadTile(r.getDrawable(R.drawable.hydra11));
      hydra[12] = loadTile(r.getDrawable(R.drawable.hydra12));
      hydra[13] = loadTile(r.getDrawable(R.drawable.hydra13));
      hydra[14] = loadTile(r.getDrawable(R.drawable.hydra14));
      hydra[15] = loadTile(r.getDrawable(R.drawable.hydra15));
      hydra[16] = loadTile(r.getDrawable(R.drawable.hydra16));
      hydra[17] = loadTile(r.getDrawable(R.drawable.hydra17));
      hydra[18] = loadTile(r.getDrawable(R.drawable.hydra18));
      hydra[19] = loadTile(r.getDrawable(R.drawable.hydra19));
      hydra[20] = loadTile(r.getDrawable(R.drawable.hydra20));
      hydra[21] = loadTile(r.getDrawable(R.drawable.hydra21));
      hydra[22] = loadTile(r.getDrawable(R.drawable.hydra22));
      hydra[23] = loadTile(r.getDrawable(R.drawable.hydra23));
      hydra[24] = loadTile(r.getDrawable(R.drawable.hydra24));
      hydra[25] = loadTile(r.getDrawable(R.drawable.hydra25));
      hydra[26] = loadTile(r.getDrawable(R.drawable.hydra26));
      hydra[27] = loadTile(r.getDrawable(R.drawable.hydra27));
      hydra[28] = loadTile(r.getDrawable(R.drawable.hydra28));
      hydra[29] = loadTile(r.getDrawable(R.drawable.hydra29));
      hydra[30] = loadTile(r.getDrawable(R.drawable.hydra30));
      hydra[31] = loadTile(r.getDrawable(R.drawable.hydra31));
      hydra[32] = loadTile(r.getDrawable(R.drawable.hydra32));
      hydra[33] = loadTile(r.getDrawable(R.drawable.hydra33));
      hydra[34] = loadTile(r.getDrawable(R.drawable.hydra34));
      hydra[35] = loadTile(r.getDrawable(R.drawable.hydra35));
      
      mushroom = new Bitmap[10];
      mushroom[1] = loadTile(r.getDrawable(R.drawable.mushroom1));
      mushroom[2] = loadTile(r.getDrawable(R.drawable.mushroom2));
      mushroom[3] = loadTile(r.getDrawable(R.drawable.mushroom3));
      mushroom[4] = loadTile(r.getDrawable(R.drawable.mushroom4));
      mushroom[5] = loadTile(r.getDrawable(R.drawable.mushroom5));
      mushroom[6] = loadTile(r.getDrawable(R.drawable.mushroom6));
      mushroom[7] = loadTile(r.getDrawable(R.drawable.mushroom7));
      mushroom[8] = loadTile(r.getDrawable(R.drawable.mushroom8));
      mushroom[9] = loadTile(r.getDrawable(R.drawable.mushroom9));

      giant = new Bitmap[4];
      giant[1] = loadTile(r.getDrawable(R.drawable.giant1));
      giant[2] = loadTile(r.getDrawable(R.drawable.giant2));
      giant[3] = loadTile(r.getDrawable(R.drawable.giant3));      
      }

    }
