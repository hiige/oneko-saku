/*
 *      oneko  -  X11 猫
 */

#include "oneko.h"
#include "patchlevel.h"

/*
 *      グローバル変数
 */

char    *ClassName = "Oneko";           /* コマンド名称 */
char    *ProgramName;                   /* コマンド名称 */

Display *theDisplay;                    /* ディスプレイ構造体 */
int     theScreen;                      /* スクリーン番号 */
unsigned int    theDepth;               /* デプス */
Window  theRoot;                        /* ルートウィンドウのＩＤ */
Window  theWindow;                      /* 猫ウィンドウのＩＤ */
char    *WindowName = NULL;             /* 猫ウィンドウの名前 */
Window  theTarget = None;               /* 目標ウィンドウのＩＤ */
char    *TargetName = NULL;             /* 目標ウィンドウの名前 */
Cursor  theCursor;                      /* ねずみカーソル */

unsigned int    WindowWidth;            /* ルートウィンドウの幅 */
unsigned int    WindowHeight;           /* ルートウィンドウの高さ */

XColor  theForegroundColor;             /* 色 (フォアグラウンド) */
XColor  theBackgroundColor;             /* 色 (バックグラウンド) */

unsigned int Scale = 1; /* scaling factor for bitmaps */
unsigned int BitmapWidth = BITMAP_WIDTH; /* scaled bitmap width */
unsigned int BitmapHeight = BITMAP_HEIGHT; /* scaled bitmap height */

int Synchronous = False;
/* Types of animals */
#define BITMAPTYPES 6
typedef struct _AnimalDefaults {
  char *name;
  int speed, idle, bitmap_width, bitmap_height;
  long time;
  int off_x, off_y;
  char *cursor,*mask;
  int cursor_width,cursor_height,cursor_x_hot,cursor_y_hot;
} AnimalDefaultsData;

AnimalDefaultsData AnimalDefaultsDataTable[] =
{
  { "neko", 13, 6, 32, 32, 125000L, 0, 0, mouse_cursor_bits,mouse_cursor_mask_bits,
      mouse_cursor_width,mouse_cursor_height, mouse_cursor_x_hot,mouse_cursor_y_hot },
  { "tora", 16, 6, 32, 32, 125000L, 0, 0, mouse_cursor_bits,mouse_cursor_mask_bits,
      mouse_cursor_width,mouse_cursor_height, mouse_cursor_x_hot,mouse_cursor_y_hot },
  { "dog" , 10, 6, 32, 32, 125000L, 0, 0, bone_cursor_bits,bone_cursor_mask_bits,
      bone_cursor_width,bone_cursor_height, bone_cursor_x_hot,bone_cursor_y_hot },
  { "bsd_daemon" , 16, 6, 32, 32, 300000L, 22, 20, bsd_cursor_bits,bsd_cursor_mask_bits,
      bsd_cursor_width,bsd_cursor_height, bsd_cursor_x_hot,bsd_cursor_y_hot },
  { "sakura" , 13, 6, 32, 32, 125000L, 0, 0, card_cursor_bits,card_cursor_mask_bits,
      card_cursor_width,card_cursor_height, card_cursor_x_hot,card_cursor_y_hot },
  { "tomoyo" , 10, 6, 32, 32, 125000L, 32, 32, petal_cursor_bits,petal_cursor_mask_bits,
      petal_cursor_width,petal_cursor_height, petal_cursor_x_hot,petal_cursor_y_hot },
};

/*
 *      いろいろな初期設定 (オプション、リソースで変えられるよ)
 */

                                        /* Resource:    */
char    *Foreground = NULL;             /*   foreground */
char    *Background = NULL;             /*   background */
long    IntervalTime = 0L;              /*   time       */
long    NekoSleepTime = 0L;             /*NekoSleep Time*/
double  NekoSpeed = (double)0;          /*   speed      */
int     IdleSpace = 0;                  /*   idle       */
int     NekoMoyou = NOTDEFINED;         /*   tora       */
int     NoShape = NOTDEFINED;           /*   noshape    */
int     NoCursor = False;
int     ReverseVideo = NOTDEFINED;      /*   reverse    */
int     ToWindow = NOTDEFINED;          /*   towindow   */
int     ToFocus = NOTDEFINED;           /*   tofocus    */
int     XOffset=0,YOffset=0;            /* X and Y offsets for cat from mouse
                                           pointer. */
/*
 *      いろいろな状態変数
 */

Bool    DontMapped = True;

int     NekoTickCount;          /* 猫動作カウンタ */
int     NekoStateCount;         /* 猫同一状態カウンタ */
int     NekoState;              /* 猫の状態 */

int     MouseX;                 /* マウスＸ座標 */
int     MouseY;                 /* マウスＹ座標 */

int     PrevMouseX = 0;         /* 直前のマウスＸ座標 */
int     PrevMouseY = 0;         /* 直前のマウスＹ座標 */
Window  PrevTarget = None;      /* 直前の目標ウィンドウのＩＤ */

int     NekoX;                  /* 猫Ｘ座標 */
int     NekoY;                  /* 猫Ｙ座標 */

int     NekoMoveDx;             /* 猫移動距離Ｘ */
int     NekoMoveDy;             /* 猫移動距離Ｙ */

int     NekoLastX;              /* 猫最終描画Ｘ座標 */
int     NekoLastY;              /* 猫最終描画Ｙ座標 */
GC      NekoLastGC;             /* 猫最終描画 GC */
/* Variables used to set how quickly the program will chose to raise itself. */
/* Look at Interval(), Handle Visiblility Notify Event */
#define DEFAULT_RAISE_WAIT 16  /* About 2 seconds with default interval */
int     RaiseWindowDelay=0;
/*
 *      その他
 */

double  SinPiPer8Times3;        /* sin(３π／８) */
double  SinPiPer8;              /* sin(π／８) */

Pixmap  Mati2Xbm, Jare2Xbm, Kaki1Xbm, Kaki2Xbm, Mati3Xbm, Sleep1Xbm, Sleep2Xbm;
Pixmap  Mati2Msk, Jare2Msk, Kaki1Msk, Kaki2Msk, Mati3Msk, Sleep1Msk, Sleep2Msk;

Pixmap  AwakeXbm, AwakeMsk;

Pixmap  Up1Xbm, Up2Xbm, Down1Xbm, Down2Xbm, Left1Xbm, Left2Xbm;
Pixmap  Up1Msk, Up2Msk, Down1Msk, Down2Msk, Left1Msk, Left2Msk;
Pixmap  Right1Xbm, Right2Xbm, UpLeft1Xbm, UpLeft2Xbm, UpRight1Xbm;
Pixmap  Right1Msk, Right2Msk, UpLeft1Msk, UpLeft2Msk, UpRight1Msk;
Pixmap  UpRight2Xbm, DownLeft1Xbm, DownLeft2Xbm, DownRight1Xbm, DownRight2Xbm;
Pixmap  UpRight2Msk, DownLeft1Msk, DownLeft2Msk, DownRight1Msk, DownRight2Msk;

Pixmap  UpTogi1Xbm, UpTogi2Xbm, DownTogi1Xbm, DownTogi2Xbm, LeftTogi1Xbm;
Pixmap  UpTogi1Msk, UpTogi2Msk, DownTogi1Msk, DownTogi2Msk, LeftTogi1Msk;
Pixmap  LeftTogi2Xbm, RightTogi1Xbm, RightTogi2Xbm;
Pixmap  LeftTogi2Msk, RightTogi1Msk, RightTogi2Msk;

GC      Mati2GC;

GC      Jare2GC, Kaki1GC, Kaki2GC, Mati3GC, Sleep1GC, Sleep2GC;

GC      AwakeGC;

GC      Up1GC, Up2GC, Down1GC, Down2GC, Left1GC, Left2GC, Right1GC, Right2GC;
GC      UpLeft1GC, UpLeft2GC, UpRight1GC, UpRight2GC, DownLeft1GC, DownLeft2GC;
GC      DownRight1GC, DownRight2GC;

GC      UpTogi1GC, UpTogi2GC, DownTogi1GC, DownTogi2GC, LeftTogi1GC;
GC      LeftTogi2GC, RightTogi1GC, RightTogi2GC;


typedef struct {
    GC          *GCCreatePtr;
    Pixmap      *BitmapCreatePtr;
    char        *PixelPattern[BITMAPTYPES];
    Pixmap      *BitmapMasksPtr;
    char        *MaskPattern[BITMAPTYPES];
} BitmapGCData;

