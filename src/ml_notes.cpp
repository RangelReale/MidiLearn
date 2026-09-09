#include "ml_notes.h"

#include <sstream>

int ml_pitch_class(int note)
{
    // Floor-mod: note is a transposed pitch and can be negative, and C++ % keeps
    // the sign of the dividend, so a plain note%12 is not a pitch class.
    return ((note%12)+12)%12;
}

bool ml_note_isblack(int note)
{
    bool isblack=false;
    switch (ml_pitch_class(note))
    {
    case 1: case 3: case 6: case 8: case 10:
        isblack=true;
        break;
    }
    return isblack;
}

int ml_range_lo(int notemin, int transpose)
{
    const int lo=notemin+transpose;
    return lo<0?0:lo;
}

int ml_range_hi(int notemax, int transpose)
{
    const int hi=notemax+transpose;
    return hi>127?127:hi;
}

int ml_white_keys(int lo, int hi)
{
    int n=0;
    for (int i=lo; i<=hi; i++)
        if (!ml_note_isblack(i)) n++;

    return n<1?1:n; // never divide a key width by zero
}

int ml_white_index(int lo, int note)
{
    int nc=0;
    for (int ctn=lo; ctn<note; ctn++)
        if (!ml_note_isblack(ctn)) nc++;

    return nc;
}

std::string ml_note_name(int note)
{
    std::string dest;

    if (note >= 0 && note <= 127)
    {

        switch (note%12)
        {
            case 0:  dest.append("Do");  break;
            case 1:  dest.append("Do#"); break;
            case 2:  dest.append("Re");  break;
            case 3:  dest.append("Re#"); break;
            case 4:  dest.append("Mi");  break;
            case 5:  dest.append("Fa");  break;
            case 6:  dest.append("Fa#"); break;
            case 7:  dest.append("Sol");  break;
            case 8:  dest.append("Sol#"); break;
            case 9:  dest.append("La");  break;
            case 10: dest.append("La#"); break;
            case 11: dest.append("Si");  break;
        }

        dest.append("-");

        {
            std::ostringstream o;
            o << note/12;
            dest.append(o.str());
        }
    }

    return dest;
}

int ml_most_used_channel(const unsigned int *chanuse)
{
    int channel=-1;
    unsigned int maxuses=0;
    for (int i=0; i<16; i++)
    {
        if (chanuse[i]>maxuses)
        {
            channel=i;
            maxuses=chanuse[i];
        }
    }
    return channel;
}
