#include "instrument.h"

#include "instrument/violin.h"
#include "instrument/pizzicato_strings.h"
#include "instrument/flute.h"
#include "instrument/recorder.h"
#include "instrument/square.h"
#include "instrument/sine.h"
#include "instrument/saw.h"
#include "instrument/triangle.h"
#include "instrument/guitar.h"
#include "instrument/piano.h"

// https://en.wikipedia.org/wiki/General_MIDI#Percussion

const struct instrument instruments[128] = {
	// Piano
	piano, // 0
	{}, // 1
	{}, // 2
	{}, // 3
	{}, // 4
	{}, // 5
	{}, // 6
	{}, // 7

	// Chromatic Percussion
	{}, // 8
	{}, // 9
	{}, // 10
	{}, // 11
	{}, // 12
	{}, // 13
	{}, // 14
	{}, // 15

	// Organ
	{}, // 16
	{}, // 17
	{}, // 18
	{}, // 19
	{}, // 20
	{}, // 21
	{}, // 22
	{}, // 23

	// Guitar
		guitar, // 24
	{}, // 25
	{}, // 26
	{}, // 27
	{}, // 28
	{}, // 29
	{}, // 30
	{}, // 31

	// Bass
	{}, // 32
	{}, // 33
	{}, // 34
	{}, // 35
	{}, // 36
	{}, // 37
	{}, // 38
	{}, // 39

	// Strings
	violin, // 40
		violin, // 41
		violin, // 42
		violin, // 43
		violin, // 44
	pizzicato_strings, // 45
	{}, // 46
	{}, // 47

	// Ensemble
	{}, // 48
	{}, // 49
	{}, // 50
	{}, // 51
	{}, // 52
	{}, // 53
	{}, // 54
	{}, // 55

	// Brass
	{}, // 56
	{}, // 57
	{}, // 58
	{}, // 59
	{}, // 60
	{}, // 61
	{}, // 62
	{}, // 63

	// Reed
	{}, // 64
	{}, // 65
	{}, // 66
	{}, // 67
	{}, // 68
	{}, // 69
	{}, // 70
	{}, // 71

	// Pipe
	{}, // 72
	flute, // 73
	recorder, // 74
	{}, // 75
	{}, // 76
	{}, // 77
	{}, // 78
	{}, // 79

	// Synth Lead
	square, // 80
	saw, // 81
	{}, // 82
	{}, // 83
	{}, // 84
	{}, // 85
	{}, // 86
	{}, // 87

	// Synth Pad
	{}, // 88
	{}, // 89
	{}, // 90
	{}, // 91
	{}, // 92
	{}, // 93
	{}, // 94
	{}, // 95

	// Synth Effects
	{}, // 96
	{}, // 97
	{}, // 98
	{}, // 99
	{}, // 100
	{}, // 101
	{}, // 102
	{}, // 103

	// Ethnic
	{}, // 104
	{}, // 105
	{}, // 106
	{}, // 107
	{}, // 108
	{}, // 109
	{}, // 110
	{}, // 111

	// Percussive
	{}, // 112
	{}, // 113
	{}, // 114
	{}, // 115
	{}, // 116
	{}, // 117
	{}, // 118
	{}, // 119

	// Sound Effects
	{}, // 120
	{}, // 121
	{}, // 122
	{}, // 123
	{}, // 124
	{}, // 125
	{}, // 126
	{}, // 127
};
