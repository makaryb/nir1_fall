/********************************************************************/
/***** GPU Graph Cut ************************************************/
/********************************************************************/
////////////////////////////////////////////////////
// Copyright (c) 2018 Kiyoshi Oguri    2018.02.14 //
// Released under the MIT license                 //
// http://opensource.org/licenses/mit-license.php //
////////////////////////////////////////////////////

extern int cost(int S, int H, int W);
extern void cut(int S, int H, int W);

#define TD_NUM (256)
#define LOOP (37)

int SIZE_S;
int SIZE_H;
int SIZE_W;
int SIZE_HW;
int SIZE_SHW;

__constant__ int size_s;
__constant__ int size_h;
__constant__ int size_w;
__constant__ int size_hw;
__constant__ int size_shw;

int *h_FLG;
int *h_FLW;
int *h_OVF;
int *h_HGT;

int *d_FLG;
int *d_FLW;
int *d_OVF;
int *d_HGT;
int *d_TAG;

__constant__ int *FLW;
__constant__ int *OVF;
__constant__ int *HGT;
__constant__ int *TAG;

size_t I_SIZE;
size_t FLW_SIZE;
size_t OVF_SIZE;
size_t HGT_SIZE;
size_t TAG_SIZE;

#define h_ADR1(S, H, W) \
 ((S)*SIZE_HW+\
  (H)*SIZE_W+\
  (W))

#define h_ADR2(S, H, W, D) \
 ((D)*SIZE_SHW+\
  (S)*SIZE_HW+\
  (H)*SIZE_W+\
  (W))

#define ADR1(S, H, W) \
 ((S)*size_hw+\
  (H)*size_w+\
  (W))

#define ADR2(S, H, W, D) \
 ((D)*size_shw+\
  (S)*size_hw+\
  (H)*size_w+\
  (W))

#define SADR1(S, H, W) \
 ((S)*SIZE_BHW+\
  (H)*SIZE_BW+\
  (W))

#define DIV(A,B) (((A)%(B)==0)? ((A)/(B)) : (((A)/(B))+1))
#define SIZE_BS (8)
#define SIZE_BH (8)
#define SIZE_BW (8)
#define SIZE_BSHW (SIZE_BS*SIZE_BH*SIZE_BW)
#define SIZE_BHW (SIZE_BH*SIZE_BW)
#define SIZE_GS DIV(SIZE_S,SIZE_BS)
#define SIZE_GH DIV(SIZE_H,SIZE_BH)
#define SIZE_GW DIV(SIZE_W,SIZE_BW)

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

inline __device__ int edg_read(int S, int H, int W, int D) {
  return FLW[ADR2(S, H, W, D)];
}

inline __device__ void edg_add(int S, int H, int W, int D, int V) {
  FLW[ADR2(S, H, W, D)] += V;
}

inline __device__ int ovf_read(int S, int H, int W) {
  return OVF[ADR1(S, H, W)];
}

inline __device__ void ovf_add(int S, int H, int W, int V) {
  atomicAdd(&(OVF[ADR1(S, H, W)]), V);
}

inline __device__ int hgt_read(int S, int H, int W) {
  return HGT[ADR1(S, H, W)];
}

inline __device__ void hgt_write(int S, int H, int W, int V) {
  HGT[ADR1(S, H, W)] = V;
}

inline __device__ int tag_read(int S, int H, int W) {
  return TAG[ADR1(S, H, W)];
}

inline __device__ void tag_set(int S, int H, int W, int M) {
  atomicOr(&(TAG[ADR1(S, H, W)]), M);
}

inline __device__ void tag_rst(int S, int H, int W, int M) {
  atomicAnd(&(TAG[ADR1(S, H, W)]), M);
}

inline __device__ int shed_read(int SM[], int S, int H, int W) {
  return SM[SADR1(S, H, W)];
}

inline __device__ void shed_write(int SM[], int S, int H, int W, int V) {
  SM[SADR1(S, H, W)] = V;
}

__global__ void reset(int FLG[]) {
  FLG[0] = 0;
}

__device__ void out_chk(int S, int H, int W, int D, int SM) {
  if (edg_read(S, H, W, D) > 0) tag_set(S, H, W, SM);
}

