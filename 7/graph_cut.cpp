/********************************************************************/
/***** CPU Graph Cut ************************************************/
/********************************************************************/
////////////////////////////////////////////////////
// Copyright (c) 2018 Kiyoshi Oguri    2018.02.14 //
// Released under the MIT license                 //
// http://opensource.org/licenses/mit-license.php //
////////////////////////////////////////////////////

extern int cost(int S, int H, int W);
extern void cut(int S, int H, int W);

#define LOOP (10)

int SIZE_S;
int SIZE_H;
int SIZE_W;
int SIZE_HW;
int SIZE_SHW;

struct shw {
  int S;
  int H;
  int W;
};

int *FLW;
int *OVF;
int *HGT;
int *TAG;
shw *QU1;
shw *QU2;

#define ADR1(S, H, W) \
 ((S)*SIZE_HW+\
  (H)*SIZE_W+\
  (W))

#define ADR2(S, H, W, D) \
 ((D)*SIZE_SHW+\
  (S)*SIZE_HW+\
  (H)*SIZE_W+\
  (W))

#define In_Mask 0x003ff000

#define In0_Set 0x00001000
#define In1_Set 0x00002000
#define In2_Set 0x00004000
#define In3_Set 0x00008000
#define In4_Set 0x00010000
#define In5_Set 0x00020000
#define In6_Set 0x00040000
#define In7_Set 0x00080000
#define In8_Set 0x00100000
#define In9_Set 0x00200000

#define In0_Rst ~In0_Set
#define In1_Rst ~In1_Set
#define In2_Rst ~In2_Set
#define In3_Rst ~In3_Set
#define In4_Rst ~In4_Set
#define In5_Rst ~In5_Set
#define In6_Rst ~In6_Set
#define In7_Rst ~In7_Set
#define In8_Rst ~In8_Set
#define In9_Rst ~In9_Set

#define Out_Mask 0x000003ff

#define Out0_Set 0x00000001
#define Out1_Set 0x00000002
#define Out2_Set 0x00000004
#define Out3_Set 0x00000008
#define Out4_Set 0x00000010
#define Out5_Set 0x00000020
#define Out6_Set 0x00000040
#define Out7_Set 0x00000080
#define Out8_Set 0x00000100
#define Out9_Set 0x00000200

#define Out0_Rst ~Out0_Set
#define Out1_Rst ~Out1_Set
#define Out2_Rst ~Out2_Set
#define Out3_Rst ~Out3_Set
#define Out4_Rst ~Out4_Set
#define Out5_Rst ~Out5_Set
#define Out6_Rst ~Out6_Set
#define Out7_Rst ~Out7_Set
#define Out8_Rst ~Out8_Set
#define Out9_Rst ~Out9_Set

inline int edg_read(int S, int H, int W, int D) {
  return FLW[ADR2(S, H, W, D)];
}

inline void edg_add(int S, int H, int W, int D, int V) {
  FLW[ADR2(S, H, W, D)] += V;
}

inline int ovf_read(int S, int H, int W) {
  return OVF[ADR1(S, H, W)];
}

inline void ovf_add(int S, int H, int W, int V) {
  OVF[ADR1(S, H, W)] += V;
}

inline int hgt_read(int S, int H, int W) {
  return HGT[ADR1(S, H, W)];
}

inline void hgt_write(int S, int H, int W, int V) {
  HGT[ADR1(S, H, W)] = V;
}

inline int tag_read(int S, int H, int W) {
  return TAG[ADR1(S, H, W)];
}

inline void tag_set(int S, int H, int W, int M) {
  TAG[ADR1(S, H, W)] |= M;
}

inline void tag_rst(int S, int H, int W, int M) {
  TAG[ADR1(S, H, W)] &= M;
}

void out_chk(int S, int H, int W, int D, int SM, int RM) {
  if (edg_read(S, H, W, D) > 0) tag_set(S, H, W, SM);
  else                          tag_rst(S, H, W, RM);
}

