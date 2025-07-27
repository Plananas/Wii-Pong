#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ogcsys.h>
#include <gccore.h>
#include <wiiuse/wpad.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

#define PADDLE_WIDTH  10.0f
#define PADDLE_HEIGHT 40.0f
#define SCREEN_WIDTH  640.0f
#define SCREEN_HEIGHT 480.0f

void Initialise() {
    VIDEO_Init();
    WPAD_Init();

    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if (rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    GX_Init(NULL, 0);
    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopyYScale((f32)rmode->xfbHeight / (f32)rmode->efbHeight);
    GX_SetDispCopySrc(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_SetDispCopyDst(rmode->fbWidth, rmode->xfbHeight);
    GX_SetCopyFilter(rmode->aa, rmode->sample_pattern, GX_TRUE, rmode->vfilter);
    GX_SetFieldMode(rmode->field_rendering, ((rmode->viHeight == 2 * rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
    GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    GX_SetZCompLoc(GX_TRUE);
    GX_SetAlphaUpdate(GX_TRUE);
    GX_SetColorUpdate(GX_TRUE);

    // Setup vertex format
    GX_SetNumChans(1);
    GX_SetNumTexGens(0);
    GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GX_SetNumTevStages(1);

    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
}

void draw_paddle(float x, float y, float width, float height) {
    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);

    GX_Color1u32(0xFFFFFFFF);  // White

    GX_Position2f32(x, y);
    GX_Position2f32(x + width, y);
    GX_Position2f32(x + width, y + height);
    GX_Position2f32(x, y + height);

    GX_End();
}

void start_frame() {
    GX_SetViewport(0, 0, rmode->fbWidth, rmode->efbHeight, 0, 1);
    GX_SetScissor(0, 0, rmode->fbWidth, rmode->efbHeight);
    GX_InvalidateTexAll();
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_Begin(GX_QUADS, GX_VTXFMT0, 0); // placeholder, real drawing follows
}

void end_frame() {
    GX_DrawDone();
    GX_CopyDisp(xfb, GX_TRUE);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_Flush();
    VIDEO_WaitVSync();
}

int main() {
    Initialise();

    float paddle_left_y = 200;
    float paddle_right_y = 200;

    while (1) {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);
        u32 held = WPAD_ButtonsHeld(0);

        if (pressed & WPAD_BUTTON_HOME)
            break;

        if (held & WPAD_BUTTON_UP && paddle_left_y > 0)
            paddle_left_y -= 5;
        if (held & WPAD_BUTTON_DOWN && paddle_left_y < (SCREEN_HEIGHT - PADDLE_HEIGHT))
            paddle_left_y += 5;

        start_frame();

        draw_paddle(50, paddle_left_y, PADDLE_WIDTH, PADDLE_HEIGHT);                   // Left
        draw_paddle(SCREEN_WIDTH - 60, paddle_right_y, PADDLE_WIDTH, PADDLE_HEIGHT);   // Right

        end_frame();
    }

    return 0;
}
