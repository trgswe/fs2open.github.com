/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/



#ifndef __SOUND_H__
#define __SOUND_H__

#include "globalincs/pstypes.h"
#include "sound/ds.h"
#include "utils/RandomRange.h"
#include "utils/id.h"

// Used for keeping track which low-level sound library is being used
#define SOUND_LIB_DIRECTSOUND		0
#define SOUND_LIB_RSX				1

#define GAME_SND_USE_DS3D			(1<<0)
#define GAME_SND_VOICE				(1<<1)
#define GAME_SND_NOT_VALID			(1<<2)

// Priorities that can be passed to snd_play() functions to limit how many concurrent sounds of a 
// given type are played.
#define SND_PRIORITY_MUST_PLAY				0
#define SND_PRIORITY_SINGLE_INSTANCE		1
#define SND_PRIORITY_DOUBLE_INSTANCE		2
#define SND_PRIORITY_TRIPLE_INSTANCE		3

// jg18 - new priority system
enum EnhancedSoundPriority
{
	SND_ENHANCED_PRIORITY_MUST_PLAY		= 0,
	SND_ENHANCED_PRIORITY_HIGH			= 1,
	SND_ENHANCED_PRIORITY_MEDIUM_HIGH	= 2,
	SND_ENHANCED_PRIORITY_MEDIUM		= 3,
	SND_ENHANCED_PRIORITY_MEDIUM_LOW	= 4,
	SND_ENHANCED_PRIORITY_LOW			= 5,
	SND_ENHANCED_PRIORITY_INVALID		= 6
};

struct EnhancedSoundData
{
	int priority = SND_ENHANCED_PRIORITY_INVALID;
	unsigned int limit = 0;		// limit on how many instances of the sound can play concurrently

	EnhancedSoundData();
	EnhancedSoundData(const int new_priority, const unsigned int new_limit);
};

extern const unsigned int SND_ENHANCED_MAX_LIMIT;

//For the adjust-audio-volume sexp
#define AAV_MUSIC		0
#define AAV_VOICE		1
#define AAV_EFFECTS		2

struct sound_load_tag {
};
using sound_load_id = ::util::ID<sound_load_tag, int, -1>;

using sound_handle = ds_sound_handle;

struct game_snd_entry {
	char filename[MAX_FILENAME_LEN];
	sound_load_id id = sound_load_id::invalid(); //!< index into Sounds[], where sound data is stored
	int id_sig       = -1;                       //!< signature of Sounds[] element

	game_snd_entry();
};

enum class GameSoundCycleType {
	RandomCycle,
	SequentialCycle
};

/**
 * Game level sound entities
 */
struct game_snd
{
	SCP_string name;				//!< The name of the sound

	SCP_vector<game_snd_entry> sound_entries; //!< A game sound consists of one or more distinct entries

	int	min = 0;					//!<distance at which sound will stop getting louder
	int max = 0;					//!<distance at which sound is inaudible
	int	flags = 0;

	GameSoundCycleType cycle_type = GameSoundCycleType::SequentialCycle;
	size_t last_entry_index; //!< The last sound entry used by this sound.

	util::UniformFloatRange pitch_range; //!< The range of possible pitch values used randomly for this sound
	util::UniformFloatRange volume_range; //!< The possible range of the default volume (range is (0, 1]).

	bool preload = false;			//!< preload sound (ie read from disk before mission starts)
	EnhancedSoundData enhanced_sound_data;

	game_snd( );
};

typedef struct sound_env
{
	int id;
	float volume;
	float damping;
	float decay;
} sound_env;

extern int		Sound_enabled;
extern float	Default_sound_volume;		// 0 -> 1.0
extern float	Default_voice_volume;		// 0 -> 1.0
extern float	Master_sound_volume;		// 0 -> 1.0
extern float	Master_voice_volume;		// 0 -> 1.0
extern size_t		Snd_sram;					// System memory consumed by sound data
extern float aav_voice_volume;
extern float aav_music_volume;
extern float aav_effect_volume;

void snd_set_effects_volume(float volume);

void snd_set_voice_volume(float volume);

//int	snd_load( char *filename, int hardware=0, int three_d=0, int *sig=NULL );
sound_load_id snd_load(game_snd_entry* entry, int* flags, int allow_hardware_load = 0);

int snd_unload(sound_load_id sndnum);
void	snd_unload_all();

