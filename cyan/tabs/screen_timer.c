//screen_timer.c
#include "../clay_ui.h"
#include "../data/data.h"

static void render_spinbox(int number, char *textChars, size_t textCharsSize, bool active) {
    Clay_Color bgColor = active ? (Clay_Color){255, 255, 255, 255} : (Clay_Color){55, 55, 55, 255};
    Clay_Color textColor = active ? (Clay_Color){55, 55, 55, 255} : (Clay_Color){255, 255, 255, 255};
    snprintf(textChars, textCharsSize, "%d", number);
    Clay_String textStr = { .length = (int) strlen(textChars), .chars = textChars };

    CLAY_AUTO_ID({
        .layout = { .padding = { .bottom = 20 }, .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() } },
        .backgroundColor = active ? (Clay_Color){255, 255, 255, 255} : (Clay_Color){22, 22, 22, 255},
    }) {
        CLAY_AUTO_ID({
            .layout = {
                .padding = {24, 24, 18, 18},
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() }
            },
            .backgroundColor = bgColor
        }) {
            CLAY_TEXT(textStr, CLAY_TEXT_CONFIG({
                .fontId = FONT_LARGE,
                .fontSize = 40,
                .textColor = textColor
            }));
        }
    }
}

static void render_timer(CyberwatchData *data, int debugOpacity, Clay_String timerString, bool timerActive) {
    Clay_Sizing expand = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) };
    TimerData *t = &data->timer;

    CLAY(CLAY_ID("Main"), {
        .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM, .sizing = expand }
    }) {
        CLAY(CLAY_ID("TopMain"), {
            .backgroundColor = {20,46,65,debugOpacity},
            .layout = {
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_PERCENT(0.45f) }
            }
        }) {
            CLAY(CLAY_ID("SpinboxRow"), {
                .layout = {
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .childGap = 10,
                    .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                    .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() }
                }
            }) {
                CLAY_AUTO_ID({
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() },
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                    }
                }) {
                    CLAY_TEXT(CLAY_STRING("H"), CLAY_TEXT_CONFIG({
                        .fontId = FONT_LARGE,
                        .fontSize = 16,
                        .textColor = {255, 255, 255, 255}
                    }));
                    render_spinbox(t->hSpinbox, t->hSpinboxChars, sizeof(t->hSpinboxChars), data->timer.selectedElement==0);
                }
                CLAY_AUTO_ID({
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() },
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                    }
                }) {
                    CLAY_TEXT(CLAY_STRING("M"), CLAY_TEXT_CONFIG({
                        .fontId = FONT_LARGE,
                        .fontSize = 16,
                        .textColor = {255, 255, 255, 255}
                    }));
                    render_spinbox(t->mSpinbox, t->mSpinboxChars, sizeof(t->mSpinboxChars), data->timer.selectedElement==1);
                }
                CLAY_AUTO_ID({
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() },
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER }
                    }
                }) {
                    CLAY_TEXT(CLAY_STRING("S"), CLAY_TEXT_CONFIG({
                        .fontId = FONT_LARGE,
                        .fontSize = 16,
                        .textColor = {255, 255, 255, 255}
                    }));
                    render_spinbox(t->sSpinbox, t->sSpinboxChars, sizeof(t->sSpinboxChars), data->timer.selectedElement==2);
                }
            }
        }

        CLAY(CLAY_ID("BottomMain"), {
            .backgroundColor = {23,56,65, debugOpacity},
            .layout = {
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
                .sizing = expand
            }
        }){
            if (timerActive){
                CLAY_TEXT(timerString, CLAY_TEXT_CONFIG({
                    .fontId = FONT_LARGE,
                    .fontSize = 140,
                    .textColor = data->timer.active ? ACCENT_COLOUR : (Clay_Color){80,80,80,255},
                }));
            } else {
                CLAY_TEXT(timerString, CLAY_TEXT_CONFIG({
                    .fontId = FONT_LARGE,
                    .fontSize = 140,
                    .textColor = {80,80,80,255},
                }));
            }
        }
    }
}

Clay_RenderCommandArray clay_timer(CyberwatchData* data, int width, int height, bool show_debug) {
    float deltaTime = get_delta();
    int debugOpacity = show_debug ? 100 : 0;

    read_timer(data);
    Clay_String timerString = { .chars = data->timer.chars, .length = strlen(data->timer.chars), .isStaticallyAllocated = false };

    int headerHeight = (int) (height * 0.1f);
    int footerHeight = (int) (height * 0.1f);
    Clay_Sizing expand = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) };

    Clay_SetLayoutDimensions((Clay_Dimensions) { (float) width, (float) height });
    Clay_BeginLayout();

    CLAY(CLAY_ID("Display"), {
        .layout = {
            .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER },
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = expand
        }
    }) {
        render_header_bar(data, debugOpacity, width, headerHeight);
        render_timer(data, debugOpacity, timerString, data->timer.active);
        render_footer(data, debugOpacity, footerHeight);
    }

    return Clay_EndLayout(deltaTime);
}

