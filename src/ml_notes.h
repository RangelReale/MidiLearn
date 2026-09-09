#ifndef H__ML_NOTES__H
#define H__ML_NOTES__H

#include <string>

/**
 * Pure note and pitch helpers, factored out of ctl_miditrack.cpp so they can be
 * tested without wxWidgets, TSE3 or a MIDI port. Nothing here touches global
 * state; the widget classes hold the state and call these.
 */

/** Whether a pitch class is a black key. Accepts any int: negative note numbers
 *  arise from transposing a low track down. */
bool ml_note_isblack(int note);

/** Low end of the visible key range: transposed, then clipped to note 0. */
int ml_range_lo(int notemin, int transpose);

/** High end of the visible key range: transposed, then clipped to note 127. */
int ml_range_hi(int notemax, int transpose);

/** White keys in [lo, hi]. Never returns less than 1, so it is safe as a
 *  divisor for a key width. */
int ml_white_keys(int lo, int hi);

/** How many white keys lie in [lo, note), i.e. the column a note draws in. */
int ml_white_index(int lo, int note);

/** Portuguese solfege name with an octave, e.g. "Do-5". Empty for a note
 *  outside 0..127. */
std::string ml_note_name(int note);

/** Index of the most-used entry in a 16-channel histogram, or -1 if all are
 *  zero. Ties go to the lower channel. */
int ml_most_used_channel(const unsigned int *chanuse);

#endif //H__ML_NOTES__H
