#include "../include/global.h"
#include "../include/alloc.h"
#include "../include/constants/species.h"
#include "../include/event_data.h"
#include "../include/main.h"
#include "../include/overworld.h"
#include "../include/pokemon.h"
#include "../include/save.h"
#include "../include/sound.h"
#include "../include/sprite.h"
#include "../include/string_util.h"
#include "../include/task.h"

#define SPAWN_BCC_SCREEN_ON_NULL
#define SPAWN_BCC_MON_ON_NULL

#define FLAG_BUG_CATCHING_CONTEST (0x2342)
#define IS_IN_BUG_CATCHING_CONTEST (FlagGet(FLAG_BUG_CATCHING_CONTEST))
#define gRemainingParkBalls (*(u8 *)0x0203FEC8)
#define SECONDS_IN_CONTEST (20 * 60) // 20 minute total time
#define gMonIconPalettes ((u16 *)(0x083d3740))

struct BugCatchingContestSwapScreen
{
    //struct Sprite *monIconSprites[2];
    u8 spriteIds[2]; // index of gSprites
    u8 cursorPos;
};

extern struct Pokemon *gBugContestMon;
extern struct BugCatchingContestSwapScreen *gBCCSwapScreen;

// change start menu behavior:
// get rid of save, add script to quit out


// timer for bug catching contest


// functions to spawn the icons in their own textboxes
void SpawnIconsForBCC(void)
{
    // need to print a textbox to the screen and also print the mon icons to the screen such that they go through the box
    // animate the one that is selected

    // sanity check - if data is not initialized, do nothing.  maybe 
    if (gBCCSwapScreen == NULL || gBugContestMon == NULL)
    {
#ifdef SPAWN_BCC_SCREEN_ON_NULL
        if (gBCCSwapScreen == NULL)
        {
            gBCCSwapScreen = Alloc(sizeof(struct BugCatchingContestSwapScreen));
        }
#endif
#ifdef SPAWN_BCC_MON_ON_NULL
        if (gBugContestMon == NULL)
        {
            gBugContestMon = AllocZeroed(sizeof(struct Pokemon));
            CreateMon(gBugContestMon, SPECIES_SCYTHER, 20, 32, 0, 0, 0, 0);
        }
#else
        return;
#endif
    }

    // load icon pals
    LoadMonIconPalettes();

    // set regs that will let me print a textbox and print an icon over it?
    SetGpuRegBits(REG_OFFSET_DISPCNT, DISPCNT_OBJWIN_ON);
    SetGpuRegBits(REG_OFFSET_WINOUT, WINOUT_WINOBJ_OBJ);
    SetGpuRegBits(REG_OFFSET_WININ, WININ_WIN0_OBJ);

    // print icon of gBugContestMon -- first the actual icon
    u32 species = GetMonData(gBugContestMon, MON_DATA_SPECIES, NULL);
    u32 pid = GetMonData(gBugContestMon, MON_DATA_PERSONALITY, NULL);
    u32 spriteId;

    spriteId = CreateMonIcon(species, 0x0809718d, 40+16, 76+16, 0, pid, 0);
    gSprites[spriteId].oam.priority = 0;
    gSprites[spriteId].invisible = 0;
    gSprites[spriteId].pos1.x = 0;
    gSprites[spriteId].pos1.y = 0;
    gSprites[spriteId].pos2.x = 56;
    gSprites[spriteId].pos2.y = 92;

    gBCCSwapScreen->spriteIds[0] = spriteId;

    // then cut it out of the textbox
    spriteId = CreateMonIcon(species, 0x0809718d, 40+16, 76+16, 0, pid, 0);
    gSprites[spriteId].oam.priority = 0;
    gSprites[spriteId].invisible = 0;
    gSprites[spriteId].pos1.x = 0;
    gSprites[spriteId].pos1.y = 0;
    gSprites[spriteId].pos2.x = 56;
    gSprites[spriteId].pos2.y = 92;
    gSprites[spriteId].oam.objMode = ST_OAM_OBJ_WINDOW;

    gBCCSwapScreen->spriteIds[1] = spriteId;
    // let's see if what we have so far works
}

// score caught mon in gBugContestMon
// max score is 400:
// level as percentage of max that can be found
// iv's relative to max as percentage (186)
// hp relative to max as percentage
// rarity factor -- caterpie metapod weedle kakuna wurmple silcoon cascoon kricketot are all at 60, scyther pinsir are at 100, 80 everything else
// scyther/pinsir with perfect iv's and no damage dealt is all that can get 400


// functions to handle storing bug catching contest mon direct from party as well as giving the player the bug catching contest mon
u32 StoreCaughtBCCMon(void)
{
    u32 ret = FALSE;
    if (gPlayerPartyCount > 1)
    {
        if (gBugContestMon == NULL)
            gBugContestMon = Alloc(sizeof(struct Pokemon));
        memcpy(gBugContestMon, &gPlayerParty[1], sizeof(struct Pokemon));
        memset(&gPlayerParty[1], 0, sizeof(struct Pokemon));
        ret = TRUE;
    }
    return ret;
}

u32 FreeCaughtBCCMonAndDeposit(void)
{
    u32 ret = GiveMonToPlayer(gBugContestMon);
    if (ret != 2) // can't give to player
    {
        Free(gBugContestMon);
        gBugContestMon = NULL;
    }
    return ret;
}