__global__ void tag_init(void) {
  ///////////////////////////////
  int total_id = blockDim.x * blockIdx.x + threadIdx.x;
  if (total_id >= size_shw) return;
  int S  = total_id / size_hw;
  int sa = total_id % size_hw;
  int H  = sa       / size_w;
  int W  = sa       % size_w;
  ///////////////////////////////
  tag_rst(S, H, W, ~Out_Mask);
  out_chk(S, H, W, 0, Out0_Set);
  out_chk(S, H, W, 1, Out1_Set);
  out_chk(S, H, W, 2, Out2_Set);
  out_chk(S, H, W, 3, Out3_Set);
  out_chk(S, H, W, 4, Out4_Set);
  out_chk(S, H, W, 5, Out5_Set);
  out_chk(S, H, W, 6, Out6_Set);
  out_chk(S, H, W, 7, Out7_Set);
  out_chk(S, H, W, 8, Out8_Set);
  out_chk(S, H, W, 9, Out9_Set);
}

__global__ void bfs_init(void) {
  ///////////////////////////////
  int total_id = blockDim.x * blockIdx.x + threadIdx.x;
  if (total_id >= size_shw) return;
  int S  = total_id / size_hw;
  int sa = total_id % size_hw;
  int H  = sa       / size_w;
  int W  = sa       % size_w;
  ///////////////////////////////
  if (ovf_read(S, H, W) < 0) hgt_write(S, H, W, 0);
  else                       hgt_write(S, H, W, size_shw);
}

__device__ void bfs_front_i(int SM[], int S, int H, int W, int s, int h, int w, int &nt) {
  ///////////////////////////////
  int nS = S + s;
  int nH = H + h;
  int nW = W + w;
  ///////////////////////////////
  int T = shed_read(SM, nS, nH, nW) + 1;
  if (nt > T) nt = T;
}

__global__ void bfs_do_i(void) {
  __shared__ int FLG;
  __shared__ int SM[SIZE_BSHW];
  ///////////////////////////////
  int bs = blockIdx.z;
  int bh = blockIdx.y;
  int bw = blockIdx.x;
  //---------------------------//
  int s = threadIdx.z;
  int h = threadIdx.y;
  int w = threadIdx.x;
  //---------------------------//
  int S = SIZE_BS * bs + s;
  if (S >= size_s) return;
  int H = SIZE_BH * bh + h;
  if (H >= size_h) return;
  int W = SIZE_BW * bw + w;
  if (W >= size_w) return;
  ///////////////////////////////
  bool nzz = (s != 0);
  bool nzy = (h != 0);
  bool nzx = (w != 0);
  bool niz = ((s != (SIZE_BS-1)) && (S != (size_s-1)));
  bool niy = ((h != (SIZE_BH-1)) && (H != (size_h-1)));
  bool nix = ((w != (SIZE_BW-1)) && (W != (size_w-1)));
  ///////////////////////////////
  int tag = tag_read(S, H, W);
  shed_write(SM, s, h, w, hgt_read(S, H, W));
  for ( ; ; ) {
    __syncthreads();
    if ((s == 0) && (h == 0) && (w == 0)) FLG = 0;
    __syncthreads();
    if (tag & Out_Mask) {
      int ct = shed_read(SM, s, h, w);
      int nt = ct;
      if (niz       ) if (tag & Out0_Set) bfs_front_i(SM, s, h, w, 1, 0, 0, nt);
      if (niz && nix) if (tag & Out5_Set) bfs_front_i(SM, s, h, w, 1, 0, 1, nt);
      if (niz && nzx) if (tag & Out6_Set) bfs_front_i(SM, s, h, w, 1, 0,-1, nt);
      if (       niy) if (tag & Out2_Set) bfs_front_i(SM, s, h, w, 0, 1, 0, nt);
      if (       nzy) if (tag & Out1_Set) bfs_front_i(SM, s, h, w, 0,-1, 0, nt);
      if (       nix) if (tag & Out4_Set) bfs_front_i(SM, s, h, w, 0, 0, 1, nt);
      if (       nzx) if (tag & Out3_Set) bfs_front_i(SM, s, h, w, 0, 0,-1, nt);
      if (nzz && nix) if (tag & Out8_Set) bfs_front_i(SM, s, h, w,-1, 0, 1, nt);
      if (nzz && nzx) if (tag & Out7_Set) bfs_front_i(SM, s, h, w,-1, 0,-1, nt);
      if (nzz       ) if (tag & Out9_Set) bfs_front_i(SM, s, h, w,-1, 0, 0, nt);
      if (nt != ct) {
        shed_write(SM, s, h, w, nt);
        if (FLG == 0) FLG = 1;
      }
    }
    __syncthreads();
    if (FLG == 0) break;
  }
  hgt_write(S, H, W, shed_read(SM, s, h, w));
}

