#include "simple_logger.h"
#include "simple_json.h"

#include "gfc_input.h"

#include "gf2d_sprite.h"
#include "gf2d_element_label.h"

#include "cutscene.h"
#include "editor.h"
#include "audio.h"

SJson* _cutscene, * _dialogue;
Sprite* _playerPortraits, * _bossPortraits;
Window* _cut_menu;
char* _text;
GFC_TextLine _buf;
Uint8 _speaker, _frame, _skip_frame, _eol, _eos, _last_space;
int _line, _char;

Uint8 is_cutscene()
{
	return (_cutscene != NULL);
}

void cutscene_free()
{
	if (_cutscene) sj_free(_cutscene);
	if (_playerPortraits) gf2d_sprite_free(_playerPortraits);
	if (_bossPortraits) gf2d_sprite_free(_bossPortraits);
	if (_cut_menu) gf2d_window_free(_cut_menu);
	_cutscene = NULL;
	_playerPortraits = NULL;
	_bossPortraits = NULL;
	_cut_menu = NULL;
}

void cutscene_update()
{
	if (!_cutscene) return;

	SJson* lineInfo;
	char ch;
	GFC_TextLine buf2;
	_skip_frame = (++_skip_frame) % 2;
	if (_skip_frame) return; // render text at 30 FPS to make sound sync up better
	if (_eos & !gfc_input_key_down("w"))
	{
		return;
	}
	if (_eos) _eos = 0;

	if (_eol)
	{
		_line++;
		lineInfo = sj_array_get_nth(_dialogue, _line);
		if (!lineInfo) // end of cutscene
		{
			cutscene_free();
			end_loop();
			return;
		}

		_text = _strdup(sj_object_get_string(lineInfo, "text"));
		sj_object_get_uint8(lineInfo, "speaker", &_speaker);
		sj_object_get_uint8(lineInfo, "frame", &_frame);
		gf2d_element_label_set_text(gf2d_window_get_element_by_id(_cut_menu, 1), "");
		gf2d_element_label_set_text(gf2d_window_get_element_by_id(_cut_menu, 2), "");
		memset(_buf, 0, sizeof(GFC_TextLine));
		_eol = 0;
		_char = 0;
		_last_space = 0;
	}

	ch = _text[_char];
	if (!ch)
	{
		_eol = 1;
		_eos = 1;
		return;
	}
	_buf[_char] = ch;

	if (ch != ' ') gf2d_windows_play_sound("text");
	else if (_char < 50) _last_space = _char;
	if (ch == '.' || ch == '!' || ch == '?' || ch == ',') _eos = 1;

	if (_char >= 50)
	{
		memset(buf2, 0, sizeof(GFC_TextLine));
		strncpy(buf2, _buf, _last_space + 1);
		gf2d_element_label_set_text(gf2d_window_get_element_by_id(_cut_menu, 1), buf2);
		strcpy(buf2, _buf + _last_space + 1);
		gf2d_element_label_set_text(gf2d_window_get_element_by_id(_cut_menu, 2), buf2);
	}
	else
	{
		gf2d_element_label_set_text(gf2d_window_get_element_by_id(_cut_menu, 1), _buf);
		gf2d_element_label_set_text(gf2d_window_get_element_by_id(_cut_menu, 2), "");
	}

	_char++;
	if (_char == strlen(_text))
	{
		_eol = 1;
		_eos = 1;
	}
}

void cutscene_draw_portrait()
{
	GFC_Vector2D scale, flip;
	Sprite* port;
	scale = gfc_vector2d(2.5, 2.5);
	flip = gfc_vector2d(_speaker, 0);
	if (_speaker) port = _bossPortraits;
	else port = _playerPortraits;
	gf2d_sprite_draw(
		port,
		gfc_vector2d(1100, 476),
		&scale,
		NULL,
		NULL,
		&flip,
		NULL,
		_frame
	);
}

void cutscene_load(const char* cutscene)
{
	SJson* config;
	const char* str, *str2;
	if (_cutscene) return; // one at a time, please
	if (is_editor_open()) return; // don't load cutscenes in the editor

	_cut_menu = gf2d_window_load("menus/cutscene_window.json");
	if (!_cut_menu) return;
	
	_cutscene = sj_load(cutscene);
	if (!_cutscene)
	{
		cutscene_free();
		return;
	}

	config = sj_object_get_value(_cutscene, "cutsceneInfo");
	if (!config)
	{
		cutscene_free();
		return;
	}

	str = _strdup(sj_object_get_string(config, "player_portraits"));
	str2 = _strdup(sj_object_get_string(config, "boss_portraits"));
	_playerPortraits = gf2d_sprite_load_all(str, 40, 40, 1, 0);
	_bossPortraits = gf2d_sprite_load_all(str2, 40, 40, 1, 0);
	if (!_playerPortraits || !_bossPortraits)
	{
		cutscene_free();
		return;
	}

	_dialogue = sj_object_get_value(config, "dialogue");
	if (!_dialogue)
	{
		cutscene_free();
		return;
	}

	str = _strdup(sj_object_get_string(config, "music_intro"));
	str2 = _strdup(sj_object_get_string(config, "music_loop"));
	load_music_pair(str, str2, 1);

	_line = -1;
	_skip_frame = 1;
	_eol = 1;
}