BitmapGCData    BitmapGCDataTable[] =
{
    { &Mati2GC, &Mati2Xbm,  mati2_bits, mati2_tora_bits, mati2_dog_bits, mati2_bsd_bits, mati2_sakura_bits, mati2_tomoyo_bits,
      &Mati2Msk, mati2_mask_bits, mati2_mask_bits, mati2_dog_mask_bits, mati2_bsd_mask_bits, mati2_sakura_mask_bits, mati2_tomoyo_mask_bits },
    { &Jare2GC, &Jare2Xbm,  jare2_bits, jare2_tora_bits, jare2_dog_bits, jare2_bsd_bits, jare2_sakura_bits, jare2_tomoyo_bits,
      &Jare2Msk, jare2_mask_bits, jare2_mask_bits, jare2_dog_mask_bits, jare2_bsd_mask_bits, jare2_sakura_mask_bits, jare2_tomoyo_mask_bits },
    { &Kaki1GC, &Kaki1Xbm,  kaki1_bits, kaki1_tora_bits, kaki1_dog_bits, kaki1_bsd_bits, kaki1_sakura_bits, kaki1_tomoyo_bits,
      &Kaki1Msk, kaki1_mask_bits, kaki1_mask_bits, kaki1_dog_mask_bits, kaki1_bsd_mask_bits, kaki1_sakura_mask_bits, kaki1_tomoyo_mask_bits },
    { &Kaki2GC, &Kaki2Xbm,  kaki2_bits, kaki2_tora_bits, kaki2_dog_bits, kaki2_bsd_bits, kaki2_sakura_bits, kaki2_tomoyo_bits,
      &Kaki2Msk, kaki2_mask_bits, kaki2_mask_bits, kaki2_dog_mask_bits, kaki2_bsd_mask_bits, kaki2_sakura_mask_bits, kaki2_tomoyo_mask_bits },
    { &Mati3GC, &Mati3Xbm,  mati3_bits, mati3_tora_bits, mati3_dog_bits, mati3_bsd_bits, mati3_sakura_bits, mati3_tomoyo_bits,
      &Mati3Msk, mati3_mask_bits, mati3_mask_bits, mati3_dog_mask_bits, mati3_bsd_mask_bits, mati3_sakura_mask_bits, mati3_tomoyo_mask_bits },
    { &Sleep1GC, &Sleep1Xbm,  sleep1_bits, sleep1_tora_bits, sleep1_dog_bits, sleep1_bsd_bits, sleep1_sakura_bits, sleep1_tomoyo_bits,
      &Sleep1Msk, sleep1_mask_bits, sleep1_mask_bits, sleep1_dog_mask_bits, sleep1_bsd_mask_bits, sleep1_sakura_mask_bits, sleep1_tomoyo_mask_bits },
    { &Sleep2GC, &Sleep2Xbm,  sleep2_bits, sleep2_tora_bits, sleep2_dog_bits, sleep2_bsd_bits, sleep2_sakura_bits, sleep2_tomoyo_bits,
      &Sleep2Msk, sleep2_mask_bits, sleep2_mask_bits, sleep2_dog_mask_bits, sleep2_bsd_mask_bits, sleep2_sakura_mask_bits, sleep2_tomoyo_mask_bits },
    { &AwakeGC, &AwakeXbm,  awake_bits, awake_tora_bits, awake_dog_bits, awake_bsd_bits, awake_sakura_bits, awake_tomoyo_bits,
      &AwakeMsk, awake_mask_bits, awake_mask_bits, awake_dog_mask_bits, awake_bsd_mask_bits, awake_sakura_mask_bits, awake_tomoyo_mask_bits },
    { &Up1GC, &Up1Xbm,  up1_bits, up1_tora_bits, up1_dog_bits, up1_bsd_bits, up1_sakura_bits, up1_tomoyo_bits,
      &Up1Msk, up1_mask_bits, up1_mask_bits, up1_dog_mask_bits, up1_bsd_mask_bits, up1_sakura_mask_bits, up1_tomoyo_mask_bits },
    { &Up2GC, &Up2Xbm,  up2_bits, up2_tora_bits, up2_dog_bits, up2_bsd_bits, up2_sakura_bits, up2_tomoyo_bits,
      &Up2Msk, up2_mask_bits, up2_mask_bits, up2_dog_mask_bits, up2_bsd_mask_bits, up2_sakura_mask_bits, up2_tomoyo_mask_bits },
    { &Down1GC, &Down1Xbm,  down1_bits, down1_tora_bits, down1_dog_bits, down1_bsd_bits, down1_sakura_bits, down1_tomoyo_bits,
      &Down1Msk, down1_mask_bits, down1_mask_bits, down1_dog_mask_bits, down1_bsd_mask_bits, down1_sakura_mask_bits, down1_tomoyo_mask_bits },
    { &Down2GC, &Down2Xbm,  down2_bits, down2_tora_bits, down2_dog_bits, down2_bsd_bits, down2_sakura_bits, down2_tomoyo_bits,
      &Down2Msk, down2_mask_bits, down2_mask_bits, down2_dog_mask_bits, down2_bsd_mask_bits, down2_sakura_mask_bits, down2_tomoyo_mask_bits },
    { &Left1GC, &Left1Xbm,  left1_bits, left1_tora_bits, left1_dog_bits, left1_bsd_bits, left1_sakura_bits, left1_tomoyo_bits,
      &Left1Msk, left1_mask_bits, left1_mask_bits, left1_dog_mask_bits, left1_bsd_mask_bits, left1_sakura_mask_bits, left1_tomoyo_mask_bits },
    { &Left2GC, &Left2Xbm,  left2_bits, left2_tora_bits, left2_dog_bits, left2_bsd_bits, left2_sakura_bits, left2_tomoyo_bits,
      &Left2Msk, left2_mask_bits, left2_mask_bits, left2_dog_mask_bits, left2_bsd_mask_bits, left2_sakura_mask_bits, left2_tomoyo_mask_bits },
    { &Right1GC, &Right1Xbm,  right1_bits, right1_tora_bits, right1_dog_bits, right1_bsd_bits, right1_sakura_bits, right1_tomoyo_bits,
      &Right1Msk, right1_mask_bits, right1_mask_bits,right1_dog_mask_bits, right1_bsd_mask_bits, right1_sakura_mask_bits, right1_tomoyo_mask_bits },
    { &Right2GC, &Right2Xbm,  right2_bits, right2_tora_bits, right2_dog_bits, right2_bsd_bits, right2_sakura_bits, right2_tomoyo_bits,
      &Right2Msk, right2_mask_bits, right2_mask_bits, right2_dog_mask_bits, right2_bsd_mask_bits, right2_sakura_mask_bits, right2_tomoyo_mask_bits },
    { &UpLeft1GC, &UpLeft1Xbm,  upleft1_bits, upleft1_tora_bits, upleft1_dog_bits, upleft1_bsd_bits, upleft1_sakura_bits, upleft1_tomoyo_bits,
      &UpLeft1Msk, upleft1_mask_bits, upleft1_mask_bits, upleft1_dog_mask_bits, upleft1_bsd_mask_bits, upleft1_sakura_mask_bits, upleft1_tomoyo_mask_bits },
    { &UpLeft2GC, &UpLeft2Xbm,  upleft2_bits, upleft2_tora_bits, upleft2_dog_bits, upleft2_bsd_bits, upleft2_sakura_bits, upleft2_tomoyo_bits,
      &UpLeft2Msk, upleft2_mask_bits, upleft2_mask_bits,upleft2_dog_mask_bits, upleft2_bsd_mask_bits, upleft2_sakura_mask_bits, upleft2_tomoyo_mask_bits },
    { &UpRight1GC, &UpRight1Xbm,  upright1_bits, upright1_tora_bits, upright1_dog_bits, upright1_bsd_bits, upright1_sakura_bits, upright1_tomoyo_bits,
      &UpRight1Msk, upright1_mask_bits, upright1_mask_bits,upright1_dog_mask_bits, upright1_bsd_mask_bits, upright1_sakura_mask_bits, upright1_tomoyo_mask_bits },
    { &UpRight2GC, &UpRight2Xbm,  upright2_bits, upright2_tora_bits, upright2_dog_bits, upright2_bsd_bits, upright2_sakura_bits, upright2_tomoyo_bits,
      &UpRight2Msk, upright2_mask_bits, upright2_mask_bits,upright2_dog_mask_bits, upright2_bsd_mask_bits, upright2_sakura_mask_bits, upright2_tomoyo_mask_bits },
    { &DownLeft1GC, &DownLeft1Xbm,  dwleft1_bits, dwleft1_tora_bits, dwleft1_dog_bits, dwleft1_bsd_bits, dwleft1_sakura_bits, dwleft1_tomoyo_bits,
      &DownLeft1Msk, dwleft1_mask_bits, dwleft1_mask_bits, dwleft1_dog_mask_bits, dwleft1_bsd_mask_bits, dwleft1_sakura_mask_bits, dwleft1_tomoyo_mask_bits },
    { &DownLeft2GC, &DownLeft2Xbm,  dwleft2_bits, dwleft2_tora_bits, dwleft2_dog_bits, dwleft2_bsd_bits, dwleft2_sakura_bits, dwleft2_tomoyo_bits,
      &DownLeft2Msk, dwleft2_mask_bits, dwleft2_mask_bits, dwleft2_dog_mask_bits, dwleft2_bsd_mask_bits, dwleft2_sakura_mask_bits, dwleft2_tomoyo_mask_bits },
    { &DownRight1GC, &DownRight1Xbm,  dwright1_bits, dwright1_tora_bits, dwright1_dog_bits, dwright1_bsd_bits, dwright1_sakura_bits, dwright1_tomoyo_bits,
      &DownRight1Msk, dwright1_mask_bits, dwright1_mask_bits, dwright1_dog_mask_bits, dwright1_bsd_mask_bits, dwright1_sakura_mask_bits, dwright1_tomoyo_mask_bits },
    { &DownRight2GC, &DownRight2Xbm,  dwright2_bits, dwright2_tora_bits, dwright2_dog_bits, dwright2_bsd_bits, dwright2_sakura_bits, dwright2_tomoyo_bits,
      &DownRight2Msk, dwright2_mask_bits, dwright2_mask_bits, dwright2_dog_mask_bits, dwright2_bsd_mask_bits, dwright2_sakura_mask_bits, dwright2_tomoyo_mask_bits },
    { &UpTogi1GC, &UpTogi1Xbm,  utogi1_bits, utogi1_tora_bits, utogi1_dog_bits, utogi1_bsd_bits, utogi1_sakura_bits, utogi1_tomoyo_bits,
      &UpTogi1Msk, utogi1_mask_bits, utogi1_mask_bits, utogi1_dog_mask_bits, utogi1_bsd_mask_bits, utogi1_sakura_mask_bits, utogi1_tomoyo_mask_bits },
    { &UpTogi2GC, &UpTogi2Xbm,  utogi2_bits, utogi2_tora_bits, utogi2_dog_bits, utogi2_bsd_bits, utogi2_sakura_bits, utogi2_tomoyo_bits,
      &UpTogi2Msk, utogi2_mask_bits, utogi2_mask_bits, utogi2_dog_mask_bits, utogi2_bsd_mask_bits, utogi2_sakura_mask_bits, utogi2_tomoyo_mask_bits },
    { &DownTogi1GC, &DownTogi1Xbm,  dtogi1_bits, dtogi1_tora_bits, dtogi1_dog_bits, dtogi1_bsd_bits, dtogi1_sakura_bits, dtogi1_tomoyo_bits,
      &DownTogi1Msk, dtogi1_mask_bits, dtogi1_mask_bits, dtogi1_dog_mask_bits, dtogi1_bsd_mask_bits, dtogi1_sakura_mask_bits, dtogi1_tomoyo_mask_bits },
    { &DownTogi2GC, &DownTogi2Xbm,  dtogi2_bits, dtogi2_tora_bits, dtogi2_dog_bits, dtogi2_bsd_bits, dtogi2_sakura_bits, dtogi2_tomoyo_bits,
      &DownTogi2Msk, dtogi2_mask_bits, dtogi2_mask_bits, dtogi2_dog_mask_bits, dtogi2_bsd_mask_bits, dtogi2_sakura_mask_bits, dtogi2_tomoyo_mask_bits },
    { &LeftTogi1GC, &LeftTogi1Xbm,  ltogi1_bits, ltogi1_tora_bits, ltogi1_dog_bits, ltogi1_bsd_bits, ltogi1_sakura_bits, ltogi1_tomoyo_bits,
      &LeftTogi1Msk, ltogi1_mask_bits, ltogi1_mask_bits,ltogi1_dog_mask_bits, ltogi1_bsd_mask_bits, ltogi1_sakura_mask_bits, ltogi1_tomoyo_mask_bits },
    { &LeftTogi2GC, &LeftTogi2Xbm,  ltogi2_bits, ltogi2_tora_bits, ltogi2_dog_bits, ltogi2_bsd_bits, ltogi2_sakura_bits, ltogi2_tomoyo_bits,
      &LeftTogi2Msk, ltogi2_mask_bits, ltogi2_mask_bits,ltogi2_dog_mask_bits, ltogi2_bsd_mask_bits, ltogi2_sakura_mask_bits, ltogi2_tomoyo_mask_bits },
    { &RightTogi1GC, &RightTogi1Xbm,  rtogi1_bits, rtogi1_tora_bits, rtogi1_dog_bits, rtogi1_bsd_bits, rtogi1_sakura_bits, rtogi1_tomoyo_bits,
      &RightTogi1Msk, rtogi1_mask_bits, rtogi1_mask_bits,rtogi1_dog_mask_bits, rtogi1_bsd_mask_bits, rtogi1_sakura_mask_bits, rtogi1_tomoyo_mask_bits },
    { &RightTogi2GC, &RightTogi2Xbm,  rtogi2_bits, rtogi2_tora_bits, rtogi2_dog_bits, rtogi2_bsd_bits, rtogi2_sakura_bits, rtogi2_tomoyo_bits,
      &RightTogi2Msk, rtogi2_mask_bits, rtogi2_mask_bits,rtogi2_dog_mask_bits, rtogi2_bsd_mask_bits, rtogi2_sakura_mask_bits, rtogi2_tomoyo_mask_bits },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};

