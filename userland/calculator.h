/* ---------- calculator ---------- */
static void calc_key(char k) {
    if (k >= '0' && k <= '9') { if (cerr) { cerr = 0; ccur = 0; cacc = 0; cop = 0; } if (cfresh) { ccur = 0; cfresh = 0; } if (ccur < 100000000) ccur = ccur * 10 + (k - '0'); }
    else if (k == 'C') { ccur = cacc = cop = cfresh = cerr = 0; }
    else if (k == '=' || k == '+' || k == '-' || k == '*' || k == '/') {
        if (cerr) return;
        if (cop && !cfresh) { if (cop == '+') cacc += ccur; else if (cop == '-') cacc -= ccur; else if (cop == '*') cacc *= ccur; else if (cop == '/') { if (!ccur) { cerr = 1; return; } cacc /= ccur; } }
        else if (!cop) cacc = ccur;
        ccur = cacc; cop = (k == '=') ? 0 : k; cfresh = 1;
    }
}

