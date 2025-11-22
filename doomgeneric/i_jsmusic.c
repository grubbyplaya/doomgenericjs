#include <emscripten.h>
#include <string.h>
#include <malloc.h>

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

boolean music_initialized;
boolean music_is_playing;
int music_type;

// Initialise the music subsystem
static boolean JS_InitMusic(void) {
    music_initialized = true;

    music_type = EM_ASM_INT({
        return DGJS_MusicType;
    });

    return EM_ASM_INT({
        return DGJS_InitMusic();
    });
}

static MEMFILE *ConvertMus(byte *musdata, int len, char *filename) {
    MEMFILE *instream, *outstream;
    int result;

    instream = mem_fopen_read(musdata, len);
    outstream = mem_fopen_write();
    result = mus2mid(instream, outstream);

    if (result != 0) {
        mem_fclose(instream);
        mem_fclose(outstream);
        return NULL;
    }

    mem_fclose(instream);
    return outstream;
}

// Determine whether memory block is a .mid file
static boolean isMidi(byte *mem, int len) {
    return len > 4 && !memcmp(mem, "MThd", 4);
}

// Register a song handle from data
// Returns a handle that can be used to play the song
static void *JS_RegisterSong(void *data, int len) {
    int *handleID = malloc(sizeof(int));
    char *filename;
    unsigned char *bufData;
    MEMFILE *midfile;

    if (music_type || !isMidi(data, len)) {
        // register MIDI file
        filename = M_TempFile("doom.mid");
        midfile = ConvertMus(data, len, filename);

        bufData = midfile->buf;
        len = midfile->buflen;
    } else {
        // register MUS file
        bufData = data;
    }

    // pass song data to JS. It's up to the JS code to store the song data.
    *handleID = EM_ASM_INT({
        return DGJS_RegisterSong($0, $1);
    }, bufData, len);

    // free converted MIDI data if allocated
    if (music_type || !isMidi(data, len))
        mem_fclose(midfile);

    return handleID;
}

// Un-register (free) song data
static void JS_UnRegisterSong(void *handle) {
    if (!music_initialized || handle == NULL)
        return;

    EM_ASM({
        DGJS_UnRegisterSong($0);
    }, *(int*)handle);
}

// Play the song
static void JS_PlaySong(void *handle, boolean looping) {
    if (!music_initialized || handle == NULL)
        return;
    
    music_is_playing = true;
    EM_ASM({
        DGJS_PlaySong($0, $1);
    }, *(int*)handle, looping);   
}

static void JS_ShutdownMusic() {
    music_is_playing = false;
    EM_ASM({
        DGJS_ShutdownMusic();
    });
}

// Stop playing the current song.
static void JS_StopSong() {
    if (!music_initialized)
        return;

    music_is_playing = false;
    EM_ASM({
        DGJS_StopSong();
    });
}


// Pause music
static void JS_PauseSong() {
    music_is_playing = false;
    EM_ASM({
        DGJS_PauseSong();
    });
}

// Un-pause music
static void JS_ResumeSong() {
    music_is_playing = true;
    EM_ASM({
        DGJS_ResumeSong();
    });
}

// Set music volume - range 0-127
static void JS_SetMusicVolume(int volume) {
    EM_ASM({
        DGJS_SetMusicVolume($0);
    }, volume);
}

// Query if music is playing.
static boolean JS_MusicIsPlaying() {
    return music_is_playing;
}

// Invoked periodically to poll.
static void JS_PollMusic() {
    EM_ASM({
        DGJS_PollMusic();
    });
}

static snddevice_t music_js_devices[] =
{
	SNDDEVICE_PAS,
	SNDDEVICE_GUS,
	SNDDEVICE_WAVEBLASTER,
	SNDDEVICE_SOUNDCANVAS,
	SNDDEVICE_GENMIDI,
	SNDDEVICE_AWE32,
};


music_module_t DG_music_module =
{
	music_js_devices,
	arrlen(music_js_devices),
	JS_InitMusic,
	JS_ShutdownMusic,
	JS_SetMusicVolume,
	JS_PauseSong,
	JS_ResumeSong,
	JS_RegisterSong,
	JS_UnRegisterSong,
	JS_PlaySong,
	JS_StopSong,
	JS_MusicIsPlaying,
	JS_PollMusic,
};