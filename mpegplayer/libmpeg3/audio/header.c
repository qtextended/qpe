#include "mpeg3audio.h"
#include "tables.h"
#include "../libmpeg3.h"
#include "../mpeg3protos.h"

#include <stdio.h>

/* Return 1 if the head check doesn't find a header. */
int mpeg3audio_head_check(unsigned long head)
{
    if((head & 0xffe00000) != 0xffe00000) return 1;
    if(!((head >> 17) & 3)) return 1;
    if(((head >> 12) & 0xf) == 0xf) return 1;
	if(!((head >> 12) & 0xf)) return 1;
    if(((head >> 10) & 0x3) == 0x3 ) return 1;
	if(((head >> 19) & 1) == 1 && ((head >> 17) & 3) == 3 && ((head >> 16) & 1) == 1)
		return 1;
    if((head & 0xffff0000) == 0xfffe0000) return 1;

    return 0;
}

int mpeg3audio_decode_header(mpeg3audio_t *audio)
{
    if(audio->newhead & (1 << 20)) 
	{
        audio->lsf = (audio->newhead & (1 << 19)) ? 0x0 : 0x1;
        audio->mpeg35 = 0;
    }
    else 
	{
    	audio->lsf = 1;
    	audio->mpeg35 = 1;
    }

    audio->layer = 4 - ((audio->newhead >> 17) & 3);
    if(audio->mpeg35) 
        audio->sampling_frequency_code = 6 + ((audio->newhead >> 10) & 0x3);
    else
        audio->sampling_frequency_code = ((audio->newhead >> 10) & 0x3) + (audio->lsf * 3);

    audio->error_protection = ((audio->newhead >> 16) & 0x1) ^ 0x1;

    audio->bitrate_index = ((audio->newhead >> 12) & 0xf);
    audio->padding   = ((audio->newhead >> 9) & 0x1);
    audio->extension = ((audio->newhead >> 8) & 0x1);
    audio->mode      = ((audio->newhead >> 6) & 0x3);
    audio->mode_ext  = ((audio->newhead >> 4) & 0x3);
    audio->copyright = ((audio->newhead >> 3) & 0x1);
    audio->original  = ((audio->newhead >> 2) & 0x1);
    audio->emphasis  = audio->newhead & 0x3;
    audio->channels    = (audio->mode == MPG_MD_MONO) ? 1 : 2;
	if(audio->channels > 1) 
		audio->single = -1;
	else
		audio->single = 3;

	audio->prev_framesize = audio->framesize;

    if(!audio->bitrate_index) return 1;
	audio->bitrate = 1000 * mpeg3_tabsel_123[audio->lsf][audio->layer - 1][audio->bitrate_index];

    switch(audio->layer) 
	{
      	case 1:
        	audio->framesize  = (long)mpeg3_tabsel_123[audio->lsf][0][audio->bitrate_index] * 12000;
        	audio->framesize /= mpeg3_freqs[audio->sampling_frequency_code];
        	audio->framesize  = ((audio->framesize + audio->padding) << 2) - 4;
        	break;
      	case 2:
        	audio->framesize = (long)mpeg3_tabsel_123[audio->lsf][1][audio->bitrate_index] * 144000;
        	audio->framesize /= mpeg3_freqs[audio->sampling_frequency_code];
        	audio->framesize += audio->padding - 4;
        	break;
      	case 3:
        	if(audio->lsf)
        	  	audio->ssize = (audio->channels == 1) ? 9 : 17;
        	else
        	  	audio->ssize = (audio->channels == 1) ? 17 : 32;
        	if(audio->error_protection)
        	  	audio->ssize += 2;
        	audio->framesize  = (long)mpeg3_tabsel_123[audio->lsf][2][audio->bitrate_index] * 144000;
        	audio->framesize /= mpeg3_freqs[audio->sampling_frequency_code] << (audio->lsf);
        	audio->framesize = audio->framesize + audio->padding - 4;
        	break; 
      	default:
        	return 1;
    }

	if(audio->framesize > MAXFRAMESIZE) return 1;

	return 0;
}

int mpeg3audio_read_frame_body(mpeg3audio_t *audio)
{
	int i;
	for(i = 0; i < audio->framesize; i++)
	{
		audio->bsbuf[i] = mpeg3bits_getbits(audio->astream, 8);
	}
	return 0;
}

/* Seek to the start of the previous header */
int mpeg3audio_prev_header(mpeg3audio_t *audio)
{
	int result = 0, i, len = (int)audio->avg_framesize;

	for(i = 0; i < len && !result; i++)
	{
		mpeg3bits_getbits_reverse(audio->astream, 8);
	}
/* Get reading in the forward direction again. */
	result |= mpeg3bits_refill(audio->astream);
	return result;
}

/* Read the next header */
int mpeg3audio_read_header(mpeg3audio_t *audio)
{
	unsigned int code;
	int i;
	int attempt = 0;
	int result = 0;

	switch(audio->format)
	{
		case AUDIO_AC3:
			result = mpeg3audio_read_ac3_header(audio);
			break;

		case AUDIO_MPEG:
/* Layer 1 not supported */
			if(audio->layer == 1)
			{
				fprintf(stderr, "mpeg3audio_new: layer 1 not supported\n");
				result = 1;
			}
			audio->newhead = mpeg3bits_showbits(audio->astream, 32);
			if(!mpeg3bits_eof(audio->astream) &&
				(mpeg3audio_head_check(audio->newhead) || mpeg3audio_decode_header(audio)))
			{
				do
				{
					attempt++;
					mpeg3bits_getbyte_noptr(audio->astream);
					audio->newhead = mpeg3bits_showbits(audio->astream, 32);
				}while(!mpeg3bits_eof(audio->astream) && 
					attempt < 65536 && 
					(mpeg3audio_head_check(audio->newhead) || mpeg3audio_decode_header(audio)));
    		}

/* Skip the 4 bytes containing the header */
			mpeg3bits_getbits(audio->astream, 32);
			break;
		
		case AUDIO_PCM:
			mpeg3audio_read_pcm_header(audio);
			break;
	}
	return mpeg3bits_eof(audio->astream);
}
