#include "eval.h"
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <cglm/types-struct.h>
#include <ctype.h>
#include <leif/leif.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WINW 400
#define WINH 600
#define PANELH WINH / 4.0f
#define PADDING 15.0f
#define BUF_SIZE 64
#define FONT "/usr/share/fonts/Noto/static/NotoSans-Bold.ttf"
#define HIST_MAX 512

static void renderPanel();
static void renderButtons();
static void resizeCallBack(GLFWwindow *win, int w, int h);
static void execOp(uint32_t index);
static void addinput(char c);
static void insertCallback(void *inputData);
static void handleKeys();
static void evalInput();
static void histup();
static void histdown();
static void clearInput();
static void clearHist();

typedef struct {
    int winw, winh;

    LfInputField input;
    char inputBUF[BUF_SIZE];

    LfFont pannelFont;

    LfDiv *panel;
    float panelscroll, panelscroll_vel, panelscroll_max;

    const char *hist[HIST_MAX];
    double result_hist[HIST_MAX];
    uint32_t hist_size, hist_index;
} state;

static state s;

static const char *icons[20] = {"%", "C", "CE", "/", "7", "8", "9",
                                "*", "4", "5",  "6", "-", "1", "2",
                                "3", "+", " ",  "0", ".", "="};

void renderPanel() {
    {
        LfUIElementProps props = lf_get_theme().div_props;
        props.corner_radius = 10.0f;
        props.color = (LfColor){80, 80, 80, 255};
        lf_push_style_props(props);
        s.panel = lf_div_begin_ex(
            ((vec2s){PADDING, PADDING}),
            ((vec2s){s.winw - (PADDING * 2.0f), PANELH - (PADDING * 2.0f)}),
            true, &s.panelscroll, &s.panelscroll_vel);

        lf_pop_style_props();
    }

    for (uint32_t i = 0; i < s.hist_size; i++) {
        lf_text(s.hist[i]);
        lf_next_line();
        char result_buf[128];
        sprintf(result_buf, "%s%f", "= ", s.result_hist[i]);
        lf_text(result_buf);
        lf_next_line();
    }

    {
        LfUIElementProps props = lf_get_theme().inputfield_props;
        props.color = LF_NO_COLOR;
        props.border_width = 0.0f;
        props.text_color = LF_WHITE;
        lf_push_style_props(props);
        lf_push_font(&s.pannelFont);
        lf_input_text(&s.input);
        lf_pop_font();
        lf_pop_style_props();
    }

    lf_next_line();

    s.panel->total_area.x = lf_get_ptr_x();
    s.panel->total_area.y = lf_get_ptr_y();

    float area_y = s.panel->total_area.y - s.panelscroll;
    float size_y = s.panel->aabb.size.y;
    float maxscroll = ((area_y - size_y) * -1.0f) - s.pannelFont.font_size;
    s.panelscroll_max = maxscroll;

    lf_div_end();

    lf_set_ptr_y_absolute(lf_get_ptr_y() + PANELH - PADDING);
}

void renderButtons() {
    const float nx = 4;
    const float ny = 5;
    const float renderW = s.winw - (PADDING * (nx + 1));
    const float renderH = (s.winh - PANELH) - (PADDING * (ny + 1));
    const float w = renderW / nx;
    const float h = renderH / ny;
    float x = PADDING;
    float y = lf_get_ptr_y() + PADDING;

    uint32_t iconIndex = 0;
    for (uint32_t i = 0; i < ny; i++) {
        for (uint32_t j = 0; j < nx; j++) {
            lf_set_ptr_x_absolute(x);
            lf_set_ptr_y_absolute(y);

            LfUIElementProps props = lf_get_theme().button_props;
            props.margin_left = 0.0f;
            props.margin_right = 0.0f;
            props.margin_top = 0.0f;
            props.margin_bottom = 0.0f;
            props.padding = 0.0f;
            props.border_width = 0.0f;
            props.corner_radius = 10.0f;
            props.text_color = LF_WHITE;
            props.color = (LfColor){80, 80, 80, 255};

            lf_push_style_props(props);
            if (lf_button_fixed(icons[iconIndex], w, h) == LF_CLICKED) {
                execOp(iconIndex);
            }
            iconIndex++;
            lf_pop_style_props();

            x += w + PADDING;
        }
        x = PADDING;
        y += h + PADDING;
    }
}

void resizeCallBack(GLFWwindow *win, int w, int h) {
    (void)win;
    s.winw = w;
    s.winh = h;
    lf_resize_display(w, h);
    glViewport(0, 0, w, h);
};