void in_chk(int S, int H, int W, int s, int h, int w, int R, int SM, int RM) {
  ///////////////////////////////
  if ((S ==        0) && (s == -1)) { tag_rst(S, H, W, RM); return; }
  if ((H ==        0) && (h == -1)) { tag_rst(S, H, W, RM); return; }
  if ((W ==        0) && (w == -1)) { tag_rst(S, H, W, RM); return; }
  if ((S == SIZE_S-1) && (s ==  1)) { tag_rst(S, H, W, RM); return; }
  if ((H == SIZE_H-1) && (h ==  1)) { tag_rst(S, H, W, RM); return; }
  if ((W == SIZE_W-1) && (w ==  1)) { tag_rst(S, H, W, RM); return; }
  ///////////////////////////////
  int nS = S+s;
  int nH = H+h;
  int nW = W+w;
  ///////////////////////////////
  if (edg_read(nS, nH, nW, R) > 0) tag_set(S, H, W, SM);
  else                             tag_rst(S, H, W, RM);
}

void tag_init(int S, int H, int W) {
  out_chk(S, H, W, 0, Out0_Set, Out0_Rst);
  out_chk(S, H, W, 1, Out1_Set, Out1_Rst);
  out_chk(S, H, W, 2, Out2_Set, Out2_Rst);
  out_chk(S, H, W, 3, Out3_Set, Out3_Rst);
  out_chk(S, H, W, 4, Out4_Set, Out4_Rst);
  out_chk(S, H, W, 5, Out5_Set, Out5_Rst);
  out_chk(S, H, W, 6, Out6_Set, Out6_Rst);
  out_chk(S, H, W, 7, Out7_Set, Out7_Rst);
  out_chk(S, H, W, 8, Out8_Set, Out8_Rst);
  out_chk(S, H, W, 9, Out9_Set, Out9_Rst);
  in_chk(S, H, W, 1, 0, 0, 9, In0_Set, In0_Rst);
  in_chk(S, H, W, 0,-1, 0, 2, In1_Set, In1_Rst);
  in_chk(S, H, W, 0, 1, 0, 1, In2_Set, In2_Rst);
  in_chk(S, H, W, 0, 0,-1, 4, In3_Set, In3_Rst);
  in_chk(S, H, W, 0, 0, 1, 3, In4_Set, In4_Rst);
  in_chk(S, H, W, 1, 0, 1, 7, In5_Set, In5_Rst);
  in_chk(S, H, W, 1, 0,-1, 8, In6_Set, In6_Rst);
  in_chk(S, H, W,-1, 0,-1, 5, In7_Set, In7_Rst);
  in_chk(S, H, W,-1, 0, 1, 6, In8_Set, In8_Rst);
  in_chk(S, H, W,-1, 0, 0, 0, In9_Set, In9_Rst);
}

void bfs_init(int S, int H, int W, int &flg, shw QUE[]) {
  if (ovf_read(S, H, W) < 0) {
    hgt_write(S, H, W, 0);
    QUE[flg].S = S;
    QUE[flg].H = H;
    QUE[flg].W = W;
    flg++;
  }
  else hgt_write(S, H, W, SIZE_SHW);
}

void bfs1(int S, int H, int W, int s, int h, int w, int next, int &flg, shw QUE[]) {
  ///////////////////////////////
  int nS = S+s;
  int nH = H+h;
  int nW = W+w;
  ///////////////////////////////
  if (hgt_read(nS, nH, nW) != SIZE_SHW) return;
  hgt_write(nS, nH, nW, next);
  QUE[flg].S = nS;
  QUE[flg].H = nH;
  QUE[flg].W = nW;
  flg++;
}

void bfs_do(int p, shw DO[], int next, int &flg, shw QUE[]) {
  ///////////////////////////////
  int S = DO[p].S;
  int H = DO[p].H;
  int W = DO[p].W;
  ///////////////////////////////
  int tag = tag_read(S, H, W);
  if ((tag & In_Mask) == 0) return;
  if (tag & In9_Set) bfs1(S, H, W,-1, 0, 0, next, flg, QUE);
  if (tag & In8_Set) bfs1(S, H, W,-1, 0, 1, next, flg, QUE);
  if (tag & In7_Set) bfs1(S, H, W,-1, 0,-1, next, flg, QUE);
  if (tag & In2_Set) bfs1(S, H, W, 0, 1, 0, next, flg, QUE);
  if (tag & In1_Set) bfs1(S, H, W, 0,-1, 0, next, flg, QUE);
  if (tag & In4_Set) bfs1(S, H, W, 0, 0, 1, next, flg, QUE);
  if (tag & In3_Set) bfs1(S, H, W, 0, 0,-1, next, flg, QUE);
  if (tag & In5_Set) bfs1(S, H, W, 1, 0, 1, next, flg, QUE);
  if (tag & In6_Set) bfs1(S, H, W, 1, 0,-1, next, flg, QUE);
  if (tag & In0_Set) bfs1(S, H, W, 1, 0, 0, next, flg, QUE);
}

