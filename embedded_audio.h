#ifndef EMBEDDED_AUDIO_H
#define EMBEDDED_AUDIO_H

#include <stddef.h>

// 旋转音效
extern const unsigned char rotate_data[];
extern const size_t rotate_size;

// 移动音效
extern const unsigned char move_data[];
extern const size_t move_size;

// 下落音效
extern const unsigned char drop_data[];
extern const size_t drop_size;

// 消行音效
extern const unsigned char clear_data[];
extern const size_t clear_size;

// 游戏结束音效
extern const unsigned char gameover_data[];
extern const size_t gameover_size;

// 主题音乐1
extern const unsigned char theme1_data[];
extern const size_t theme1_size;

// 主题音乐2
extern const unsigned char theme2_data[];
extern const size_t theme2_size;

#endif // EMBEDDED_AUDIO_H 