static void insertCallback(void *inputData) {
    LfInputField *input = (LfInputField *)inputData;
    char ch = lf_char_event().charcode;
    bool isop = (ch == '+' || ch == '-' || ch == '/' || ch == '*');
    if (isdigit(ch) || isop) {
        lf_input_insert_char_idx(input, ch, input->cursor_index++);
    }
}

void execOp(uint32_t index) {
    switch (index) {
    case 0:
        break;
    case 1:
        clearInput();
        break;
    case 2:
        clearHist();
        break;
    case 3:
        addinput('/');
        break;
    case 4:
        addinput('7');
        break;
    case 5:
        addinput('8');
        break;
    case 6:
        addinput('9');
        break;
    case 7:
        addinput('*');
        break;
    case 8:
        addinput('4');
        break;
    case 9:
        addinput('5');
        break;
    case 10:
        addinput('6');
        break;
    case 11:
        addinput('-');
        break;
    case 12:
        addinput('1');
        break;
    case 13:
        addinput('2');
        break;
    case 14:
        addinput('3');
        break;
    case 15:
        addinput('+');
        break;
    case 16:
        break;
    case 17:
        addinput('0');
        break;
    case 18:
        addinput('.');
        break;
    case 19:
        evalInput();
        break;
    default:
        break;
    }
}

void handleKeys() {
    if (lf_key_went_down(GLFW_KEY_ENTER)) {
        evalInput();
    }
    if (lf_key_went_down(GLFW_KEY_UP)) {
        histdown();
    }
    if (lf_key_went_down(GLFW_KEY_DOWN)) {
        histup();
    }
}

void evalInput() {
    if (!strlen(s.inputBUF))
        return;
    double result = evalExpr(s.inputBUF);
    printf("Result is: %f\n", result);

    char *expr = malloc(strlen(s.inputBUF));
    strcpy(expr, s.inputBUF);
    s.hist[s.hist_size] = expr;
    s.result_hist[s.hist_size] = result;
    s.hist_size++;
    s.hist_index = s.hist_size;

    clearInput();

    s.panelscroll = s.panelscroll_max;
}

void addinput(char c) {
    lf_input_insert_char_idx(&s.input, c, s.input.cursor_index++);
    s.input.selected = true;
}

void histup() {
    if (s.hist_index + 1 >= s.hist_size)
        return;
    clearInput();
    s.hist_index++;
    strcpy(s.inputBUF, s.hist[s.hist_index]);
    s.input.cursor_index = strlen(s.input.buf);
}

void histdown() {
    if (s.hist_index - 1 < 0)
        return;
    clearInput();
    s.hist_index--;
    strcpy(s.inputBUF, s.hist[s.hist_index]);
    s.input.cursor_index = strlen(s.input.buf);
}

void clearInput() {
    memset(s.inputBUF, 0, sizeof(s.inputBUF));
    s.input.cursor_index = 0;
    lf_input_field_unselect_all(&s.input);
}

void clearHist() {
    memset(s.hist, 0, sizeof(s.hist));
    memset(s.result_hist, 0, sizeof(s.result_hist));
    s.hist_index = 0;
    s.hist_size = 0;
    s.panelscroll = 0.0f;
}

int main() {
    glfwInit();

    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    GLFWwindow *window = glfwCreateWindow(WINW, WINH, "calc", NULL, NULL);

    s.winw = WINW;
    s.winh = WINH;

    s.input = (LfInputField){
        .buf = s.inputBUF,
        .width = s.winw - 60.0f,
        .buf_size = BUF_SIZE,
        // .advance_height = true,
        .max_chars = BUF_SIZE,
        .placeholder = "",
        .selected = true,
        .insert_override_callback = insertCallback,
    };

    glfwMakeContextCurrent(window);

    lf_init_glfw(WINW, WINH, window);

    glfwSetFramebufferSizeCallback(window, resizeCallBack);

    LfTheme theme = lf_get_theme();
    theme.scrollbar_props.color = (LfColor){160, 160, 160, 255};
    theme.scrollbar_props.corner_radius = 1.5f;
    theme.font = lf_load_font(FONT, 24);
    s.pannelFont = lf_load_font(FONT, 30);
    lf_set_theme(theme);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        lf_begin();

        s.input.selected = true;

        renderPanel();
        renderButtons();

        lf_end();

        handleKeys();

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    for (uint32_t i = 0; i < s.hist_size; i++) {
        if (s.hist[i]) {
            free((void *)s.hist[i]);
        }
    }
    return 0;
}