void bfs(void) {
  int flg = 0;
  for (int S = 0; S < SIZE_S; S++) {
    for (int H = 0; H < SIZE_H; H++) {
      for (int W = 0; W < SIZE_W; W++) {
        bfs_init(S, H, W, flg, QU1);
      }
    }
  }
  for (int next = 1; ; next++) {
    int do_num = flg;
    if (do_num == 0) break;
    flg = 0;
    if ((next%2) == 1) for (int p = 0; p < do_num; p++) bfs_do(p, QU1, next, flg, QU2);
    else               for (int p = 0; p < do_num; p++) bfs_do(p, QU2, next, flg, QU1);
  }
}

int ovf_do(int S, int H, int W) {
  if (hgt_read(S, H, W) == SIZE_SHW) return 0;
  if (ovf_read(S, H, W) <= 0) return 0;
  return 1;
}

int ovf(void) {
  int flg;
  for (int S = 0; S < SIZE_S; S++) {
    for (int H = 0; H < SIZE_H; H++) {
      for (int W = 0; W < SIZE_W; W++) {
        flg = ovf_do(S, H, W);
        if (flg) break;
      }
      if (flg) break;
    }
    if (flg) break;
  }
  return flg;
}

void push1(int S, int H, int W, int s, int h, int w, int D, int R, int OSM, int ORM, int ISM, int IRM, int hh, int &oo) {
  ///////////////////////////////
  int nS = S+s;
  int nH = H+h;
  int nW = W+w;
  ///////////////////////////////
  if (hgt_read(nS, nH, nW) >= hh) return;
  int mm = edg_read(S, H, W, D);
  bool qq = oo >= mm;
  int pp = qq? mm: oo;
  ovf_add(nS, nH, nW, pp);
  ovf_add(S, H, W, -pp);
  edg_add(nS, nH, nW, R, pp);
  edg_add(S, H, W, D, -pp);
  oo -= pp;
  if (qq) {
    tag_rst(S, H, W, ORM);
    tag_rst(nS, nH, nW, IRM);
  }
  tag_set(nS, nH, nW, OSM);
  tag_set(S, H, W, ISM);
}

void push(int S, int H, int W) {
  int hh = hgt_read(S, H, W);
  if (hh == SIZE_SHW) return;
  int oo = ovf_read(S, H, W);
  if (oo <= 0) return;
  int tag = tag_read(S, H, W);
  if ((tag & Out_Mask) == 0) return;
  if (tag & Out0_Set) { push1(S, H, W, 1, 0, 0, 0, 9, Out9_Set, Out0_Rst, In0_Set, In9_Rst, hh, oo); if (oo <= 0) return; }
  if (tag & Out5_Set) { push1(S, H, W, 1, 0, 1, 5, 7, Out7_Set, Out5_Rst, In5_Set, In7_Rst, hh, oo); if (oo <= 0) return; }
  if (tag & Out6_Set) { push1(S, H, W, 1, 0,-1, 6, 8, Out8_Set, Out6_Rst, In6_Set, In8_Rst, hh, oo); if (oo <= 0) return; }
  if (tag & Out2_Set) { push1(S, H, W, 0, 1, 0, 2, 1, Out1_Set, Out2_Rst, In2_Set, In1_Rst, hh, oo); if (oo <= 0) return; }
  if (tag & Out1_Set) { push1(S, H, W, 0,-1, 0, 1, 2, Out2_Set, Out1_Rst, In1_Set, In2_Rst, hh, oo); if (oo <= 0) return; }
  if (tag & Out4_Set) { push1(S, H, W, 0, 0, 1, 4, 3, Out3_Set, Out4_Rst, In4_Set, In3_Rst, hh, oo); if (oo <= 0) return; }
  if (tag & Out3_Set) { push1(S, H, W, 0, 0,-1, 3, 4, Out4_Set, Out3_Rst, In3_Set, In4_Rst, hh, oo); if (oo <= 0) return; }
  if (tag & Out8_Set) { push1(S, H, W,-1, 0, 1, 8, 6, Out6_Set, Out8_Rst, In8_Set, In6_Rst, hh, oo); if (oo <= 0) return; }
  if (tag & Out7_Set) { push1(S, H, W,-1, 0,-1, 7, 5, Out5_Set, Out7_Rst, In7_Set, In5_Rst, hh, oo); if (oo <= 0) return; }
  if (tag & Out9_Set) { push1(S, H, W,-1, 0, 0, 9, 0, Out0_Set, Out9_Rst, In9_Set, In0_Rst, hh, oo); if (oo <= 0) return; }
  hgt_write(S, H, W, hh + 1);
}