typedef struct {
    GC          *TickGCPtr;
    Pixmap      *TickMaskPtr;
} Animation;

Animation       AnimationPattern[][2] =
{
  { { &Mati2GC, &Mati2Msk },
    { &Mati2GC, &Mati2Msk } },          /* NekoState == NEKO_STOP */
  { { &Jare2GC, &Jare2Msk },
    { &Mati2GC, &Mati2Msk } },          /* NekoState == NEKO_JARE */
  { { &Kaki1GC, &Kaki1Msk },
    { &Kaki2GC, &Kaki2Msk } },          /* NekoState == NEKO_KAKI */
  { { &Mati3GC, &Mati3Msk },
    { &Mati3GC, &Mati3Msk } },          /* NekoState == NEKO_AKUBI */
  { { &Sleep1GC, &Sleep1Msk },
    { &Sleep2GC, &Sleep2Msk } },                /* NekoState == NEKO_SLEEP */
  { { &AwakeGC, &AwakeMsk },
    { &AwakeGC, &AwakeMsk } },          /* NekoState == NEKO_AWAKE */
  { { &Up1GC, &Up1Msk },
    { &Up2GC, &Up2Msk } },              /* NekoState == NEKO_U_MOVE */
  { { &Down1GC, &Down1Msk },
    { &Down2GC, &Down2Msk } },          /* NekoState == NEKO_D_MOVE */
  { { &Left1GC, &Left1Msk },
    { &Left2GC, &Left2Msk } },          /* NekoState == NEKO_L_MOVE */
  { { &Right1GC, &Right1Msk },
    { &Right2GC, &Right2Msk } },                /* NekoState == NEKO_R_MOVE */
  { { &UpLeft1GC, &UpLeft1Msk },
    { &UpLeft2GC, &UpLeft2Msk } },      /* NekoState == NEKO_UL_MOVE */
  { { &UpRight1GC, &UpRight1Msk },
    { &UpRight2GC, &UpRight2Msk } },    /* NekoState == NEKO_UR_MOVE */
  { { &DownLeft1GC, &DownLeft1Msk },
    { &DownLeft2GC, &DownLeft2Msk } },  /* NekoState == NEKO_DL_MOVE */
  { { &DownRight1GC, &DownRight1Msk },
    { &DownRight2GC, &DownRight2Msk } },        /* NekoState == NEKO_DR_MOVE */
  { { &UpTogi1GC, &UpTogi1Msk },
    { &UpTogi2GC, &UpTogi2Msk } },      /* NekoState == NEKO_U_TOGI */
  { { &DownTogi1GC, &DownTogi1Msk },
    { &DownTogi2GC, &DownTogi2Msk } },  /* NekoState == NEKO_D_TOGI */
  { { &LeftTogi1GC, &LeftTogi1Msk },
    { &LeftTogi2GC, &LeftTogi2Msk } },  /* NekoState == NEKO_L_TOGI */
  { { &RightTogi1GC, &RightTogi1Msk },
    { &RightTogi2GC, &RightTogi2Msk } },        /* NekoState == NEKO_R_TOGI */
};