// Plays a sound with volume between 0 and 1.0, where 0 is the
// inaudible and 1.0 is the loudest sound in the game.
// Pan goes from -1.0 all the way left to 0.0 in center to 1.0 all the way right.
sound_handle snd_play(game_snd* gs, float pan = 0.0f, float vol_scale = 1.0f,
                      int priority = SND_PRIORITY_SINGLE_INSTANCE, bool voice_message = false);

// Play a sound directly from index returned from snd_load().  Bypasses
// the sound management process of using game_snd.
sound_handle snd_play_raw(sound_load_id soundnum, float pan, float vol_scale = 1.0f,
                          int priority = SND_PRIORITY_MUST_PLAY);

// Plays a sound with volume between 0 and 1.0, where 0 is the
// inaudible and 1.0 is the loudest sound in the game.  It scales
// the pan and volume relative to the current viewer's location.
sound_handle snd_play_3d(game_snd* gs, vec3d* source_pos, vec3d* listen_pos, float radius = 0.0f, vec3d* vel = nullptr,
                         int looping = 0, float vol_scale = 1.0f, int priority = SND_PRIORITY_SINGLE_INSTANCE,
                         vec3d* sound_fvec = nullptr, float range_factor = 1.0f, int force = 0,
                         bool is_ambient = false);

// update the given 3d sound with a new position
void snd_update_3d_pos(sound_handle soudnnum, game_snd* gs, vec3d* new_pos, float radius = 0.0f,
                       float range_factor = 1.0f);

// Use these for looping sounds.
// Returns the handle of the sound. -1 if failed.
// If startloop or stoploop are not -1, then then are used.
sound_handle snd_play_looping(game_snd* gs, float pan = 0.0f, int start_loop = -1, int stop_loop = -1,
                              float vol_scale = 1.0f, int scriptingUpdateVolume = 1);

void snd_stop(sound_handle snd_handle);

// Sets the volume of a sound that is already playing.
// The volume is between 0 and 1.0, where 0 is the
// inaudible and 1.0 is the loudest sound in the game.
void snd_set_volume(sound_handle snd_handle, float volume);

// Sets the panning location of a sound that is already playing.
// Pan goes from -1.0 all the way left to 0.0 in center to 1.0 all the way right.
void snd_set_pan(sound_handle snd_handle, float pan);

// Sets the pitch (frequency) of a sound that is already playing
// Valid values for pitch are between 100 and 100000
void snd_set_pitch(sound_handle snd_handle, float pitch);
float snd_get_pitch(sound_handle snd_handle);

// Stops all sounds from playing, even looping ones.
void	snd_stop_all();

// determines if the sound handle is still palying
int snd_is_playing(sound_handle snd_handle);

// change the looping status of a sound that is playing
void snd_chg_loop_status(sound_handle snd_handle, int loop);

// return the time in ms for the duration of the sound
int snd_get_duration(sound_load_id snd_id);

// Get the file name of the specified sound
const char* snd_get_filename(sound_load_id snd_id);

// get a 3D vol and pan for a particular sound
int	snd_get_3d_vol_and_pan(game_snd *gs, vec3d *pos, float* vol, float *pan, float radius=0.0f, float range_factor=1.0f);

int	snd_init();
void	snd_close();

// Return 1 or 0 to show that sound system is inited ok
int	snd_is_inited();

void	snd_update_listener(vec3d *pos, vec3d *vel, matrix *orient);

void 	snd_use_lib(int lib_id);

int snd_num_playing();

int snd_get_data(sound_load_id handle, char* data);
int snd_size(sound_load_id handle, int* size);
void snd_do_frame();
void snd_adjust_audio_volume(int type, float percent, int time);

// repositioning of the sound buffer pointer
void snd_rewind(sound_handle snd_handle, float seconds); // rewind N seconds from the current position
void snd_ffwd(sound_handle snd_handle, float seconds);   // fast forward N seconds from the current position
void snd_set_pos(
    sound_handle snd_handle, float val,
    int as_pct); // set the position val as either a percentage (if as_pct) or as a # of seconds into the sound

void snd_get_format(sound_load_id handle, int* bits_per_sample, int* frequency);
int snd_time_remaining(sound_handle handle);

/**
 * @brief Get the sound id that was used to start the sound with the specified handle
 * @param snd_handle The sound handle to query the id from
 * @return The loaded sound handle or invalid handle on error
 */
sound_load_id snd_get_sound_id(sound_handle snd_handle);

// sound environment
extern unsigned int SND_ENV_DEFAULT;

int sound_env_set(sound_env *se);
int sound_env_get(sound_env *se, int preset = -1);
int sound_env_disable();
int sound_env_supported();

// adjust-audio-volume
void snd_aav_init();

#endif