void push_relabel(int loop) {
  for (int S = 0; S < SIZE_S; S++) {
    for (int H = 0; H < SIZE_H; H++) {
      for (int W = 0; W < SIZE_W; W++) {
        tag_init(S, H, W);
      }
    }
  }
  for ( ; ; ) {
    bfs();
    if (ovf() == 0) return;
    for (int i = 0; i < loop; i++) {
      for (int S = 0; S < SIZE_S; S++) {
        for (int H = 0; H < SIZE_H; H++) {
          for (int W = 0; W < SIZE_W; W++) {
            push(S, H, W);
          }
        }
      }
    }
  }
}

void data_set(int penalty_w, int penalty_h, int inhibit_a, int inhibit_b) {
  for (int H = 0; H < SIZE_H; H++) {
    for (int W = 0; W < SIZE_W; W++) {
      for (int S = 0; S < SIZE_S; S++) {
        ///////////////////////////////
        for (int i = 0; i < 10; i++) FLW[ADR2(S, H, W, i)] = 0;
        OVF[ADR1(S, H, W)] = 0;
        ///////////////////////////////
        if  (S!=SIZE_S-1)          FLW[ADR2(S, H, W, 0)] = cost(S+1, H, W);
        if  (S==SIZE_S-1)          OVF[ADR1(S, H, W)]   -= cost(S+1, H, W);
        if  (S==0)                 OVF[ADR1(S, H, W)]   += cost(S,   H, W);
        if  (S!=0)                 FLW[ADR2(S, H, W, 9)] = inhibit_a;
        if          (H!=0)         FLW[ADR2(S, H, W, 1)] = penalty_h;
        if          (H!=SIZE_H-1)  FLW[ADR2(S, H, W, 2)] = penalty_h;
        if          (W!=0)         FLW[ADR2(S, H, W, 3)] = penalty_w;
        if          (W!=SIZE_W-1)  FLW[ADR2(S, H, W, 4)] = penalty_w;
        if ((S!=0)&&(W!=0))        FLW[ADR2(S, H, W, 7)] = inhibit_b;
        if ((S!=0)&&(W!=SIZE_W-1)) FLW[ADR2(S, H, W, 8)] = inhibit_b;
      }
    }
  }
}

void dep_set(void) {
  for (int H = 0; H < SIZE_H; H++) {
    for (int W = 0; W < SIZE_W; W++) {
      for (int S = SIZE_S; S >= 0; S--) {
        if (S == SIZE_S) {
          if (HGT[ADR1(S-1, H, W)] == SIZE_SHW) {
            cut(S, H, W);
            break;
          }
        }
        else if (S == 0) {
          if (HGT[ADR1(S, H, W)] != SIZE_SHW) {
            cut(S, H, W);
            break;
          }
        }
        else {
          if ((HGT[ADR1(S, H, W)] != SIZE_SHW) && (HGT[ADR1(S-1, H, W)] == SIZE_SHW)) {
            cut(S, H, W);
            break;
          }
        }
      }
    }
  }
}

void graph_cut(int penalty_w, int penalty_h, int inhibit_a, int inhibit_b) {
  FLW = new int[SIZE_SHW*10];
  OVF = new int[SIZE_SHW];
  HGT = new int[SIZE_SHW];
  TAG = new int[SIZE_SHW];
  QU1 = new shw[SIZE_SHW];
  QU2 = new shw[SIZE_SHW];
  data_set(penalty_w, penalty_h, inhibit_a, inhibit_b);
  push_relabel(LOOP);
  dep_set();
  delete [] QU2;
  delete [] QU1;
  delete [] TAG;
  delete [] HGT;
  delete [] OVF;
  delete [] FLW;
}