static void NullFunction();

/*
 * Return the bitmap scaled to Scale.
 */

char *
ScaleBitmap(char *resource) {
  char *scaled, *src;
  unsigned int off, i, j, l, bytes, line;
  unsigned char c, k, bits, bitoff;

  if(!(scaled=calloc(l = BITMAP_WIDTH * BITMAP_HEIGHT / 8, Scale*Scale))) {
    fprintf(stderr, "%s: Can't allocate scaled bitmap", ProgramName);
    exit(1);
  }
  if(bytes=Scale/8, bits=Scale%8) {  /* general algorithm */
    for (off=0, bitoff=0, i=0,
         line=bytes*BITMAP_WIDTH+bits*BITMAP_WIDTH/8;
         i < l;
         ++i) {
      for (j=8, c=resource[i]; j--; c>>=1)
        if(c&1) {
          if(bitoff + bits >= 8) {
            scaled[off++] |= 0xff << bitoff;
            bitoff += bits - 8;
            memset(scaled + off, 0xff, bytes);
            scaled[off+=bytes] |= 0xff >> 8 - bitoff;
          }
          else {
            scaled[off] |= (0xff >> 8 - bits) << bitoff;
            bitoff += bits;
            if(bytes) {
              scaled[off] |= 0xff << bitoff;
              memset(scaled + off + 1, 0xff, bytes - 1);
              scaled[off+=bytes] |= 0xff >> 8 - bitoff;
            }
          }
        }
        else if((bitoff += bits) >= 8){
          off += bytes + 1; bitoff -= 8;
        }
        else off += bytes;
      if(!((i+1) % (BITMAP_WIDTH / 8))) {
        for (src=scaled+off-line, j=Scale; --j; off+=line)
          memcpy(scaled + off, src, line);
      }
    }
  }
  else { /* algorithm for multiples of 8 */
    for (off=0, i=0, line=bytes*BITMAP_WIDTH; i < l; ++i) {
      for (j=8, c=resource[i]; j--; c>>=1, off+=bytes)
        if(c&1) memset(scaled+off, 0xff, bytes);
      if(!((i+1) % (BITMAP_WIDTH / 8))) {
        for (src=scaled+off-line, j=Scale; --j; off+=line)
          memcpy(scaled + off, src, line);
      }
    }
  }
  return scaled;
}

/*
 *      ビットマップデータ・GC 初期化
 */

void
InitBitmapAndGCs()
{
    BitmapGCData        *BitmapGCDataTablePtr;
    XGCValues           theGCValues;
    char *bm;

    theGCValues.function = GXcopy;
    theGCValues.foreground = theForegroundColor.pixel;
    theGCValues.background = theBackgroundColor.pixel;
    theGCValues.fill_style = FillTiled;
    theGCValues.ts_x_origin = 0;
    theGCValues.ts_y_origin = 0;

    for (BitmapGCDataTablePtr = BitmapGCDataTable;
         BitmapGCDataTablePtr->GCCreatePtr != NULL;
         BitmapGCDataTablePtr++) {


        bm = (Scale == 1)
             ?BitmapGCDataTablePtr->PixelPattern[NekoMoyou]
             :ScaleBitmap(BitmapGCDataTablePtr->PixelPattern[NekoMoyou]);
        *(BitmapGCDataTablePtr->BitmapCreatePtr)
            = XCreatePixmapFromBitmapData(theDisplay, theRoot,
                bm, BitmapWidth, BitmapHeight,
                theForegroundColor.pixel,
                theBackgroundColor.pixel,
                DefaultDepth(theDisplay, theScreen));
        if(Scale != 1) free(bm);

        theGCValues.tile = *(BitmapGCDataTablePtr->BitmapCreatePtr);

        bm = (Scale == 1)
             ?BitmapGCDataTablePtr->MaskPattern[NekoMoyou]
             :ScaleBitmap(BitmapGCDataTablePtr->MaskPattern[NekoMoyou]);
        *(BitmapGCDataTablePtr->BitmapMasksPtr)
            = XCreateBitmapFromData(theDisplay, theRoot,
                bm, BitmapWidth, BitmapHeight);
        if(Scale != 1) free(bm);

        *(BitmapGCDataTablePtr->GCCreatePtr)
            = XCreateGC(theDisplay, theWindow,
                GCFunction | GCForeground | GCBackground | GCTile |
                GCTileStipXOrigin | GCTileStipYOrigin | GCFillStyle,
                &theGCValues);
    }
}

/*
 *      リソース・データベースから必要なリソースを取り出す
 */

char *
NekoGetDefault(char *resource)
{
        char    *value;

        if (value = XGetDefault(theDisplay, ProgramName, resource)) {
                return value;
        }
        if (value = XGetDefault(theDisplay, ClassName, resource)) {
                return value;
        }
        return NULL;
}

/*
 *      リソース・データベースからオプションを設定
 */

void
GetResources()
{
  char  *resource;
  int           num;
  int loop;
  if (Foreground == NULL) {
    if ((resource = NekoGetDefault("foreground")) != NULL) {
      Foreground = resource;
    }
  }

  if (Background == NULL) {
    if ((resource = NekoGetDefault("background")) != NULL) {
      Background = resource;
    }
  }

  if (IntervalTime == 0) {
    if ((resource = NekoGetDefault("time")) != NULL) {
      if (num = atoi(resource)) {
        IntervalTime = num;
      }
    }
  }

  if (NekoSpeed == (double)0) {
    if ((resource = NekoGetDefault("speed")) != NULL) {
      if (num = atoi(resource)) {
        NekoSpeed = (double)num;
      }
    }
  }

  if (IdleSpace == 0) {
    if ((resource = NekoGetDefault("idle")) != NULL) {
      if (num = atoi(resource)) {
        IdleSpace = num;
      }
    }
  }

  if (NekoMoyou == NOTDEFINED) {
    for (loop=0;loop<BITMAPTYPES;loop++)
      if ((resource = NekoGetDefault(AnimalDefaultsDataTable[loop].name)) != NULL) {
        if (IsTrue(resource))
          NekoMoyou = loop;
      }
  }

  if (NoShape == NOTDEFINED) {
    if ((resource = NekoGetDefault("noshape")) != NULL) {
      NoShape = IsTrue(resource);
    }
  }

  if (ReverseVideo == NOTDEFINED) {
    if ((resource = NekoGetDefault("reverse")) != NULL) {
      ReverseVideo = IsTrue(resource);
    }
  }

  if (Foreground == NULL) {
    Foreground = DEFAULT_FOREGROUND;
  }
  if (Background == NULL) {
    Background = DEFAULT_BACKGROUND;
  }
  if (NekoMoyou == NOTDEFINED) {
    NekoMoyou = 0;
  }
  if (IntervalTime == 0) {
    IntervalTime = AnimalDefaultsDataTable[NekoMoyou].time;
  }
  if (NekoSpeed == (double)0) {
    NekoSpeed = (double)(AnimalDefaultsDataTable[NekoMoyou].speed);
  }
  if (IdleSpace == 0) {
    IdleSpace = AnimalDefaultsDataTable[NekoMoyou].idle;
  }
  XOffset = XOffset + AnimalDefaultsDataTable[NekoMoyou].off_x;
  YOffset = YOffset + AnimalDefaultsDataTable[NekoMoyou].off_y;
  if (NoShape == NOTDEFINED) {
    NoShape = False;
  }
  if (ReverseVideo == NOTDEFINED) {
    ReverseVideo = False;
  }
  if (ToWindow == NOTDEFINED) {
    ToWindow = False;
  }
  if (ToFocus == NOTDEFINED) {
    ToFocus = False;
  }
}

/*
 *      ねずみ型カーソルを作る
 */

