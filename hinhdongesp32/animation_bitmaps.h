#ifndef ANIMATION_BITMAPS_H
#define ANIMATION_BITMAPS_H

#include "DasaiBitmaps/1.h"
#include "DasaiBitmaps/2.h"
#include "DasaiBitmaps/3.h"
#include "DasaiBitmaps/4.h"
#include "DasaiBitmaps/5.h"
#include "DasaiBitmaps/6.h"
#include "DasaiBitmaps/9.h"
#include "DasaiBitmaps/10.h"
#include "DasaiBitmaps/11.h"
#include "DasaiBitmaps/13.h"
#include "DasaiBitmaps/14.h"
#include "DasaiBitmaps/16.h"
#include "DasaiBitmaps/18.h"
#include "DasaiBitmaps/19.h"
#include "DasaiBitmaps/21.h"
#include "DasaiBitmaps/22.h"
#include "DasaiBitmaps/23.h"
#include "DasaiBitmaps/24.h"
#include "DasaiBitmaps/26.h"
#include "DasaiBitmaps/27.h"
#include "DasaiBitmaps/28.h"
#include "DasaiBitmaps/29.h"
#include "DasaiBitmaps/30.h"
#include "DasaiBitmaps/32.h"
#include "DasaiBitmaps/37.h"
#include "DasaiBitmaps/38.h"
#include "DasaiBitmaps/39.h"
#include "DasaiBitmaps/42.h"
#include "DasaiBitmaps/jojos.h"

struct BitmapMood {
    const uint8_t (*frames)[1024];
    int count;
};

const BitmapMood extraMoods[] = {
    {anim_1_frames, 19}, {anim_2_frames, 21}, {anim_3_frames, 15}, {anim_4_frames, 52},
    {anim_5_frames, 27}, {anim_6_frames, 62}, {anim_9_frames, 41}, {anim_10_frames, 28},
    {anim_11_frames, 29}, {anim_13_frames, 105}, {anim_14_frames, 61}, {anim_16_frames, 29},
    {anim_18_frames, 16}, {anim_19_frames, 15}, {anim_21_frames, 57}, {anim_22_frames, 19},
    {anim_23_frames, 62}, {anim_24_frames, 72}, {anim_26_frames, 37}, {anim_27_frames, 50},
    {anim_28_frames, 17}, {anim_29_frames, 39}, {anim_30_frames, 19}, {anim_32_frames, 31},
    {anim_37_frames, 64}, {anim_38_frames, 28}, {anim_39_frames, 33}, {anim_42_frames, 137},
    {jojos_frames, 55}
};

const int EXTRA_MOODS_COUNT = 29;

#endif
