# doomgeneric.js
This is doomgeneric compiled to JavaScript using Emscripten, with bindings added to allow for sound and music support.

# Porting
## Generic
Implement these functions to get Doom running:
* DGJS_DrawFrame(framePtr, width, height): Draws a Doom frame. framePtr is a pointer to the framebuffer, which you will need to fetch from Module memory.
* DGJS_SetTitle(title, titleLen): Optional, allows you to get the IWAD name as a string. title is a pointer, which you will need to fetch from module memory.
* DGJS_GetKey(): Returns a two-element array, containing a key code at index 0 and its pressed state at index 1.

## Audio
These functions have to be implemented to add sound and music support. Leaving them as stubs is fine for the most part. Functions with a return value are **bolded.**

### Music
The global variable DGJS_MusicType is used to pass either a .mus file (false) or a .midi file (true).
* **DGJS_InitMusic()**: Initializes the music driver. Returns true by default and false if an error occured.
* **DGJS_RegisterSong(song, len)**: Passes a song to the music driver. It should return an identifier that is used to play the song.
* DGJS_UnRegisterSong(songId): Dereferences a song from the music driver. It uses the identifier returned by DGJS_RegisterSong.
* DGJS_PlaySong(songId, looping): Plays the song referenced by songId. If looping is true, the song should loop.
* DGJS_StopSong(): Stops the current song.
* DGJS_PauseSong(): Pauses the current song.
* DGJS_ResumeSong(): Resumes the current song.
* DGJS_SetMusicVolume(volume): Sets the song volume.
* DGJS_PollMusic(): Updates the song driver at a fixed interval.

### Sound effects
* DGJS_InitSound(): Initializes the sound driver.
* DGJS_ShutdownSound(): Shuts down the sound driver.
* DGJS_UpdateSound(): Updates the sound driver at a fixed interval.
* DGJS_UpdateSoundParams(channel, volume, pan): Sets the given sound channel's volume and panning.
* **DGJS_StartSound(sfxlumpnum, channel, volume, pan)**: Plays the sound effect specified by sfxlumpnum on a given channel, setting the channel's volume and panning. Should return channel by default, and -1 if an error occurs.
* DGJS_StopSound(channel): Stops the given sound channel.
* **DGJS_SoundIsPlaying(channel)**: Returns true if the given sound channel is playing, and false otherwise.
* DGJS_CacheSFX_PCM/Buzzer(data, size, sfxlumpnum): Caches the sound data, which is size bytes large, using sfxlumpnum as an identifier. data is passed as a pointer, so you will need to fetch the sound data from Module memory yourself.