void
MakeMouseCursor()
{
    Pixmap                      theCursorSource;
    Pixmap                      theCursorMask;

   if(NoCursor) return;

    theCursorSource
        = XCreateBitmapFromData(theDisplay, theRoot,
                                AnimalDefaultsDataTable[NekoMoyou].cursor,
                                AnimalDefaultsDataTable[NekoMoyou].cursor_width,
                                AnimalDefaultsDataTable[NekoMoyou].cursor_height);

    theCursorMask
        = XCreateBitmapFromData(theDisplay, theRoot,
                                AnimalDefaultsDataTable[NekoMoyou].mask,
                                AnimalDefaultsDataTable[NekoMoyou].cursor_width,
                                AnimalDefaultsDataTable[NekoMoyou].cursor_height);

    theCursor = XCreatePixmapCursor(theDisplay, theCursorSource, theCursorMask,
                                    &theBackgroundColor, &theForegroundColor,
                                    AnimalDefaultsDataTable[NekoMoyou].cursor_x_hot,
                                    AnimalDefaultsDataTable[NekoMoyou].cursor_y_hot);
    XFreePixmap(theDisplay,theCursorSource);
    XFreePixmap(theDisplay,theCursorMask);
}

/*
 *      色を初期設定する
 */

void
SetupColors()
{
    XColor      theExactColor;
    Colormap    theColormap;

    theColormap = DefaultColormap(theDisplay, theScreen);

    if (theDepth == 1) {
        Foreground = "black";
        Background = "white";
    }

    if (ReverseVideo == True) {
        char    *tmp;

        tmp = Foreground;
        Foreground = Background;
        Background = tmp;
    }

    if (!XAllocNamedColor(theDisplay, theColormap,
                Foreground, &theForegroundColor, &theExactColor)) {
        fprintf(stderr, "%s: Can't XAllocNamedColor(\"%s\").\n",
                ProgramName, Foreground);
        exit(1);
    }

    if (!XAllocNamedColor(theDisplay, theColormap,
                Background, &theBackgroundColor, &theExactColor)) {
        fprintf(stderr, "%s: Can't XAllocNamedColor(\"%s\").\n",
                ProgramName, Background);
        exit(1);
    }
}

/*
 * Routine to let user select a window using the mouse
 *
 * This routine originate in dsimple.c
 */

Window SelectWindow(Display *dpy)
{
  int status;
  Cursor cursor;
  XEvent event;
  Window target_win = None, root = theRoot;
  int buttons = 0;

  /* Make the target cursor */
  cursor = theCursor;

  /* Grab the pointer using target cursor, letting it room all over */
  status = XGrabPointer(dpy, root, False,
                        ButtonPressMask|ButtonReleaseMask, GrabModeSync,
                        GrabModeAsync, root, cursor, CurrentTime);
  if (status != GrabSuccess) {
    fprintf(stderr, "%s: Can't grab the mouse.\n", ProgramName);
    exit(1);
  }

  /* Let the user select a window... */
  while ((target_win == None) || (buttons != 0)) {
    /* allow one more event */
    XAllowEvents(dpy, SyncPointer, CurrentTime);
    XWindowEvent(dpy, root, ButtonPressMask|ButtonReleaseMask, &event);
    switch (event.type) {
    case ButtonPress:
      if (target_win == None) {
        target_win = event.xbutton.subwindow; /* window selected */
        if (target_win == None) target_win = root;
      }
      buttons++;
      break;
    case ButtonRelease:
      if (buttons > 0) /* there may have been some down before we started */
        buttons--;
       break;
    }
  }

  XUngrabPointer(dpy, CurrentTime);      /* Done with pointer */

  return(target_win);
}

/*
 * WindowWithName: routine to locate a window with a given name on a display.
 *                 If no window with the given name is found, 0 is returned.
 *                 If more than one window has the given name, the first
 *                 one found will be returned.  Only top and its subwindows
 *                 are looked at.  Normally, top should be the RootWindow.
 *
 * This routine originate in dsimple.c
 */
Window WindowWithName(Display *dpy, Window top, char *name)
{
        Window *children, dummy;
        unsigned int nchildren;
        int i;
        Window w=0;
        char *window_name;

        if (XFetchName(dpy, top, &window_name) && !strcmp(window_name, name))
          return(top);

        if (!XQueryTree(dpy, top, &dummy, &dummy, &children, &nchildren))
          return(0);

        for (i=0; i<nchildren; i++) {
                w = WindowWithName(dpy, children[i], name);
                if (w)
                  break;
        }
        if (children) XFree ((char *)children);
        return(w);
}

/*
 *      スクリーン環境初期化
 */

void
InitScreen(char *DisplayName)
{
  XSetWindowAttributes  theWindowAttributes;
  unsigned long         theWindowMask;
  Window                        theTempRoot;
  int                           WindowPointX;
  int                           WindowPointY;
  unsigned int          BorderWidth;
  int                           event_base, error_base;

  if ((theDisplay = XOpenDisplay(DisplayName)) == NULL) {
    fprintf(stderr, "%s: Can't open display", ProgramName);
    if (DisplayName != NULL) {
      fprintf(stderr, " %s.\n", DisplayName);
    } else {
      fprintf(stderr, ".\n");
    }
    exit(1);
  }

  GetResources();

  if (Synchronous == True) {
    fprintf(stderr,"Synchronizing.\n");
    XSynchronize(theDisplay,True);
  }

  if (!NoShape && XShapeQueryExtension(theDisplay,
                                       &event_base, &error_base) == False) {
    fprintf(stderr, "Display not suported shape extension.\n");
    NoShape = True;
  }

  theScreen = DefaultScreen(theDisplay);
  theDepth = DefaultDepth(theDisplay, theScreen);

  theRoot = RootWindow(theDisplay, theScreen);

  XGetGeometry(theDisplay, theRoot, &theTempRoot,
               &WindowPointX, &WindowPointY,
               &WindowWidth, &WindowHeight,
               &BorderWidth, &theDepth);

  SetupColors();
  MakeMouseCursor();

  if (ToWindow && theTarget == None) {
    if (TargetName != NULL) {
      int i;

      for (i=0; i<5; i++) {
        theTarget = WindowWithName(theDisplay, theRoot, TargetName);
        if (theTarget != None) break;
      }
      if (theTarget == None) {
        fprintf(stderr, "%s: No window with name '%s' exists.\n",
                ProgramName, TargetName);
        exit(1);
      }
    } else {
      theTarget = SelectWindow(theDisplay);
      if (theTarget == theRoot) {
        theTarget = None;
        ToWindow = False;
      }
    }
    if (theTarget != None) {
      Window            QueryRoot, QueryParent, *QueryChildren;
      unsigned int      nchild;

      for (;;) {
        if (XQueryTree(theDisplay, theTarget, &QueryRoot,
                       &QueryParent, &QueryChildren, &nchild)) {
          XFree(QueryChildren);
          if (QueryParent == QueryRoot) break;
          theTarget = QueryParent;
        }
        else {
          fprintf(stderr, "%s: Target Lost.\n",ProgramName);
          exit(1);
        }
      }
    }
  }

  theWindowAttributes.background_pixel = theBackgroundColor.pixel;
  if(!NoCursor) theWindowAttributes.cursor = theCursor;
  theWindowAttributes.override_redirect = True;

  if (!ToWindow) XChangeWindowAttributes(theDisplay, theRoot, (NoCursor?0:CWCursor),
                                         &theWindowAttributes);

  theWindowMask = CWBackPixel | (NoCursor?0:CWCursor) | CWOverrideRedirect;

  theWindow = XCreateWindow(theDisplay, theRoot, 0, 0,
                            BitmapWidth, BitmapHeight,
                            0, theDepth, InputOutput, CopyFromParent,
                            theWindowMask, &theWindowAttributes);

  if (WindowName == NULL) WindowName = ProgramName;
  XStoreName(theDisplay, theWindow, WindowName);

  InitBitmapAndGCs();

  XSelectInput(theDisplay, theWindow,
               ExposureMask|VisibilityChangeMask|KeyPressMask);

  XFlush(theDisplay);
}


/*
 *      SIGINT シグナル処理
 */

void
RestoreCursor()
{
  XSetWindowAttributes  theWindowAttributes;
  BitmapGCData *BitmapGCDataTablePtr;

  if(!NoCursor) {
    theWindowAttributes.cursor = None;
    XChangeWindowAttributes(theDisplay, theRoot, CWCursor,
                            &theWindowAttributes);
    XFreeCursor(theDisplay,theCursor);
  }
  else XChangeWindowAttributes(theDisplay, theRoot, 0,
                               &theWindowAttributes);
  for (BitmapGCDataTablePtr = BitmapGCDataTable;
       BitmapGCDataTablePtr->GCCreatePtr != NULL;
       BitmapGCDataTablePtr++) {
    XFreePixmap(theDisplay,*(BitmapGCDataTablePtr->BitmapCreatePtr));
    XFreePixmap(theDisplay,*(BitmapGCDataTablePtr->BitmapMasksPtr));
    XFreeGC(theDisplay,*(BitmapGCDataTablePtr->GCCreatePtr));
       }
  XCloseDisplay(theDisplay);
  exit(0);
}