__device__ void bfs_front_o(int S, int H, int W, int s, int h, int w, int &nt) {
  ///////////////////////////////
  int nS = S + s;
  int nH = H + h;
  int nW = W + w;
  ///////////////////////////////
  int T = hgt_read(nS, nH, nW) + 1;
  if (nt > T) nt = T;
}

__global__ void bfs_do_o(int FLG[]) {
  ///////////////////////////////
  int bs = blockIdx.z;
  int bh = blockIdx.y;
  int bw = blockIdx.x;
  //---------------------------//
  int s = threadIdx.z;
  int h = threadIdx.y;
  int w = threadIdx.x;
  //---------------------------//
  int S = SIZE_BS * bs + s;
  if (S >= size_s) return;
  int H = SIZE_BH * bh + h;
  if (H >= size_h) return;
  int W = SIZE_BW * bw + w;
  if (W >= size_w) return;
  ///////////////////////////////
  bool pzz = ((s == 0) && (S != 0));
  bool pzy = ((h == 0) && (H != 0));
  bool pzx = ((w == 0) && (W != 0));
  bool piz = ((s == (SIZE_BS-1)) && (S != (size_s-1)));
  bool piy = ((h == (SIZE_BH-1)) && (H != (size_h-1)));
  bool pix = ((w == (SIZE_BW-1)) && (W != (size_w-1)));
  ///////////////////////////////
  int tag = tag_read(S, H, W);
  if (tag & Out_Mask) {
    int ct = hgt_read(S, H, W);
    int nt = ct;
    if (piz       ) if (tag & Out0_Set) bfs_front_o(S, H, W, 1, 0, 0, nt);
    if (piz || pix) if (tag & Out5_Set) bfs_front_o(S, H, W, 1, 0, 1, nt);
    if (piz || pzx) if (tag & Out6_Set) bfs_front_o(S, H, W, 1, 0,-1, nt);
    if (       piy) if (tag & Out2_Set) bfs_front_o(S, H, W, 0, 1, 0, nt);
    if (       pzy) if (tag & Out1_Set) bfs_front_o(S, H, W, 0,-1, 0, nt);
    if (       pix) if (tag & Out4_Set) bfs_front_o(S, H, W, 0, 0, 1, nt);
    if (       pzx) if (tag & Out3_Set) bfs_front_o(S, H, W, 0, 0,-1, nt);
    if (pzz || pix) if (tag & Out8_Set) bfs_front_o(S, H, W,-1, 0, 1, nt);
    if (pzz || pzx) if (tag & Out7_Set) bfs_front_o(S, H, W,-1, 0,-1, nt);
    if (pzz       ) if (tag & Out9_Set) bfs_front_o(S, H, W,-1, 0, 0, nt);
    if (nt != ct) {
      hgt_write(S, H, W, nt);
      if (FLG[0] == 0) FLG[0] = 1;
    }
  }
}

__global__ void ovf_do(int FLG[]) {
  if (FLG[0] != 0) return;
  ///////////////////////////////
  int total_id = blockDim.x * blockIdx.x + threadIdx.x;
  if (total_id >= size_shw) return;
  int S  = total_id / size_hw;
  int sa = total_id % size_hw;
  int H  = sa       / size_w;
  int W  = sa       % size_w;
  ///////////////////////////////
  if (hgt_read(S, H, W) == size_shw) return;
  if (ovf_read(S, H, W) <= 0) return;
  FLG[0] = 1;
}

__device__ void push1(int S, int H, int W, int s, int h, int w, int D, int R, int SM, int RM, int hh, int &oo) {
  ///////////////////////////////
  int nS = S + s;
  int nH = H + h;
  int nW = W + w;
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
  if (qq) tag_rst(S, H, W, RM);
  tag_set(nS, nH, nW, SM);
}

