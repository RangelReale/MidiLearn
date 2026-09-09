// Unit tests for the pure note helpers in src/ml_notes.cpp. Deliberately links
// nothing but that file: no wxWidgets, no TSE3, no MIDI port, so it runs anywhere.
#include "ml_notes.h"

#include <iostream>
#include <string>

static int checks = 0;
static int failures = 0;

static void check(bool ok, const char *expr, int line)
{
    ++checks;
    if (!ok)
    {
        ++failures;
        std::cout << "FAIL line " << line << ": " << expr << std::endl;
    }
}

static void check_eq(int got, int want, const char *expr, int line)
{
    ++checks;
    if (got != want)
    {
        ++failures;
        std::cout << "FAIL line " << line << ": " << expr
                  << " == " << got << ", expected " << want << std::endl;
    }
}

static void check_str(const std::string &got, const std::string &want,
                      const char *expr, int line)
{
    ++checks;
    if (got != want)
    {
        ++failures;
        std::cout << "FAIL line " << line << ": " << expr
                  << " == " << got << ", expected " << want << std::endl;
    }
}

#define CHECK(e)         check((e), #e, __LINE__)
#define CHECK_EQ(e, w)   check_eq((e), (w), #e, __LINE__)
#define CHECK_STR(e, w)  check_str((e), (w), #e, __LINE__)

static void test_note_isblack()
{
    // one octave from C
    CHECK(!ml_note_isblack(0));   // C
    CHECK( ml_note_isblack(1));   // C#
    CHECK(!ml_note_isblack(2));   // D
    CHECK( ml_note_isblack(3));   // D#
    CHECK(!ml_note_isblack(4));   // E
    CHECK(!ml_note_isblack(5));   // F
    CHECK( ml_note_isblack(6));   // F#
    CHECK(!ml_note_isblack(7));   // G
    CHECK( ml_note_isblack(8));   // G#
    CHECK(!ml_note_isblack(9));   // A
    CHECK( ml_note_isblack(10));  // A#
    CHECK(!ml_note_isblack(11));  // B

    // and an octave up, to prove it is the pitch class that matters
    CHECK(!ml_note_isblack(60));
    CHECK( ml_note_isblack(61));
    CHECK(!ml_note_isblack(127));

    // Negative notes: reachable by transposing a low track down. C++ % keeps the
    // sign, so the pre-fix switch matched nothing and called all of these white.
    CHECK(!ml_note_isblack(-1));   // B
    CHECK( ml_note_isblack(-2));   // A#  <- the regression
    CHECK(!ml_note_isblack(-12));  // C
    CHECK( ml_note_isblack(-11));  // C#  <- the regression
    CHECK( ml_note_isblack(-14));  // A#
}

static void test_range()
{
    CHECK_EQ(ml_range_lo(40, 0), 40);
    CHECK_EQ(ml_range_lo(40, 5), 45);
    CHECK_EQ(ml_range_lo(40, -5), 35);
    CHECK_EQ(ml_range_lo(2, -12), 0);    // clipped, not negative
    CHECK_EQ(ml_range_lo(0, -1), 0);

    CHECK_EQ(ml_range_hi(80, 0), 80);
    CHECK_EQ(ml_range_hi(80, -5), 75);
    CHECK_EQ(ml_range_hi(120, 12), 127); // clipped, not 132
    CHECK_EQ(ml_range_hi(127, 1), 127);
}

static void test_white_keys()
{
    CHECK_EQ(ml_white_keys(0, 11), 7);    // one octave
    CHECK_EQ(ml_white_keys(0, 12), 8);
    CHECK_EQ(ml_white_keys(60, 71), 7);
    CHECK_EQ(ml_white_keys(0, 0), 1);

    // A range holding only black keys must still be a usable divisor.
    CHECK_EQ(ml_white_keys(1, 1), 1);
    CHECK_EQ(ml_white_keys(6, 6), 1);
    CHECK(ml_white_keys(1, 1) > 0);
}

static void test_white_index()
{
    CHECK_EQ(ml_white_index(0, 0), 0);
    CHECK_EQ(ml_white_index(0, 1), 1);    // note 0 is white
    CHECK_EQ(ml_white_index(0, 2), 1);    // note 1 is black
    CHECK_EQ(ml_white_index(0, 12), 7);   // a full octave of white keys
    CHECK_EQ(ml_white_index(60, 60), 0);
    CHECK_EQ(ml_white_index(60, 62), 1);

    // monotonic, and never decreasing
    int last = 0;
    for (int n = 60; n <= 72; n++)
    {
        const int idx = ml_white_index(60, n);
        CHECK(idx >= last);
        last = idx;
    }
}

// The regression 2.8 was really about: the key count and the note columns must
// come from the same range, or notes draw past the right edge once the song is
// transposed. Here the count and the positions are checked against each other
// across the whole transpose range the buttons can reach.
static void test_layout_consistent_under_transpose()
{
    const int notemin = 21;  // a bass-ish track
    const int notemax = 96;

    for (int tr = -36; tr <= 36; tr++)
    {
        const int lo = ml_range_lo(notemin, tr);
        const int hi = ml_range_hi(notemax, tr);
        if (lo > hi) continue;

        const int whites = ml_white_keys(lo, hi);

        for (int n = lo; n <= hi; n++)
        {
            const int idx = ml_white_index(lo, n);
            CHECK(idx >= 0);
            // Every drawn note stays within the keyboard it is drawn on.
            CHECK(idx <= whites);
            if (!ml_note_isblack(hi))
                CHECK(idx < whites);
        }
    }
}

static void test_note_name()
{
    CHECK_STR(ml_note_name(0), "Do-0");
    CHECK_STR(ml_note_name(60), "Do-5");
    CHECK_STR(ml_note_name(61), "Do#-5");
    CHECK_STR(ml_note_name(62), "Re-5");
    CHECK_STR(ml_note_name(64), "Mi-5");
    CHECK_STR(ml_note_name(65), "Fa-5");
    CHECK_STR(ml_note_name(67), "Sol-5");
    CHECK_STR(ml_note_name(69), "La-5");
    CHECK_STR(ml_note_name(71), "Si-5");
    CHECK_STR(ml_note_name(127), "Sol-10");

    // outside the MIDI range it returns nothing at all
    CHECK_STR(ml_note_name(-1), "");
    CHECK_STR(ml_note_name(128), "");
}

static void test_most_used_channel()
{
    unsigned int h[16];

    for (int i = 0; i < 16; i++) h[i] = 0;
    CHECK_EQ(ml_most_used_channel(h), -1);   // nothing counted at all

    for (int i = 0; i < 16; i++) h[i] = 0;
    h[3] = 1;
    CHECK_EQ(ml_most_used_channel(h), 3);

    for (int i = 0; i < 16; i++) h[i] = 0;
    h[9] = 400; h[2] = 40;
    CHECK_EQ(ml_most_used_channel(h), 9);

    for (int i = 0; i < 16; i++) h[i] = 0;
    h[15] = 7;
    CHECK_EQ(ml_most_used_channel(h), 15);   // the last slot is reachable

    // a tie goes to the lower channel
    for (int i = 0; i < 16; i++) h[i] = 0;
    h[0] = 5; h[3] = 5;
    CHECK_EQ(ml_most_used_channel(h), 0);
}

int main()
{
    test_note_isblack();
    test_range();
    test_white_keys();
    test_white_index();
    test_layout_consistent_under_transpose();
    test_note_name();
    test_most_used_channel();

    std::cout << checks << " checks, " << failures << " failed" << std::endl;
    return failures == 0 ? 0 : 1;
}
