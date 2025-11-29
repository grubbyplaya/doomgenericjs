#include <emscripten.h>
#include <stdlib.h>

#include "config.h"
#include "doomtype.h"
#include "memio.h"
#include "mus2mid.h"

#include "deh_str.h"
#include "gusconf.h"
#include "i_sound.h"
#include "i_system.h"
#include "i_swap.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "sha1.h"
#include "w_wad.h"
#include "z_zone.h"

static int use_sfx_prefix;
static int sound_initialized;

// Initialise sound module
// Returns true if successfully initialised
static boolean JS_InitSound(boolean sfxPrefix) {
    use_sfx_prefix = sfxPrefix;
    sound_initialized = true;
    return EM_ASM_INT({
        return DGJS_InitSound();
    });
}

static void JS_ShutdownSound() {
    EM_ASM({
        DGJS_ShutdownSound();
    });
}

static void GetSfxLumpName(sfxinfo_t *sfx, char *buf, size_t buf_len) {
    // Linked sfx lumps? Get the lump number for the sound linked to.
    if (sfx->link != NULL)
        sfx = sfx->link;

    if (use_sfx_prefix) {
        M_snprintf(buf, buf_len, "ds%s", DEH_String(sfx->name));
    } else {
        M_StringCopy(buf, DEH_String(sfx->name), buf_len);
    }
}

// Returns the lump index of the given sound.
static int JS_GetSfxLumpNum(sfxinfo_t *sfxinfo) {
    char namebuf[9];
    GetSfxLumpName(sfxinfo, namebuf, sizeof(namebuf));
    return W_GetNumForName(namebuf);
}

// Called periodically to update the subsystem.
static void JS_UpdateSound() {
    EM_ASM({
        DGJS_UpdateSound();
    });
}

// Update the sound settings on the given channel.
static void JS_UpdateSoundParams(int channel, int vol, int sep) {
    EM_ASM({
        DGJS_UpdateSoundParams($0, $1, $2);
    }, channel, vol, sep);
}

// Start a sound on a given channel.  Returns the channel i or -1 on failure.
static int JS_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep) {
	if (!sound_initialized || channel < 0 || channel >= 16)
		return -1;

    return EM_ASM_INT({
        return DGJS_StartSound($0, $1, $2, $3);
    }, sfxinfo->lumpnum, channel, vol, sep);
}

// Stop the sound playing on the given channel.
static void JS_StopSound(int channel) {
    EM_ASM({
        DGJS_StopSound($0);
    }, channel);
}

// Query if a sound is playing on the given channel
static boolean JS_SoundIsPlaying(int channel) {
    return EM_ASM_INT({
        return DGJS_SoundIsPlaying($0);
    }, channel);
}


static void CacheSFX(sfxinfo_t *sfxinfo, char *namebuf, boolean isBuzzSfx) {
    int lumpnum = W_CheckNumForName(namebuf);
    char *data = W_CacheLumpNum(lumpnum, PU_STATIC);
    int lumplen = W_LumpLength(lumpnum);
    
    if (!isBuzzSfx) {
        EM_ASM({
            DGJS_CacheSFX_PCM($0, $1, $2);
        }, data, lumplen, lumpnum);
    } else {
        EM_ASM({
            DGJS_CacheSFX_Buzzer($0, $1, $2);
        }, data, lumplen, lumpnum);
    }
}

// Called on startup to precache sound effects (if necessary)
static void JS_CacheSounds(sfxinfo_t *sounds, int num_sounds) {
    char namebuf[9];
    for (int i = 0; i < num_sounds; i++) {
        GetSfxLumpName(&sounds[i], namebuf, sizeof(namebuf));
        sounds[i].lumpnum = W_CheckNumForName(namebuf);
        if (sounds[i].lumpnum != -1)
            CacheSFX(&sounds[i], namebuf, false);

        // only cache buzzer sounds if Doom/Doom II is the IWAD
        if (use_sfx_prefix) {
            namebuf[1] = 'P';
            if (W_CheckNumForName(namebuf) != -1)
                CacheSFX(&sounds[i], namebuf, true);
        }
    }
}

static snddevice_t sound_js_devices[] = {
    SNDDEVICE_PCSPEAKER,
	SNDDEVICE_SB,
	SNDDEVICE_PAS,
	SNDDEVICE_GUS,
	SNDDEVICE_WAVEBLASTER,
	SNDDEVICE_SOUNDCANVAS,
	SNDDEVICE_AWE32,
};


sound_module_t DG_sound_module = {
	sound_js_devices,
	arrlen(sound_js_devices),
	JS_InitSound,
	JS_ShutdownSound,
	JS_GetSfxLumpNum,
	JS_UpdateSound,
	JS_UpdateSoundParams,
	JS_StartSound,
	JS_StopSound,
	JS_SoundIsPlaying,
	JS_CacheSounds,
};