/*
 *      インターバル
 *
 *      　この関数を呼ぶと、ある一定の時間返ってこなくなる。猫
 *      の動作タイミング調整に利用すること。
 */

void
Interval()
{
    pause();
    if (RaiseWindowDelay>0)
      RaiseWindowDelay--;
}


/*
 *      ティックカウント処理
 */

void
TickCount()
{
    if (++NekoTickCount >= MAX_TICK) {
        NekoTickCount = 0;
    }

    if (NekoTickCount % 2 == 0) {
        if (NekoStateCount < MAX_TICK) {
            NekoStateCount++;
        }
    }
}


/*
 *      猫状態設定
 */

void
SetNekoState(int SetValue)
{
    NekoTickCount = 0;
    NekoStateCount = 0;

    NekoState = SetValue;
}


/*
 *      猫描画処理
 */

void
DrawNeko(int x, int y, Animation DrawAnime)
{
/*@@@@@@*/
    register GC         DrawGC = *(DrawAnime.TickGCPtr);
    register Pixmap     DrawMask = *(DrawAnime.TickMaskPtr);

    if ((x != NekoLastX) || (y != NekoLastY)
                || (DrawGC != NekoLastGC)) {
      XWindowChanges    theChanges;

      theChanges.x = x;
      theChanges.y = y;
      XConfigureWindow(theDisplay, theWindow, CWX | CWY, &theChanges);
      if (NoShape == False) {
        XShapeCombineMask(theDisplay, theWindow, ShapeBounding,
                          0, 0, DrawMask, ShapeSet);

      }
      if (DontMapped) {
        XMapWindow(theDisplay, theWindow);
        DontMapped = 0;
      }
      XFillRectangle(theDisplay, theWindow, DrawGC,
                     0, 0, BitmapWidth, BitmapHeight);
    }

    XFlush(theDisplay);

    NekoLastX = x;
    NekoLastY = y;

    NekoLastGC = DrawGC;
}


/*
 *      猫再描画処理
 */

void
RedrawNeko()
{
  XFillRectangle(theDisplay, theWindow, NekoLastGC,
                 0, 0, BitmapWidth, BitmapHeight);

  XFlush(theDisplay);
}


/*
 *      猫移動方法決定
 *
 *      This sets the direction that the neko is moving.
 *
 */

void
NekoDirection()
{
    int                 NewState;
    double              LargeX, LargeY;
    double              Length;
    double              SinTheta;

    if (NekoMoveDx == 0 && NekoMoveDy == 0) {
        NewState = NEKO_STOP;
    } else {
        LargeX = (double)NekoMoveDx;
        LargeY = (double)(-NekoMoveDy);
        Length = sqrt(LargeX * LargeX + LargeY * LargeY);
        SinTheta = LargeY / Length;

        if (NekoMoveDx > 0) {
            if (SinTheta > SinPiPer8Times3) {
                NewState = NEKO_U_MOVE;
            } else if ((SinTheta <= SinPiPer8Times3)
                        && (SinTheta > SinPiPer8)) {
                NewState = NEKO_UR_MOVE;
            } else if ((SinTheta <= SinPiPer8)
                        && (SinTheta > -(SinPiPer8))) {
                NewState = NEKO_R_MOVE;
            } else if ((SinTheta <= -(SinPiPer8))
                        && (SinTheta > -(SinPiPer8Times3))) {
                NewState = NEKO_DR_MOVE;
            } else {
                NewState = NEKO_D_MOVE;
            }
        } else {
            if (SinTheta > SinPiPer8Times3) {
                NewState = NEKO_U_MOVE;
            } else if ((SinTheta <= SinPiPer8Times3)
                        && (SinTheta > SinPiPer8)) {
                NewState = NEKO_UL_MOVE;
            } else if ((SinTheta <= SinPiPer8)
                        && (SinTheta > -(SinPiPer8))) {
                NewState = NEKO_L_MOVE;
            } else if ((SinTheta <= -(SinPiPer8))
                        && (SinTheta > -(SinPiPer8Times3))) {
                NewState = NEKO_DL_MOVE;
            } else {
                NewState = NEKO_D_MOVE;
            }
        }
    }

    if (NekoState != NewState) {
        SetNekoState(NewState);
    }
}


/*
 *      猫壁ぶつかり判定
 */

Bool
IsWindowOver()
{
    Bool        ReturnValue = False;

    if (NekoY <= 0) {
        NekoY = 0;
        ReturnValue = True;
    } else if (NekoY >= WindowHeight - BitmapHeight) {
        NekoY = WindowHeight - BitmapHeight;
        ReturnValue = True;
    }
    if (NekoX <= 0) {
        NekoX = 0;
        ReturnValue = True;
    } else if (NekoX >= WindowWidth - BitmapWidth) {
        NekoX = WindowWidth - BitmapWidth;
        ReturnValue = True;
    }

    return(ReturnValue);
}


/*
 *      猫移動状況判定
 */

Bool
IsNekoDontMove()
{
    if (NekoX == NekoLastX && NekoY == NekoLastY) {
        return(True);
    } else {
        return(False);
    }
}


/*
 *      猫移動開始判定
 */

Bool
IsNekoMoveStart()
{
    if ((PrevMouseX >= MouseX - IdleSpace
         && PrevMouseX <= MouseX + IdleSpace) &&
         (PrevMouseY >= MouseY - IdleSpace
         && PrevMouseY <= MouseY + IdleSpace) &&
        (PrevTarget == theTarget)) {
        return(False);
    } else {
        return(True);
    }
}


/*
 *      猫移動 dx, dy 計算
 */

void
CalcDxDy()
{
    Window              QueryRoot, QueryChild;
    int                 AbsoluteX, AbsoluteY;
    int                 RelativeX, RelativeY;
    unsigned int        ModKeyMask;
    double              LargeX, LargeY;
    double              DoubleLength, Length;

    XQueryPointer(theDisplay, theWindow,
                   &QueryRoot, &QueryChild,
                   &AbsoluteX, &AbsoluteY,
                   &RelativeX, &RelativeY,
                   &ModKeyMask);

    PrevMouseX = MouseX;
    PrevMouseY = MouseY;
    PrevTarget = theTarget;

    MouseX = AbsoluteX+XOffset;
    MouseY = AbsoluteY+YOffset;

    if (ToFocus) {
      int               revert;

      XGetInputFocus(theDisplay, &theTarget, &revert);

      if (theTarget != theRoot
          && theTarget != PointerRoot && theTarget != None) {
        Window          QueryParent, *QueryChildren;
        unsigned int    nchild;

        for (;;) {
          if (XQueryTree(theDisplay, theTarget, &QueryRoot,
                         &QueryParent, &QueryChildren, &nchild)) {
            XFree(QueryChildren);
            if (QueryParent == QueryRoot) break;
            theTarget = QueryParent;
          }
          else {
            theTarget = None;
            break;
          }
        }
      }
      else {
        theTarget = None;
      }
    }

    if ((ToWindow || ToFocus) && theTarget != None) {
      int                       status;
      XWindowAttributes         theTargetAttributes;

      status =
        XGetWindowAttributes(theDisplay, theTarget, &theTargetAttributes);

      if (ToWindow && status == 0) {
        fprintf(stderr, "%s: '%s', Target Lost.\n",ProgramName, WindowName);
        RestoreCursor();
      }

      if (theTargetAttributes.x+theTargetAttributes.width > 0
          && theTargetAttributes.x < (int)WindowWidth
          && theTargetAttributes.y+theTargetAttributes.height > 0
          && theTargetAttributes.y < (int)WindowHeight
          && theTargetAttributes.map_state == IsViewable) {
        if (ToFocus) {
          if (MouseX < theTargetAttributes.x+BitmapWidth/2)
            LargeX = (double)(theTargetAttributes.x + XOffset - NekoX);
          else if (MouseX > theTargetAttributes.x+theTargetAttributes.width
                   -BitmapWidth/2)
            LargeX = (double)(theTargetAttributes.x + theTargetAttributes.width
                              + XOffset - NekoX - (double)(BitmapWidth));
          else
            LargeX = (double)(MouseX - NekoX - (double)(BitmapWidth / 2));

          LargeY = (double)(theTargetAttributes.y
                            + YOffset - NekoY - (double)(BitmapHeight));
        }
        else {
          MouseX = theTargetAttributes.x
            + theTargetAttributes.width / 2 + XOffset;
          MouseY = theTargetAttributes.y + YOffset;
          LargeX = (double)(MouseX - NekoX - (double)(BitmapWidth / 2));
          LargeY = (double)(MouseY - NekoY - (double)(BitmapHeight));
        }
      }
      else {
        LargeX = (double)(MouseX - NekoX - (double)(BitmapWidth / 2));
        LargeY = (double)(MouseY - NekoY - (double)(BitmapHeight));
      }
    }
    else {
      LargeX = (double)(MouseX - NekoX - (double)(BitmapWidth / 2));
      LargeY = (double)(MouseY - NekoY - (double)(BitmapHeight));
    }

    DoubleLength = LargeX * LargeX + LargeY * LargeY;

    if (DoubleLength != (double)0) {
        Length = sqrt(DoubleLength);
        if (Length <= NekoSpeed) {
            NekoMoveDx = (int)LargeX;
            NekoMoveDy = (int)LargeY;
        } else {
            NekoMoveDx = (int)((NekoSpeed * LargeX) / Length);
            NekoMoveDy = (int)((NekoSpeed * LargeY) / Length);
        }
    } else {
        NekoMoveDx = NekoMoveDy = 0;
    }
}


