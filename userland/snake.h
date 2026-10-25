/* ---------- snake ---------- */
static void snake_reset(void) { snl = 3; for (int i = 0; i < 3; i++) { snx[i] = 8 - i; sny[i] = 7; } sdx = 1; sdy = 0; sax = 14; say = 7; sscore = 0; salive = 1; sstart = 0; }
static void snake_tick(void) {
    if (!salive || !sstart) return;
    int nx = snx[0] + sdx, ny = sny[0] + sdy;
    if (nx < 0 || ny < 0 || nx >= 20 || ny >= 15) { salive = 0; music(J_ERR, 3, 0); return; }
    for (int i = 0; i < snl; i++) if (snx[i] == nx && sny[i] == ny) { salive = 0; music(J_ERR, 3, 0); return; }
    int grow = (nx == sax && ny == say);
    if (grow && snl < 299) snl++;
    for (int i = snl - 1; i > 0; i--) { snx[i] = snx[i - 1]; sny[i] = sny[i - 1]; }
    snx[0] = nx; sny[0] = ny;
    if (grow) { sscore++; music(J_OK, 2, 0); for (int t = 0; t < 50; t++) { sax = (int)(rnd() % 20); say = (int)(rnd() % 15); int ok = 1; for (int i = 0; i < snl; i++) if (snx[i] == sax && sny[i] == say) ok = 0; if (ok) break; } }
}

