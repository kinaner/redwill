/* ---------- windows ---------- */
#define NW 17          /* 9 built-in apps + 8 slots for windows created by .rpp/.rxe programs */
typedef struct { int x, y, w, h, rx, ry, rw, rh, open, min, max; } Win;
static Win W[NW] = {
    { 120, 50, 580, 400 }, { 620, 90, 230, 266 }, { 100, 40, 700, 480 }, { 120, 140, 520, 330 }, { 200, 70, 404, 332 },
    { 300, 150, 340, 250 }, { 330, 100, 336, 306 }, { 300, 120, 380, 290 }, { 90, 36, 700, 500 } };      /* entries 9.. are filled in by the API */
static const char *WN[NW] = { "Notepad", "Calculator", "My Documents", "Terminal", "Paint", "About RedWill", "Snake", "Settings", "RedWill Browser" };
static const u8 CANMAX[NW] = { 1, 0, 1, 1, 0, 0, 0, 0, 1 };
static int Z[NW] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
static int menu = 0, cal = 0, ctxo = 0, ctxx, ctxy, sel = -1, drag = -1, ddx, ddy, desk_sel = -1;
static u32 lastclick, lastc2;
static int lastwin = -1;
static int top_win(void) { for (int i = NW - 1; i >= 0; i--) { int w = Z[i]; if (W[w].open && !W[w].min) return w; } return -1; }
static void raise_win(int w) { int j = 0; for (int i = 0; i < NW; i++) if (Z[i] != w) Z[j++] = Z[i]; Z[NW - 1] = w; }
static void open_win(int w) { W[w].open = 1; W[w].min = 0; raise_win(w); dirty = 1; }

/* notepad */
static char nbuf[8200], nname[24]; static int nlen, nnode = -1, nup;
/* explorer */
static int esel = -1, eview = 0, ecur = 0, eord[96], en; static u32 ecl;
/* terminal */
static char tl[32][80]; static int tn; static Fld tin; static char tlast[80]; static u32 tfg = 0xD8DAFF;
/* calc */
static int cacc, ccur, cop, cfresh, cerr;
static const char CB[4][4] = { { '7', '8', '9', '/' }, { '4', '5', '6', '*' }, { '1', '2', '3', '-' }, { '0', 'C', '=', '+' } };
/* paint */
static int pcol = 0, pbr = 3, pdown = 0, plx, ply;
static const u32 PAL[10] = { 0x000000, 0xFFFFFF, 0xE02020, 0x20A030, 0x2050E0, 0xF0D020, 0xF08020, 0x9030C0, 0x805020, 0x909090 };
/* snake */
static int snx[300], sny[300], snl, sdx, sdy, sax, say, sscore, salive, sstart; static u32 snt;
/* browser */
static Fld faddr; static char hist[8][64]; static int hn, hp = -1, bscroll, bfocus, bmax;
static char pgbuf[4096];
static struct { int x, y, w, h; char t[48]; } LK[32]; static int nlk;
static u32 rng = 12345;
static u32 rnd(void) { rng = rng * 1664525u + 1013904223u; return rng >> 8; }

