#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;
static u8 *fifo = NULL;

#define FIFO_SIZE (256 * 1024)

void initialize_graphics() {
    // Initialize video and input
    VIDEO_Init();
    WPAD_Init();
    
    // Get video mode and allocate framebuffer
    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    
    // Configure video
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    
    if (rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();
    
    // Allocate and initialize GX
    fifo = memalign(32, FIFO_SIZE);
    GX_Init(fifo, FIFO_SIZE);
    
    // Setup GX rendering
    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopyYScale((f32)rmode->xfbHeight / (f32)rmode->efbHeight);
    GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth, rmode->xfbHeight);
    GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
    GX_SetFieldMode(rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
    GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    GX_SetColorUpdate(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);
    
    // Setup vertex format for drawing
    GX_SetNumChans(1);
    GX_SetNumTexGens(0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetNumTevStages(1);
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    
    // Setup matrices for 2D rendering
    Mtx44 perspective;
    guOrtho(perspective, 0, 479, 0, 639, 0, 300);
    GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);
}

void draw_square(float x, float y, float size) {
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    
    GX_Position2f32(x, y);
    GX_Color4u8(255, 0, 0, 255);  // Red
    
    GX_Position2f32(x + size, y);
    GX_Color4u8(255, 0, 0, 255);
    
    GX_Position2f32(x + size, y + size);
    GX_Color4u8(255, 0, 0, 255);
    
    GX_Position2f32(x, y + size);
    GX_Color4u8(255, 0, 0, 255);
    
    GX_End();
}

int main() {
    initialize_graphics();
    
    // Main loop
    while (1) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);
        
        if (pressed & WPAD_BUTTON_HOME)
            break;
        
        // Clear screen to blue
        GX_SetCopyClear((GXColor){20, 20, 30, 255}, GX_MAX_Z24);
        
        // Setup for drawing
        GX_InvalidateTexAll();
        GX_ClearVtxDesc();
        GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
        GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
        GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        
        // Set matrices
        Mtx modelview;
        guMtxIdentity(modelview);
        GX_LoadPosMtxImm(modelview, GX_PNMTX0);
        
        // Draw red square in center

        draw_square(295, 215, 50);
        
        // Finish frame
        GX_DrawDone();
        GX_CopyDisp(xfb, GX_TRUE);
        VIDEO_SetNextFramebuffer(xfb);
        VIDEO_Flush();
        VIDEO_WaitVSync();
    }
    
    return 0;
}