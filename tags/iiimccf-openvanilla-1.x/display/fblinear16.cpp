#include <assert.h>
#include "fblinear16.h"
#include <iostream>
using namespace std;

__u32 FBLinear16::tab_cfb16[] = {
0x00000000, 0xffff0000, 0x0000ffff, 0xffffffff
};

FBLinear16::FBLinear16() {
    InitColorMap();
    mNextLine = mNextLine ? mNextLine : mXres<<1;
}

void FBLinear16::InitColorMap() {
    int i;
    __u32 red, green, blue;
    for(i = 0; i < 16; i++) {
        red = red16[i];
        green = green16[i];
        blue = blue16[i];

        cfb16[i] = ((red   & 0xf800) ) |
                   ((green & 0xfc00) >> 5) |
                   ((blue  & 0xf800) >> 11);
    }
}

void FBLinear16::FillRect(int x1,int y1,int x2,int y2,int color) {
    assert( x1 >= 0 && x1 < mXres && y1 >=0 && y1 < mYres);
    assert( x2 >= 0 && x2 < mXres && y2 >=0 && y2 < mYres);
    assert(x1 <= x2 && y1 <= y2);
    __u8* dest = (__u8*)mpBuf + mNextLine * y1 + x1 * 2;
    __u32* dest32;

    __u32 fgx = cfb16[color];
    fgx |= fgx<<16;

    int height = y2 - y1 + 1;
    int width = x2 - x1 + 1;
    int cnt;
    for(; height--; dest += mNextLine) {
        dest32 = (__u32*)dest;
        for (cnt = width/2; cnt--;) {
            fb_writel(fgx, dest32++);
        }
        if (width & 1)
           fb_writew(fgx, (__u16*)dest32);
    }
}

void FBLinear16::RevRect(int x1,int y1,int x2,int y2) {
    assert( x1 >= 0 && x1 < Width() && y1 >=0 && y1 < Height());
    assert( x2 >= 0 && x2 < Width() && y2 >=0 && y2 < Height());
    assert(x1 <= x2 && y1 <= y2);
    __u8* dest = (__u8*)mpBuf + mNextLine * y1 + x1 * 2;
    __u16* dest16;
    __u32* dest32;

    int height = y2 - y1 + 1;
    int width = x2 - x1 + 1;
    int cnt;
    for(; height--; dest += mNextLine) {
        dest32 = (__u32*)dest;
        for (cnt = width/2; cnt--;) {
            fb_writel(fb_readl(dest32) ^ 0xffffffff, dest32++);
        }
        if (width & 1) {
           dest16 = (__u16*)dest32;
           fb_writew(fb_readw(dest16) ^ 0xffff, dest16);
        }
    }
}

void FBLinear16::SaveRect(int x1,int y1,int x2,int y2, BitMap& pBuffer) {
    assert( x1 >= 0 && x1 < Width() && y1 >=0 && y1 < Height());
    assert( x2 >= 0 && x2 < Width() && y2 >=0 && y2 < Height());
    assert(x1 <= x2 && y1 <= y2);
  
    int height = y2 - y1 + 1;
    int width = x2 - x1 + 1;

    pBuffer.h = height;
    pBuffer.w = width;
    pBuffer.BufLen = height * width * 2;
    
    // allocate memory for saving
    pBuffer.pBuf = (char*) new char[pBuffer.BufLen];
    
    __u16* buf = (__u16*)pBuffer.pBuf;
    __u8* dest = (__u8*)mpBuf + mNextLine * y1 + x1 * 2;
    
    for(; height--; dest += mNextLine) {
        __u16* dest16 = (__u16*)dest;
        for ( int cnt = width ; cnt--;) {
            fb_writew( fb_readw(dest16++) , buf++ );
        }
    }
    
}

void FBLinear16::RstrRect(int x1,int y1,int x2,int y2, BitMap& pBuffer) {
    assert( x1 >= 0 && x1 < Width() && y1 >=0 && y1 < Height());
    assert( x2 >= 0 && x2 < Width() && y2 >=0 && y2 < Height());
    assert(x1 <= x2 && y1 <= y2);

    int height = y2 - y1 + 1;
    int width = x2 - x1 + 1;
    assert ( pBuffer.h == height &&  
             pBuffer.w == width  && 
             pBuffer.BufLen == height * width * 2 );
    
    __u16* buf = (__u16*)pBuffer.pBuf;
    __u8* dest = (__u8*)mpBuf + mNextLine * y1 + x1 * 2;

    for(; height--; dest += mNextLine) {
        __u16* dest16 = (__u16*)dest;
        for ( int cnt = width ; cnt--;) {
            fb_writew( fb_readw(buf++), dest16++ );
        }
    }
    
    // release memory 
    delete [] pBuffer.pBuf;
    pBuffer.pBuf = NULL;
}

inline void FBLinear16::PutPixel(int x,int y,int color) {
    assert( x >= 0 && x < mXres && y >=0 && y < mYres);
    fb_writew(cfb16[color], mpBuf + mNextLine * y + x * 2);
}

void FBLinear16::DrawChar(int x,int y,int fg,int bg,struct CharBitMap* pFont) {
    __u32 fgx,bgx,eorx;
    fgx = cfb16[fg];
    bgx = cfb16[bg];
    fgx |= (fgx << 16);
    bgx |= (bgx << 16);
    eorx = fgx ^ bgx;

    __u8* dest = ((__u8*)mpBuf + mNextLine * y + x * 2);
    __u32* dest32;

    char* cdat = pFont->pBuf;
    int row, cnt;
    for (row = mBlockHeight; row--; dest += mNextLine ) {
        dest32 = (__u32*)dest;
        for (cnt = (pFont->w)/8; cnt--;) {
            fb_writel((tab_cfb16[*cdat >> 6] & eorx) ^ bgx, dest32++);
            fb_writel((tab_cfb16[*cdat >> 4 & 3] & eorx) ^ bgx, dest32++);
            fb_writel((tab_cfb16[*cdat >> 2 & 3] & eorx) ^ bgx, dest32++);
            fb_writel((tab_cfb16[*cdat & 3] & eorx) ^ bgx, dest32++);
            cdat++;
        }
        if (pFont->isMulti8)
            continue;

        if (pFont->w & 4) {
            fb_writel((tab_cfb16[*cdat >> 6] & eorx) ^ bgx, dest32++);
            fb_writel((tab_cfb16[*cdat >> 4 & 3] & eorx) ^ bgx, dest32++);
        }
        if (pFont->w & 2) {
            fb_writel((tab_cfb16[*cdat >> 2 & 3] & eorx) ^ bgx, dest32++);
        }
        if (pFont->w & 1) {
            fb_writew((tab_cfb16[*cdat & 3] & eorx) ^ bgx, (__u16*)dest32);
        }
        cdat++;
    }
}

