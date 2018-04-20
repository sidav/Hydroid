package com.roguetemple.hydroid;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.io.Writer;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.Toast;

public class Hydroid extends Activity {

    HydroidGame game;
    AsciiView av;
    
    public Dialog onCreateDialog(int i) {
    	AlertDialog.Builder builder = new AlertDialog.Builder(game.main);
        builder.setMessage("It seems that Hydra Slayer has crashed; this might be caused by a corrupt save file. Please report to zeno@attnam.com.")
               .setCancelable(false)
               .setPositiveButton("Delete save", new DialogInterface.OnClickListener() {
                   public void onClick(DialogInterface dialog, int id) {
                  new File("/sdcard/hydroid/hydra.sav").delete();
                  android.os.Process.killProcess(android.os.Process.myPid());
    		       }
               })
               .setNegativeButton("Just quit", new DialogInterface.OnClickListener() {
                   public void onClick(DialogInterface dialog, int id) {
                        dialog.cancel();
                        android.os.Process.killProcess(android.os.Process.myPid());
                   }
               });
        return builder.create();	
    }

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
      debug("oncreate");
      super.onCreate(savedInstanceState);
      setContentView(R.layout.main);
      
      if(game == null) {
    	  debug("new game");
          game = new HydroidGame(this);
        game.start();
        }

      av = (AsciiView) findViewById(R.id.ascv);
      av.game = game;
      av.invalidate();
      }
    
    public void debug(String s) {
    	int x = 1;
    	if(x == 0)
    	try {
        	Writer out = new OutputStreamWriter(new FileOutputStream("/sdcard/hydroid/debug.txt", true), "UTF-8");
    		out.write(s);
    		out.write(System.getProperty("line.separator"));
    		out.close();
    	} catch(IOException e) {
    	}
    }
    
    
    @Override
    public void onPause() {
      debug("pause");
      game.panicQueue();
      game.interrupt();
      super.onPause();
  
      try {
        Thread.sleep(300);
        }
      catch(Exception e) {
        }            
      }
    
    int lastMenu;

    public int currentMenu() {
      switch(game.ic()) {
    	case HydroidGame.IC_GAME:     return R.menu.menugame;
    	case HydroidGame.IC_GAMETWIN: return R.menu.menutwin;
    	case HydroidGame.IC_INV:      return R.menu.menuinv;
    	case HydroidGame.IC_QUIT:     return R.menu.menuquit;
    	case HydroidGame.IC_TROLL:    return R.menu.menutroll;
    	case HydroidGame.IC_YESNO:    return R.menu.menuyesno;
    	case HydroidGame.IC_MYESNO:   return R.menu.menuyesno;
        case HydroidGame.IC_RACE:     return R.menu.menuselect;
    	default: return R.menu.menugeneral;
    	}  
      }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
      lastMenu = currentMenu();
      MenuInflater inflater = getMenuInflater();
      inflater.inflate(lastMenu, menu);
      return true;
      }
    
    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
      if(lastMenu == currentMenu()) return true;
      lastMenu = currentMenu();
      MenuInflater inflater = getMenuInflater();
      menu.clear();
      inflater.inflate(lastMenu, menu);
      return true;
      }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
      // Handle item selection
      switch (item.getItemId()) {
        case R.id.quit:
          game.pushKey('q');
          return true;
        case R.id.messages:
          game.pushKey('m');
          return true;
        case R.id.fullinfo:
          game.pushKey('f');
          return true;
        case R.id.gethelp:
          game.pushKey('?');
          return true;
        case R.id.desccur:
          game.pushKey('v');
          return true;
        case R.id.dropcur:
          game.pushKey('d');
          return true;
        case R.id.targetcur:
          game.pushKey('t');
          return true;
        case R.id.inventory:
          game.pushKey('i');
          return true;
        case R.id.wpnnext:
          game.pushKey('[');
          return true;
        case R.id.wpnprev:
          game.pushKey(']');
          return true;
        case R.id.pickup:
          game.pushKey('g');
          return true;
        case R.id.mok:
          game.pushKey(10);
          return true;
        case R.id.mcancel:
      	  game.pushKey(' ');
      	  return true;
        case R.id.mmore:
      	  game.pushKey('h');
      	  return true;
        case R.id.msavegame:
          game.pushKey('s');
      	  return true;
        case R.id.mreturn:
      	  game.pushKey('z');
      	  return true;
        case R.id.quitnorec:
      	  game.pushKey('x');
      	  return true;
        case R.id.mnewgame:
      	  game.pushKey('n');
      	  return true;
        case R.id.twinctrl:
          game.pushKey('c');
          return true;
        case R.id.twinswap:
      	  game.pushKey('s');
      	  return true;
        case R.id.myes:
      	  game.pushKey('y');
      	  return true;
        case R.id.mno:
      	  game.pushKey('n');
      	  return true;
        case R.id.mscores:
      	  game.pushKey('s');
      	  return true;
        case R.id.mscoresh:
      	  game.pushKey('h');
      	  return true;
        case R.id.mchangerace:
      	  game.pushKey(HydroidGame.DBASE);
      	  return true;
        case R.id.mplaygame:
      	  game.pushKey(10);
      	  return true;
        case R.id.setdirs:
          game.pushKey('d');
      	  return true;
        case R.id.explore:
          game.pushKey('o');
      	  return true;
        case R.id.switchmode: {
          AsciiView av = (AsciiView) findViewById(R.id.ascv);
          av.wantgraph = !av.wantgraph;
          return true;
          }
        case R.id.website: {
          game.openWebsite();
          return true;
          }
        case R.id.softkeyboard: {
	      av.requestFocus();
          getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
	      InputMethodManager mgr = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
          mgr.showSoftInput(av, InputMethodManager.SHOW_FORCED);
          av.invalidate();
          Toast.makeText(getApplicationContext(), "soft key #2", Toast.LENGTH_SHORT).show();
          return true;
          }
        default:
          return super.onOptionsItemSelected(item);
        }
      }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent msg) {
      if(game != null && !game.handleKey(keyCode, msg))
        return super.onKeyDown(keyCode, msg);
      return true;
      }

    }
// invalidateOptionsMenu