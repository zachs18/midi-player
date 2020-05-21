#ifndef PERCUSSION_H_INCLUDED
#define PERCUSSION_H_INCLUDED

struct percussion {
	const short *start;
	const short *end;
};

extern const struct percussion percussions[128];

#endif // ndef PERCUSSION_H_INCLUDED