/*
 *      動作解析猫描画処理
 */

void
NekoThinkDraw()
{
    CalcDxDy();

    if (NekoState != NEKO_SLEEP) {
        DrawNeko(NekoX, NekoY,
                AnimationPattern[NekoState][NekoTickCount & 0x1]);
    } else {
        DrawNeko(NekoX, NekoY,
                AnimationPattern[NekoState][(NekoTickCount >> 2) & 0x1]);
    }

    TickCount();

    switch (NekoState) {
    case NEKO_STOP:
        if (IsNekoMoveStart()) {
            SetNekoState(NEKO_AWAKE);
            break;
        }
        if (NekoStateCount < NEKO_STOP_TIME) {
            break;
        }
        if (NekoMoveDx < 0 && NekoX <= 0) {
            SetNekoState(NEKO_L_TOGI);
        } else if (NekoMoveDx > 0 && NekoX >= WindowWidth - BitmapWidth) {
            SetNekoState(NEKO_R_TOGI);
        } else if ((NekoMoveDy < 0 && NekoY <= 0)
                   || (ToFocus && theTarget != None && NekoY > MouseY)){
            SetNekoState(NEKO_U_TOGI);
        } else if ((NekoMoveDy > 0 && NekoY >= WindowHeight - BitmapHeight)
                   || (ToFocus && theTarget != None
                       &&  NekoY < MouseY - BitmapHeight)){
            SetNekoState(NEKO_D_TOGI);
        } else {
            SetNekoState(NEKO_JARE);
        }
        break;
    case NEKO_JARE:
        if (IsNekoMoveStart()) {
            SetNekoState(NEKO_AWAKE);
            break;
        }
        if (NekoStateCount < NEKO_JARE_TIME) {
            break;
        }
        SetNekoState(NEKO_KAKI);
        break;
    case NEKO_KAKI:
        if (IsNekoMoveStart()) {
            SetNekoState(NEKO_AWAKE);
            break;
        }
        if (NekoStateCount < NEKO_KAKI_TIME) {
            break;
        }
        SetNekoState(NEKO_AKUBI);
        break;
    case NEKO_AKUBI:
        if (IsNekoMoveStart()) {
            SetNekoState(NEKO_AWAKE);
            break;
        }
        if (NekoStateCount < NEKO_AKUBI_TIME) {
            break;
        }
        SetNekoState(NEKO_SLEEP);
        break;
    case NEKO_SLEEP:
        if (NekoStateCount < NekoSleepTime) {
            break;
        }
        if (IsNekoMoveStart()) {
            SetNekoState(NEKO_AWAKE);
            break;
        }
        break;
    case NEKO_AWAKE:
        if (NekoStateCount < NEKO_AWAKE_TIME) {
            break;
        }
        NekoDirection();        /* 猫が動く向きを求める */
        break;
    case NEKO_U_MOVE:
    case NEKO_D_MOVE:
    case NEKO_L_MOVE:
    case NEKO_R_MOVE:
    case NEKO_UL_MOVE:
    case NEKO_UR_MOVE:
    case NEKO_DL_MOVE:
    case NEKO_DR_MOVE:
        NekoX += NekoMoveDx;
        NekoY += NekoMoveDy;
        NekoDirection();
        if (IsWindowOver()) {
            if (IsNekoDontMove()) {
                SetNekoState(NEKO_STOP);
            }
        }
        break;
    case NEKO_U_TOGI:
    case NEKO_D_TOGI:
    case NEKO_L_TOGI:
    case NEKO_R_TOGI:
        if (IsNekoMoveStart()) {
            SetNekoState(NEKO_AWAKE);
            break;
        }
        if (NekoStateCount < NEKO_TOGI_TIME) {
            break;
        }
        SetNekoState(NEKO_KAKI);
        break;
    default:
        /* Internal Error */
        SetNekoState(NEKO_STOP);
        break;
    }

    Interval();
}


/*
 *      キーイベント処理
 */

Bool
ProcessKeyPress(XKeyEvent *theKeyEvent)
{
  int                   Length;
  int                   theKeyBufferMaxLen = AVAIL_KEYBUF;
  char          theKeyBuffer[AVAIL_KEYBUF + 1];
  KeySym                theKeySym;
  XComposeStatus        theComposeStatus;
  Bool          ReturnState;

  ReturnState = True;

  Length = XLookupString(theKeyEvent,
                         theKeyBuffer, theKeyBufferMaxLen,
                         &theKeySym, &theComposeStatus);

  if (Length > 0) {
    switch (theKeyBuffer[0]) {
    case 'q':
    case 'Q':
      if (theKeyEvent->state & Mod1Mask) { /* META (Alt) キー */
        ReturnState = False;
      }
      break;
    default:
      break;
    }
  }

  return(ReturnState);
}


/*
 *      イベント処理
 */

Bool
ProcessEvent()
{
    XEvent      theEvent;
    Bool        ContinueState = True;

    while (XPending(theDisplay)) {
        XNextEvent(theDisplay,&theEvent);
        switch (theEvent.type) {
        case Expose:
            if (theEvent.xexpose.count == 0) {
                RedrawNeko();
            }
            break;
        case KeyPress:
            ContinueState = ProcessKeyPress(&theEvent.xkey);
            if (!ContinueState) {
                    return(ContinueState);
            }
            break;
        case VisibilityNotify:
            if (RaiseWindowDelay==0) {
              XRaiseWindow(theDisplay,theWindow);
              RaiseWindowDelay=DEFAULT_RAISE_WAIT;
            }
        default:
            /* Unknown Event */
            break;
        }
    }

    return(ContinueState);
}


/*
 *      猫処理
 */

void
ProcessNeko()
{
  struct itimerval      Value;

  /* 猫の初期化 */

  NekoX = (WindowWidth - BitmapWidth / 2) / 2;
  NekoY = (WindowHeight - BitmapHeight / 2) / 2;

  NekoLastX = NekoX;
  NekoLastY = NekoY;

  SetNekoState(NEKO_STOP);

  /* タイマー設定 */

  timerclear(&Value.it_interval);
  timerclear(&Value.it_value);

  Value.it_interval.tv_usec = IntervalTime;
  Value.it_value.tv_usec = IntervalTime;

  setitimer(ITIMER_REAL, &Value, 0);

  /* メイン処理 */

  do {
    NekoThinkDraw();
  } while (ProcessEvent());
}


/*
 *      SIGALRM シグナル処理
 */

static void
NullFunction()
{
  /* No Operation */
#if defined(SYSV) || defined(SVR4)
  signal(SIGALRM, NullFunction);
#endif /* SYSV || SVR4 */
}

/*
 *      エラー処理
 */

int
NekoErrorHandler(Display *dpy, XErrorEvent *err)
{
  if (err->error_code==BadWindow && (ToWindow || ToFocus)) {
    return 0;
  } else {
    char msg[80];
    XGetErrorText(dpy, err->error_code, msg, 80);
    fprintf(stderr, "%s: Error and exit.\n%s\n", ProgramName, msg);
    exit(1);
  }
}