__global__ void push(void) {
  ///////////////////////////////
  int total_id = blockDim.x * blockIdx.x + threadIdx.x;
  if (total_id >= size_shw) return;
  int S  = total_id / size_hw;
  int sa = total_id % size_hw;
  int H  = sa       / size_w;
  int W  = sa       % size_w;
  ///////////////////////////////
  int hh = hgt_read(S, H, W);
  if (hh == size_shw) return;
  int oo = ovf_read(S, H, W);
  if (oo <= 0) return;
  int tag = tag_read(S, H, W);
  if ((tag & Out_Mask) == 0) return;
  if (tag & Out0_Set) push1(S, H, W, 1, 0, 0, 0, 9, Out9_Set, Out0_Rst, hh, oo); if (oo <= 0) return;
  if (tag & Out5_Set) push1(S, H, W, 1, 0, 1, 5, 7, Out7_Set, Out5_Rst, hh, oo); if (oo <= 0) return;
  if (tag & Out6_Set) push1(S, H, W, 1, 0,-1, 6, 8, Out8_Set, Out6_Rst, hh, oo); if (oo <= 0) return;
  if (tag & Out2_Set) push1(S, H, W, 0, 1, 0, 2, 1, Out1_Set, Out2_Rst, hh, oo); if (oo <= 0) return;
  if (tag & Out1_Set) push1(S, H, W, 0,-1, 0, 1, 2, Out2_Set, Out1_Rst, hh, oo); if (oo <= 0) return;
  if (tag & Out4_Set) push1(S, H, W, 0, 0, 1, 4, 3, Out3_Set, Out4_Rst, hh, oo); if (oo <= 0) return;
  if (tag & Out3_Set) push1(S, H, W, 0, 0,-1, 3, 4, Out4_Set, Out3_Rst, hh, oo); if (oo <= 0) return;
  if (tag & Out8_Set) push1(S, H, W,-1, 0, 1, 8, 6, Out6_Set, Out8_Rst, hh, oo); if (oo <= 0) return;
  if (tag & Out7_Set) push1(S, H, W,-1, 0,-1, 7, 5, Out5_Set, Out7_Rst, hh, oo); if (oo <= 0) return;
  if (tag & Out9_Set) push1(S, H, W,-1, 0, 0, 9, 0, Out0_Set, Out9_Rst, hh, oo); if (oo <= 0) return;
  hgt_write(S, H, W, hh + 1);
}

void push_relabel(int loop) {
  dim3 grid(SIZE_GW, SIZE_GH, SIZE_GS);
  dim3 block(SIZE_BW, SIZE_BH, SIZE_BS);
  tag_init<<< (SIZE_SHW/TD_NUM)+1, TD_NUM >>>();
  for ( ; ; ) {
    bfs_init<<< (SIZE_SHW/TD_NUM)+1, TD_NUM >>>();
    for ( ; ; ) {
      bfs_do_i<<< grid, block >>>();
      reset<<< 1, 1 >>>(d_FLG);
      bfs_do_o<<< grid, block >>>(d_FLG);
      cudaThreadSynchronize();
      cudaMemcpy(h_FLG, d_FLG, I_SIZE, cudaMemcpyDeviceToHost);
      if (h_FLG[0] == 0) break;
    }
    reset<<< 1, 1 >>>(d_FLG);
    ovf_do<<< (SIZE_SHW/TD_NUM)+1, TD_NUM >>>(d_FLG);
    cudaThreadSynchronize();
    cudaMemcpy(h_FLG, d_FLG, I_SIZE, cudaMemcpyDeviceToHost);
    if (h_FLG[0] == 0) break;
    for (int i = 0; i < loop; i++) {
      push<<< (SIZE_SHW/TD_NUM)+1, TD_NUM >>>();
    }
  }
}

void data_set(int penalty_w, int penalty_h, int inhibit_a, int inhibit_b) {
  for (int H = 0; H < SIZE_H; H++) {
    for (int W = 0; W < SIZE_W; W++) {
      for (int S = 0; S < SIZE_S; S++) {
        ///////////////////////////////
        for (int i = 0; i < 10; i++) h_FLW[h_ADR2(S, H, W, i)] = 0;
        h_OVF[h_ADR1(S, H, W)] = 0;
        ///////////////////////////////
        if  (S!=SIZE_S-1)          h_FLW[h_ADR2(S, H, W, 0)] = cost(S+1, H, W);
        if  (S==SIZE_S-1)          h_OVF[h_ADR1(S, H, W)]   -= cost(S+1, H, W);
        if  (S==0)                 h_OVF[h_ADR1(S, H, W)]   += cost(S,   H, W);
        if  (S!=0)                 h_FLW[h_ADR2(S, H, W, 9)] = inhibit_a;
        if          (H!=0)         h_FLW[h_ADR2(S, H, W, 1)] = penalty_h;
        if          (H!=SIZE_H-1)  h_FLW[h_ADR2(S, H, W, 2)] = penalty_h;
        if          (W!=0)         h_FLW[h_ADR2(S, H, W, 3)] = penalty_w;
        if          (W!=SIZE_W-1)  h_FLW[h_ADR2(S, H, W, 4)] = penalty_w;
        if ((S!=0)&&(W!=0))        h_FLW[h_ADR2(S, H, W, 7)] = inhibit_b;
        if ((S!=0)&&(W!=SIZE_W-1)) h_FLW[h_ADR2(S, H, W, 8)] = inhibit_b;
      }
    }
  }
  cudaMemcpy(d_FLW, h_FLW, FLW_SIZE, cudaMemcpyHostToDevice);
  cudaMemcpy(d_OVF, h_OVF, OVF_SIZE, cudaMemcpyHostToDevice);
}

