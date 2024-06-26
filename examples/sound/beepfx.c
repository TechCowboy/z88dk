
// For targets with 1-bit sound

#include <stdio.h>
#include <sound.h>
#include <stdlib.h>

#pragma printf = "%2u %s"

// list of all beepfx sound effects

typedef struct effects_s
{

   void *effect;
   char *name;
   
} effects_t;

const effects_t beepfx[] = {

    {BEEPFX_SHOT_1,            "BEEPFX_SHOT_1"},
    {BEEPFX_SHOT_2,            "BEEPFX_SHOT_2"},
    {BEEPFX_JUMP_1,            "BEEPFX_JUMP_1"},
    {BEEPFX_JUMP_2,            "BEEPFX_JUMP_2"},
    {BEEPFX_PICK,              "BEEPFX_PICK"},
    {BEEPFX_DROP_1,            "BEEPFX_DROP_1"},
    {BEEPFX_DROP_2,            "BEEPFX_DROP_2"},
    {BEEPFX_GRAB_1,            "BEEPFX_GRAB_1"},
    {BEEPFX_GRAB_2,            "BEEPFX_GRAB_2"},
    {BEEPFX_FAT_BEEP_1,        "BEEPFX_FAT_BEEP_1"},
    {BEEPFX_FAT_BEEP_2,        "BEEPFX_FAT_BEEP_2"},
    {BEEPFX_FAT_BEEP_3,        "BEEPFX_FAT_BEEP_3"},
    {BEEPFX_HARSH_BEEP_1,      "BEEPFX_HARSH_BEEP_1"},
    {BEEPFX_HARSH_BEEP_2,      "BEEPFX_HARSH_BEEP_2"},
    {BEEPFX_HARSH_BEEP_3,      "BEEPFX_HARSH_BEEP_3"},
    {BEEPFX_HIT_1,             "BEEPFX_HIT_1"},
    {BEEPFX_HIT_2,             "BEEPFX_HIT_2"},
    {BEEPFX_HIT_3,             "BEEPFX_HIT_3"},
    {BEEPFX_HIT_4,             "BEEPFX_HIT_4"},
    {BEEPFX_JET_BURST,         "BEEPFX_JET_BURST"},
    {BEEPFX_BOOM_1,            "BEEPFX_BOOM_1"},
    {BEEPFX_BOOM_2,            "BEEPFX_BOOM_2"},
    {BEEPFX_BOOM_3,            "BEEPFX_BOOM_3"},
    {BEEPFX_BOOM_4,            "BEEPFX_BOOM_4"},
    {BEEPFX_BOOM_5,            "BEEPFX_BOOM_5"},
    {BEEPFX_BOOM_6,            "BEEPFX_BOOM_6"},
    {BEEPFX_BOOM_7,            "BEEPFX_BOOM_7"},
    {BEEPFX_BOOM_8,            "BEEPFX_BOOM_8"},
    {BEEPFX_ITEM_1,            "BEEPFX_ITEM_1"},
    {BEEPFX_ITEM_2,            "BEEPFX_ITEM_2"},
    {BEEPFX_ITEM_3,            "BEEPFX_ITEM_3"},
    {BEEPFX_ITEM_4,            "BEEPFX_ITEM_4"},
    {BEEPFX_ITEM_5,            "BEEPFX_ITEM_5"},
    {BEEPFX_ITEM_6,            "BEEPFX_ITEM_6"},
    {BEEPFX_SWITCH_1,          "BEEPFX_SWITCH_1"},
    {BEEPFX_SWITCH_2,          "BEEPFX_SWITCH_2"},
    {BEEPFX_POWER_OFF,         "BEEPFX_POWER_OFF"},
    {BEEPFX_SCORE,             "BEEPFX_SCORE"},
    {BEEPFX_CLANG,             "BEEPFX_CLANG"},
    {BEEPFX_WATER_TAP,         "BEEPFX_WATER_TAP"},
    {BEEPFX_SELECT_1,          "BEEPFX_SELECT_1"},
    {BEEPFX_SELECT_2,          "BEEPFX_SELECT_2"},
    {BEEPFX_SELECT_3,          "BEEPFX_SELECT_3"},
    {BEEPFX_SELECT_4,          "BEEPFX_SELECT_4"},
    {BEEPFX_SELECT_5,          "BEEPFX_SELECT_5"},
    {BEEPFX_SELECT_6,          "BEEPFX_SELECT_6"},
    {BEEPFX_SELECT_7,          "BEEPFX_SELECT_7"},
    {BEEPFX_ALARM_1,           "BEEPFX_ALARM_1"},
    {BEEPFX_ALARM_2,           "BEEPFX_ALARM_2"},
    {BEEPFX_ALARM_3,           "BEEPFX_ALARM_3"},
    {BEEPFX_EAT,               "BEEPFX_EAT"},
    {BEEPFX_GULP,              "BEEPFX_GULP"},
    {BEEPFX_ROBOBLIP,          "BEEPFX_ROBOBLIP"},
    {BEEPFX_NOPE,              "BEEPFX_NOPE"},
    {BEEPFX_UH_HUH,            "BEEPFX_UH_HUH"},
    {BEEPFX_OLD_COMPUTER,      "BEEPFX_OLD_COMPUTER"},
    {BEEPFX_YEAH,              "BEEPFX_YEAH"},
    {BEEPFX_AWW,               "BEEPFX_AWW"}

};

void main()
{
   unsigned int i;

   
   printf("LIST OF BEEPFX EFFECTS:\n\n");
   
   for (i = 0; i < sizeof(beepfx) / sizeof(effects_t); ++i)
   {
      printf("%2u: %s\n", i, beepfx[i].name);
      
      bit_beepfx(beepfx[i].effect);
   }
}