void timer_cycle_element(CyberwatchData *data) {
    data->timer.selectedElement = (data->timer.selectedElement+1) % TIMER_SELECTABLE_ELEMENT_COUNT;
    // printf("data->timer.selectedElement = %i\n", data->timer.selectedElement);
}

void timer_init(CyberwatchData *data){
    data->timer.selectedElement = -1;
}

void timer_toggle(CyberwatchData *data){
    if (!data) return;
    if (data->timer.hSpinbox==0&&data->timer.mSpinbox==0&&data->timer.sSpinbox==0 && data->timer.h==0&&data->timer.m==0&&data->timer.s==0){return;}
    bool newState = !data->timer.active;
    // If activating and current timer values are 0, load from spinboxes
    if (newState) {
        TimerData *t = &data->timer;
        if (t->h == 0 && t->m == 0 && t->s == 0) {
            t->h = t->hSpinbox;
            t->m = t->mSpinbox;
            t->s = t->sSpinbox;
            snprintf(t->chars, sizeof(t->chars), "%02d:%02d:%02d", t->h, t->m, t->s);
        }

        time_t now = time(NULL);
        if (now != (time_t)-1) {
            struct tm *nowTm = localtime(&now);
            if (nowTm) {
                t->lastUpdated = *nowTm;
            }
        }
    }

    data->timer.active = newState;
    if (!data->timer.active) { data->timer.selectedElement=-1; }
}

void read_timer(CyberwatchData* data){
    static int lastAppliedHSpinbox = -1;
    static int lastAppliedMSpinbox = -1;
    static int lastAppliedSSpinbox = -1;
    static bool hasAppliedSpinbox = false;

    TimerData *t = &data->timer;
    bool spinboxChanged = !hasAppliedSpinbox || t->hSpinbox != lastAppliedHSpinbox || t->mSpinbox != lastAppliedMSpinbox || t->sSpinbox != lastAppliedSSpinbox;

    if (spinboxChanged) {
        t->h = t->hSpinbox;
        t->m = t->mSpinbox;
        t->s = t->sSpinbox;
        lastAppliedHSpinbox = t->hSpinbox;
        lastAppliedMSpinbox = t->mSpinbox;
        lastAppliedSSpinbox = t->sSpinbox;
        hasAppliedSpinbox = true;
    }

    if (t->active) {
        time_t now = time(NULL);
        struct tm *now_tm = localtime(&now);
        if (t->lastUpdated.tm_year >= 70) {
            time_t last = mktime(&t->lastUpdated);
            if (last != (time_t)-1 && now > last) {
                int elapsed = (int) difftime(now, last);
                int totalSeconds = t->h * 3600 + t->m * 60 + t->s - elapsed;
                if (totalSeconds <= 0) {
                    totalSeconds = 0;
                    t->active = false;
                }
                t->h = totalSeconds / 3600;
                t->m = (totalSeconds % 3600) / 60;
                t->s = totalSeconds % 60;
            }
        }
        if (now_tm) {
            t->lastUpdated = *now_tm;
        }
    }
    snprintf(t->chars, sizeof(t->chars), "%03d:%02d:%02d", t->h, t->m, t->s);
}

void timer_spinbox_input(CyberwatchData *data, int difference){
    if (!data) return;
    TimerData *t = &data->timer;
    int sel = t->selectedElement;
    if (sel < 0 || sel >= TIMER_SELECTABLE_ELEMENT_COUNT) return;

    int wrap = (sel == 0) ? 1000 : 60; // hours wrap 0-999  minutes&seconds wrap 0-59
    int *target = NULL;
    char *buf = NULL;
    size_t bufsize = 0;

    if (sel == 0) { target = &t->hSpinbox; buf = t->hSpinboxChars; bufsize = sizeof(t->hSpinboxChars); }
    else if (sel == 1) { target = &t->mSpinbox; buf = t->mSpinboxChars; bufsize = sizeof(t->mSpinboxChars); }
    else if (sel == 2) { target = &t->sSpinbox; buf = t->sSpinboxChars; bufsize = sizeof(t->sSpinboxChars); }

    if (!target) return;

    int val = *target + difference;
    // wrap negatives
    val = ((val % wrap) + wrap) % wrap;
    *target = val;
    snprintf(buf, bufsize, "%d", val);
}

