/* sndplay plays sounds */

#include <mus-config.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "_sndlib.h"

#ifndef _MSC_VER
  #include <unistd.h>
#endif
#include <string.h>

#if __APPLE__
  #define BUFFER_SIZE 256
#else
#ifdef _MSC_VER
  /* this setting from Scott Middleton (actually used 8096) */
  #define BUFFER_SIZE 8192
#else
  #define BUFFER_SIZE 4096
#endif
#endif

#if __APPLE__
  #define OutSample float
  #define MUS_CONVERT(samp) (float)(samp)
#else
  #define OutSample short
  #define MUS_SAMPLE_TO_SHORT(n) ((short)((n) * (1 << 15)))
  #define MUS_CONVERT(samp) MUS_SAMPLE_TO_SHORT(samp)
#endif


/* 22-Nov-00:  moved alsa support to separate block */
/* 8-Apr-04:   added start/end (seconds-based) args */
/* 2-Nov-05:   added -volume arg (removed later) */
/* 22-July-08: added -mutable arg */
/* 1-May-12:   removed alsa special case */


int main(int argc, char *argv[])
{
  int fd, afd, i, j, n, k, chans, srate;
  mus_long_t frames, m;
  mus_float_t **bufs;
  OutSample *obuf;
  int buffer_size = BUFFER_SIZE, curframes, sample_size, out_chans, outbytes;
  char *name = NULL;
  mus_long_t start = 0, end = 0;
  double begin_time = 0.0, end_time = 0.0;
  int mutate = 1, include_mutate = 0;

  if (argc == 1) 
    {
      printf("usage: sndplay file [-start 1.0] [-end 1.0] [-bufsize %d] [-buffers 2x12] [-describe]\n", BUFFER_SIZE); 
      exit(0);
    }
  mus_sound_initialize();

  for (i = 1; i < argc; i++)
    {
      if (strcmp(argv[i], "-buffers") == 0) 
	{
#if (HAVE_OSS || HAVE_ALSA)
	  static char x_string[2] = {'x','\0'};
	  char *arg;
	  int a, b;
	  arg = strtok(argv[i + 1], x_string);
	  a = atoi(arg);
	  arg = strtok(NULL, x_string);
	  b = atoi(arg);
	  mus_oss_set_buffers(a, b);
#endif
	  i++;
	}
      else
	{
	  if (strcmp(argv[i], "-bufsize") == 0) 
	    {
	      buffer_size = atoi(argv[i + 1]);
	      i++;
	    }
	  else
	    {
	      if (strcmp(argv[i], "-start") == 0) 
		{
		  begin_time = atof(argv[i + 1]);
		  i++;
		}
	      else
		{
		  if (strcmp(argv[i], "-end") == 0) 
		    {
		      end_time = atof(argv[i + 1]);
		      i++;
		    }
		  else 
		    {
		      if (strcmp(argv[i], "-mutable") == 0) 
			{
			  mutate = atoi(argv[i + 1]);
			  include_mutate = 1;
			  i++;
			}
		      else name = argv[i];
		    }}}}}

  if (name == NULL) 
    {
      printf("usage: sndplay file [-start 1.0] [-end 1.0] [-bufsize %d] [-buffers 2x12] [-mutable 1]\n", BUFFER_SIZE); 
      exit(0);
    }

  afd = -1;

  if (!(mus_header_type_p(mus_sound_header_type(name))))
    {
      fprintf(stderr, "can't play %s (header type: %s?)\n",
	      name,
	      mus_header_type_name(mus_header_type()));
      exit(0);
    }

  if (!(mus_data_format_p(mus_sound_data_format(name))))
    {
      fprintf(stderr, "can't play %s (data format: %s (%s)?)\n",
	      name,
	      mus_data_format_name(mus_sound_data_format(name)),
	      mus_header_original_format_name(mus_sound_original_format(name), 
					      mus_sound_header_type(name)));
      exit(0);
    }

  fd = mus_sound_open_input(name);
  if (fd != -1)
    {
      chans = mus_sound_chans(name);
      if (chans > 2)
	{
	  int available_chans;
	  available_chans = mus_audio_device_channels(MUS_AUDIO_DEFAULT);
	  if (available_chans < chans)
	    {
	      fprintf(stderr, "%s has %d channels, but we can only handle %d\n", name, chans, available_chans);
	      exit(1);
	    }
	}

      out_chans = chans;
      srate = mus_sound_srate(name);
      frames = mus_sound_frames(name);
      sample_size = mus_bytes_per_sample(MUS_AUDIO_COMPATIBLE_FORMAT);
      start = (mus_long_t)(begin_time * srate);
      if (start > 0)
	mus_file_seek_frame(fd, start);
      if (end_time > 0.0)
	end = (mus_long_t)(end_time * srate);
      else end = frames;
      if ((end - start) < frames)
	frames = end - start;

      bufs = (mus_float_t **)calloc(chans, sizeof(mus_float_t *));
      for (i = 0; i < chans; i++) bufs[i] = (mus_float_t *)calloc(buffer_size, sizeof(mus_float_t));
      obuf = (OutSample *)calloc(buffer_size * out_chans, sizeof(OutSample));
      outbytes = buffer_size * out_chans * sample_size;

      for (m = 0; m < frames; m += buffer_size)
	{
	  if ((m + buffer_size) <= frames)
	    curframes = buffer_size;
	  else curframes = frames - m;
	  mus_file_read(fd, 0, curframes - 1, chans, bufs); 
	  /* some systems are happier if we read the file before opening the dac */
	  /* at this point the data is in separate arrays of mus_sample_t's */

	  if (chans == 1)
	    {
	      for (k = 0; k < curframes; k++) 
		obuf[k] = MUS_CONVERT(bufs[0][k]);
	    }
	  else
	    {
	      if (chans == 2)
		{
		  for (k = 0, n = 0; k < curframes; k++, n += 2) 
		    {
		      obuf[n] = MUS_CONVERT(bufs[0][k]); 
		      obuf[n + 1] = MUS_CONVERT(bufs[1][k]);
		    }
		}
	      else
		{
		  for (k = 0, j = 0; k < curframes; k++, j += chans)
		    {
		      for (n = 0; n < chans; n++) 
			obuf[j + n] = MUS_CONVERT(bufs[n][k]);
		    }
		}
	    }
#if __APPLE__
	  if (include_mutate == 1)
	    mus_audio_output_properties_mutable(mutate);
#endif
	  if (afd == -1)
	    {
	      afd = mus_audio_open_output(MUS_AUDIO_DEFAULT, srate, out_chans, MUS_AUDIO_COMPATIBLE_FORMAT, outbytes);
	      if (afd == -1) break;
	    }
	  outbytes = curframes * out_chans * sample_size;
	  mus_audio_write(afd, (char *)obuf, outbytes);
	}
      if (afd != -1) mus_audio_close(afd);
      mus_sound_close_input(fd);
      for (i = 0; i < chans; i++) free(bufs[i]);
      free(bufs);
      free(obuf);
    }
  return(0);
}