/*
 *      Usage
 */

char    *message[] = {
"",
"Options are:",
"-pet <pet>             : select a pet appearing in a new window",
"-display <display>     : Neko appears on specified display.",
"-fg <color>            : Foreground color",
"-bg <color>            : Background color",
"-scale <integer>       : scale all bitmaps by a factor",
"-speed <dots>          : change pixels traveled in one move",
"-sleeptime <tick time> : delay waking up",
"-time <microseconds>   : change delay between all animations",
"-idle <dots>",
"-name <name>           : set window name of neko.",
"-nocursor              : do not modify the cursor.",
"-towindow              : Neko chases selected window.",
"-toname <name>         : Neko chases specified window.",
"-tofocus               : Neko runs on top of focus window",
"-rv                    : Reverse video. (effects monochrome display only)",
"-position <geometry>   : adjust position relative to mouse pointer.",
"-debug                 : puts you in synchronous mode.",
"-patchlevel            : print out your current patchlevel.",
"-<pet name>            : change the currently selected pet.",
"-h | -help             : print this message.",
NULL };

void
Usage()
{
  char  **mptr;
  int loop;

  mptr = message;
  fprintf(stderr, "Usage: %s [<global options>]"
                  " -pet [<pet name>] [<pet options>] ...\n", ProgramName);
  while (*mptr) {
    fprintf(stderr,"%s\n", *mptr);
    mptr++;
  }
  fputs("\nPets are:\n", stderr);
  for (loop=0;loop<BITMAPTYPES;loop++)
    fprintf(stderr,"%s Uses %s bitmaps\n",AnimalDefaultsDataTable[loop].name,AnimalDefaultsDataTable[loop].name);
}

unsigned long djb2(char*s) {
  unsigned long r; char c;
  for(r=5381; c=*s++; r += (r << 5) + c);
  return r;
}

/*
 *      オプションの理解
 */

void
GetArguments(int argc, char *argv[], char *theDisplayName)
{
  char *av=0;
  int           ArgCounter;
  extern int XOffset,YOffset;
  int loop,found=0;

  theDisplayName[0] = '\0';

  for (ArgCounter = 0; ArgCounter < argc; ArgCounter++) {
    switch(djb2(argv[ArgCounter])) {
      case 0x17c77a67b: /* -pet */
      do {
        ArgCounter++;
        if (ArgCounter >= argc) {
          fprintf(stderr, "%s: -pet option error.\n", ProgramName);
          exit(1);
        }
        av = argv[ArgCounter];
        if (strcmp(av, "bsd") == 0)
          av = "bsd_daemon";
        for (loop=0;loop<BITMAPTYPES;loop++) {
          if (strcmp(av,AnimalDefaultsDataTable[loop].name)==0)
            {NekoMoyou = loop;found=1;}
        }
        if (!found) {
          fprintf(stderr,
                  "%s: Unknown pet \"%s\".\n", ProgramName,
                  argv[ArgCounter]);
          exit(1);
        }
        for (loop=ArgCounter; ++loop < argc;)
          if(!strcmp(argv[loop], "-pet")) break;
        if(loop != argc)
          if(fork()) {
            GetArguments(loop  - ArgCounter - 1,
                         argv + ArgCounter + 1,
                         theDisplayName);
            return;
          }
          else ArgCounter=loop;
        else break;
      } while(1);
      break;
      case 0x1ae4d112535fc8: /* -display */
      ArgCounter++;
      if (ArgCounter < argc) {
        strcpy(theDisplayName, argv[ArgCounter]);
      } else {
        fprintf(stderr, "%s: -display option error.\n", ProgramName);
        exit(1);
      }
      break;
      case 0xb877fdf: /* -fg */
      case 0xbf536549077c5bcd: /* -foreground */
      ArgCounter++;
      Foreground = argv[ArgCounter];
      break;
      case 0xb877f5b: /* -bg */
      case 0xbf52a9f58e85dc92: /* -background */
      ArgCounter++;
      Background = argv[ArgCounter];
      break;
      case 0x65279301f7a: /* -scale */
      ArgCounter++;
      if (ArgCounter < argc) {
        Scale = atol(argv[ArgCounter]);
        BitmapWidth *= Scale;
        BitmapHeight *= Scale;
      } else {
        fprintf(stderr, "%s: -scale option error.\n", ProgramName);
        exit(1);
      }
      break;
      case 0x65279375083: /* -speed */
      ArgCounter++;
      if (ArgCounter < argc) {
        NekoSpeed = atof(argv[ArgCounter]);
      } else {
        fprintf(stderr, "%s: -speed option error.\n", ProgramName);
        exit(1);
      }
      break;
      case 0x726770a83db5159a: /* -sleeptime */
      ArgCounter++;
      if (ArgCounter < argc) {
        NekoSleepTime = atoi(argv[ArgCounter]);
      } else {
        fprintf(stderr, "%s: -sleeptime option error.\n", ProgramName);
        exit(1);
      }
      break;
      case 0x310b6eb7e1: /* -time */
      ArgCounter++;
      if (ArgCounter < argc) {
        IntervalTime = atol(argv[ArgCounter]);
      } else {
        fprintf(stderr, "%s: -time option error.\n", ProgramName);
        exit(1);
      }
      break;
      case 0x310b689a50: /* -idle */
      ArgCounter++;
      if (ArgCounter < argc) {
        IdleSpace = atol(argv[ArgCounter]);
      } else {
        fprintf(stderr, "%s: -idle option error.\n", ProgramName);
        exit(1);
      }
      break;
      case 0x310b6b4b93: /* -name */
      ArgCounter++;
      if (ArgCounter < argc) {
        WindowName = argv[ArgCounter];
      } else {
        fprintf(stderr, "%s: -name option error.\n", ProgramName);
        exit(1);
      }
      break;
      case 0x3777f58403a9c8d: /* -nocursor */
      NoCursor = True;
      break;
      case 0x3777f93f7912f0d: /* -towindow */
      ToWindow = True;
      ToFocus = False;
      break;
      case 0xd0a1a26950f6: /* -toname */
      ArgCounter++;
      if (ArgCounter < argc) {
        TargetName = argv[ArgCounter];
        ToWindow = True;
        ToFocus = False;
      } else {
        fprintf(stderr, "%s: -toname option error.\n", ProgramName);
        exit(1);
      }
      break;
      case 0x1ae4d5ef0a30f5: /* -tofocus */
      ToFocus = True;
      ToWindow = False;
      break;
      case 0xb87817a: /* -rv */
      ReverseVideo = True;
      break;
      case 0x3777f6c3d389887: /* -position */
      ArgCounter++;
      unsigned int width, height;
      XParseGeometry(argv[ArgCounter],&XOffset,&YOffset,&width,&height);
      // FIXME: unused
      break;
      case 0x6527821ce19: /* -debug */
      Synchronous = True;
      break;
      case 0xbf54f990118a263a: /* -patchlevel */
      fprintf(stderr,"Patchlevel :%s\n",PATCHLEVEL);
      break;
      case 0x59707a: /* -h */
      case 0x310b68123b: /* -help */
      Usage();
      exit(0);
      default:
      av = argv[ArgCounter] + 1;
      if (strcmp(av, "bsd") == 0)
        av = "bsd_daemon";
      for (loop=0;loop<BITMAPTYPES;loop++) {
        if (strcmp(av,AnimalDefaultsDataTable[loop].name)==0)
          {NekoMoyou = loop;found=1;}
      }
      if (!found) {
        fprintf(stderr,
                "%s: Unknown option \"%s\".\n", ProgramName,
                argv[ArgCounter]);
        Usage();
        exit(1);
      }
    }
  }

  if (strlen(theDisplayName) < 1) {
    theDisplayName = NULL;
  }
}


/*
 *      メイン関数
 */

int
main(int argc, char *argv[])
{
  char  theDisplayName[MAXDISPLAYNAME];

  ProgramName = argv[0];

  argc--;
  argv++;

  GetArguments(argc, argv, theDisplayName);

  XSetErrorHandler(NekoErrorHandler);

  InitScreen(theDisplayName);

  signal(SIGALRM, NullFunction);
  signal(SIGINT, RestoreCursor);
  signal(SIGTERM, RestoreCursor);
  signal(SIGQUIT, RestoreCursor);

  SinPiPer8Times3 = sin(PI_PER8 * (double)3);
  SinPiPer8 = sin(PI_PER8);

  ProcessNeko();

  RestoreCursor();
}