int sink_chk(void) {
  cudaMemcpy(h_OVF, d_OVF, OVF_SIZE, cudaMemcpyDeviceToHost);
  int total = 0;
  for (int H = 0; H < SIZE_H; H++) {
    for (int W = 0; W < SIZE_W; W++) {
      int sink = h_OVF[h_ADR1(SIZE_S-1, H, W)];
      if (sink < 0) total += -sink;
    }
  }
  return total;
}

void dep_set(void) {
  cudaMemcpy(h_HGT, d_HGT, HGT_SIZE, cudaMemcpyDeviceToHost);
  for (int H = 0; H < SIZE_H; H++) {
    for (int W = 0; W < SIZE_W; W++) {
      for (int S = SIZE_S; S >= 0; S--) {
        if (S == SIZE_S) {
          if (h_HGT[h_ADR1(S-1, H, W)] == SIZE_SHW) {
            cut(S, H, W);
            break;
          }
        }
        else if (S == 0) {
          if (h_HGT[h_ADR1(S, H, W)] != SIZE_SHW) {
            cut(S, H, W);
            break;
          }
        }
        else {
          if ((h_HGT[h_ADR1(S, H, W)] != SIZE_SHW) && (h_HGT[h_ADR1(S-1, H, W)] == SIZE_SHW)) {
            cut(S, H, W);
            break;
          }
        }
      }
    }
  }
}

int graph_cut(int penalty_w, int penalty_h, int inhibit_a, int inhibit_b) {
  cudaMemcpyToSymbol(size_s,   &SIZE_S,   sizeof(int));
  cudaMemcpyToSymbol(size_h,   &SIZE_H,   sizeof(int));
  cudaMemcpyToSymbol(size_w,   &SIZE_W,   sizeof(int));
  cudaMemcpyToSymbol(size_hw,  &SIZE_HW,  sizeof(int));
  cudaMemcpyToSymbol(size_shw, &SIZE_SHW, sizeof(int));
  I_SIZE = sizeof(int);
  FLW_SIZE = sizeof(int) * SIZE_SHW*10;
  OVF_SIZE = sizeof(int) * SIZE_SHW;
  HGT_SIZE = sizeof(int) * SIZE_SHW;
  TAG_SIZE = sizeof(int) * SIZE_SHW;
  h_FLG = new int[1];
  h_FLW = new int[SIZE_SHW*10];
  h_OVF = new int[SIZE_SHW];
  h_HGT = new int[SIZE_SHW];
  cudaMalloc((void **)&d_FLG, I_SIZE);
  cudaMalloc((void **)&d_FLW, FLW_SIZE);
  cudaMalloc((void **)&d_OVF, OVF_SIZE);
  cudaMalloc((void **)&d_HGT, HGT_SIZE);
  cudaMalloc((void **)&d_TAG, TAG_SIZE);
  cudaMemcpyToSymbol(FLW, &d_FLW, sizeof(int*));
  cudaMemcpyToSymbol(OVF, &d_OVF, sizeof(int*));
  cudaMemcpyToSymbol(HGT, &d_HGT, sizeof(int*));
  cudaMemcpyToSymbol(TAG, &d_TAG, sizeof(int*));
  data_set(penalty_w, penalty_h, inhibit_a, inhibit_b);
  int before = sink_chk();
  push_relabel(LOOP);
  int after = sink_chk();
  dep_set();
  cudaFree(d_TAG);
  cudaFree(d_HGT);
  cudaFree(d_OVF);
  cudaFree(d_FLW);
  cudaFree(d_FLG);
  delete [] h_HGT;
  delete [] h_OVF;
  delete [] h_FLW;
  delete [] h_FLG;
  return before - after;
}
