#include <mus-config.h>

#if USE_SND
  #include "snd.h"
#endif

#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <stdarg.h>

#ifndef _MSC_VER
  #include <unistd.h>
#else
  #include <io.h>
  #pragma warning(disable: 4244)
#endif
#include <string.h>

#include "_sndlib.h"
#include "sndlib-strings.h"


static mus_error_handler_t *mus_error_handler = NULL;

mus_error_handler_t *mus_error_set_handler(mus_error_handler_t *new_error_handler)
{
  mus_error_handler_t *old_handler;
  old_handler = mus_error_handler;
  mus_error_handler = new_error_handler;
  return(old_handler);
}


static char *mus_error_buffer = NULL;
static int mus_error_buffer_size = 1024;

int mus_error(int error, const char *format, ...)
{
  int bytes_needed = 0;
  va_list ap;

  if (format == NULL) 
    return(MUS_ERROR); /* else bus error in Mac OSX */

  if (mus_error_buffer == NULL)
    mus_error_buffer = (char *)calloc(mus_error_buffer_size, sizeof(char));

  va_start(ap, format);

  /* can't use vasprintf here or below because the error handler may jump anywhere,
   *   leaving unfreed memory behind.
   */
  bytes_needed = vsnprintf(mus_error_buffer, mus_error_buffer_size, format, ap);
  va_end(ap);

  if (bytes_needed >= mus_error_buffer_size)
    {
      mus_error_buffer_size = bytes_needed * 2;
      free(mus_error_buffer);
      mus_error_buffer = (char *)calloc(mus_error_buffer_size, sizeof(char));

      va_start(ap, format);
      vsnprintf(mus_error_buffer, mus_error_buffer_size, format, ap);
      va_end(ap);
    }

  if (mus_error_handler)
    (*mus_error_handler)(error, mus_error_buffer);
  else 
    {
      fprintf(stderr, "%s", mus_error_buffer);
      fputc('\n', stderr);
    }

  return(MUS_ERROR);
}


static mus_print_handler_t *mus_print_handler = NULL;

mus_print_handler_t *mus_print_set_handler(mus_print_handler_t *new_print_handler) 
{
  mus_print_handler_t *old_handler;
  old_handler = mus_print_handler;
  mus_print_handler = new_print_handler;
  return(old_handler);
}


void mus_print(const char *format, ...)
{
  va_list ap;

  if (mus_print_handler)
    {
      int bytes_needed = 0;

      if (mus_error_buffer == NULL)
	mus_error_buffer = (char *)calloc(mus_error_buffer_size, sizeof(char));

      va_start(ap, format);
      bytes_needed = vsnprintf(mus_error_buffer, mus_error_buffer_size, format, ap);
      va_end(ap);

      if (bytes_needed >= mus_error_buffer_size)
	{
	  mus_error_buffer_size = bytes_needed * 2;
	  free(mus_error_buffer);
	  mus_error_buffer = (char *)calloc(mus_error_buffer_size, sizeof(char));

	  va_start(ap, format);
	  vsnprintf(mus_error_buffer, mus_error_buffer_size, format, ap);
	  va_end(ap);
	}

      (*mus_print_handler)(mus_error_buffer);
    }
  else
    {
      va_start(ap, format);
      vfprintf(stdout, format, ap);
      va_end(ap);
    }
}


static const char *mus_initial_error_names[MUS_INITIAL_ERROR_TAG] = {
  "no error", "no frequency method", "no phase method", "null gen arg to method", "no length method",
  "no free method", "no describe method", "no data method", "no scaler method",
  "memory allocation failed", 
  "can't open file", "no sample input", "no sample output",
  "no such channel", "no file name provided", "no location method", "no channel method",
  "no such fft window", "unsupported data format", "header read failed",
  "unsupported header type", "file descriptors not initialized", "not a sound file", "file closed", "write error",
  "header write failed", "can't open temp file", "interrupted", "bad envelope",

  "audio channels not available", "audio srate not available", "audio format not available",
  "no audio input available", "audio configuration not available", 
  "audio write error", "audio size not available", "audio device not available",
  "can't close audio", "can't open audio", "audio read error",
  "can't write audio", "can't read audio", "no audio read permission", 
  "can't close file", "arg out of range", 

  "no channels method", "no hop method", "no width method", "no file-name method", "no ramp method", "no run method",
  "no increment method", "no offset method",
  "no xcoeff method", "no ycoeff method", "no xcoeffs method", "no ycoeffs method", "no reset", "bad size", "can't convert",
  "read error",
  "no feedforward method", "no feedback method", "no interp-type method", "no position method", "no order method",
};

static char **mus_error_names = NULL;
static int mus_error_names_size = 0;
static int mus_error_tag = MUS_INITIAL_ERROR_TAG;

int mus_make_error(const char *error_name) 
{
  int new_error, err;
  new_error = mus_error_tag++;
  err = new_error - MUS_INITIAL_ERROR_TAG;
  if (error_name)
    {
      int len, i;
      if (err >= mus_error_names_size)
	{
	  if (mus_error_names_size == 0)
	    {
	      mus_error_names_size = 8;
	      mus_error_names = (char **)calloc(mus_error_names_size, sizeof(char *));
	    }
	  else
	    {
	      len = mus_error_names_size;
	      mus_error_names_size += 8;
	      mus_error_names = (char **)realloc(mus_error_names, mus_error_names_size * sizeof(char *));
	      for (i = len; i < mus_error_names_size; i++) mus_error_names[i] = NULL;
	    }
	}
      len = strlen(error_name);
      mus_error_names[err] = (char *)calloc(len + 1, sizeof(char));
      strcpy(mus_error_names[err], error_name);
    }
  return(new_error);
}


const char *mus_error_type_to_string(int err)
{
  if (err >= 0)
    {
      if (err < MUS_INITIAL_ERROR_TAG)
	return(mus_initial_error_names[err]);
      else
	{
	  err -= MUS_INITIAL_ERROR_TAG;
	  if ((mus_error_names) && (err < mus_error_names_size))
	    return(mus_error_names[err]);
	}
    }
  return("unknown mus error");
}


static void default_mus_error(int ignore, char *msg)
{
  /* default error handler simply prints the error message */
  fprintf(stderr, "%s", msg);
}


static time_t local_file_write_date(const char *filename)
{
  struct stat statbuf;
  int err;
  err = stat(filename, &statbuf);
  if (err < 0) return((time_t)0);
  return((time_t)(statbuf.st_mtime));
}



/* -------- sound file table -------- */

typedef struct {
  char *file_name;  /* full path -- everything is keyed to this name */
  int table_pos, file_name_length;
  mus_long_t *aux_comment_start, *aux_comment_end;
  int *loop_modes, *loop_starts, *loop_ends;
  int markers, base_detune, base_note;
  int *marker_ids, *marker_positions;
  mus_long_t samples, true_file_length;
  mus_long_t data_location;
  int srate, chans, header_type, data_format, original_sound_format, datum_size; 
  mus_long_t comment_start, comment_end;
  int type_specifier, bits_per_sample, block_align, fact_samples;
  time_t write_date;
  mus_float_t *maxamps;
  mus_long_t *maxtimes;
  int maxamps_size; /* we can't depend on sf->chans here because the user could set chans to some bogus value */
} sound_file;

static int sound_table_size = 0, sound_table_size4 = 0;
static sound_file **sound_table = NULL;

static void free_sound_file(sound_file *sf)
{
  if (sf)
    {
      sound_table[sf->table_pos] = NULL;
      if (sf->aux_comment_start) free(sf->aux_comment_start);
      if (sf->aux_comment_end) free(sf->aux_comment_end);
      if (sf->file_name) free(sf->file_name);
      if (sf->loop_modes) free(sf->loop_modes);
      if (sf->loop_starts) free(sf->loop_starts);
      if (sf->loop_ends) free(sf->loop_ends);
      if (sf->marker_ids) free(sf->marker_ids);
      if (sf->marker_positions) free(sf->marker_positions);
      if (sf->maxamps) free(sf->maxamps);
      if (sf->maxtimes) free(sf->maxtimes);
      sf->maxamps_size = 0;
      free(sf);
    }
}

static sound_file *add_to_sound_table(const char *name)
{
  int i, len, pos = -1;

  for (i = 0; i < sound_table_size; i++)
    if (sound_table[i] == NULL) 
      {
	pos = i;
	break;
      }

  if (pos == -1)
    {
      pos = sound_table_size;
      sound_table_size += 16;
      sound_table_size4 += 16;
      if (sound_table == NULL)
	{
	  sound_table_size4 -= 4;
	  sound_table = (sound_file **)calloc(sound_table_size, sizeof(sound_file *));
	}
      else 
	{
	  sound_table = (sound_file **)realloc(sound_table, sound_table_size * sizeof(sound_file *));
	  for (i = pos; i < sound_table_size; i++) sound_table[i] = NULL;
	}
    }

  sound_table[pos] = (sound_file *)calloc(1, sizeof(sound_file));
  sound_table[pos]->table_pos = pos;
  len = strlen(name);
  sound_table[pos]->file_name = (char *)calloc(len + 1, sizeof(char));
  strcpy(sound_table[pos]->file_name, name);
  sound_table[pos]->file_name_length = len;

  return(sound_table[pos]);
}


int mus_sound_prune(void)
{
  int i, pruned = 0;

  for (i = 0; i < sound_table_size; i++)
    if ((sound_table[i]) && 
	(!(mus_file_probe(sound_table[i]->file_name))))
      {
	free_sound_file(sound_table[i]);
	sound_table[i] = NULL;
	pruned++;
      }

  return(pruned);
}


int mus_sound_forget(const char *name)
{
  int i, len, len2, short_len = 0;
  bool free_name = false;
  char *short_name = NULL;

  if (name == NULL) return(MUS_ERROR);
  len = strlen(name);
  if (len > 6)
    len2 = len - 6;
  else len2 = len / 2;

  if (name[0] == '/')
    {
      for (i = 0; i < len; i++)
	if (name[i] == '/')
	  short_name = (char *)(name + i + 1);
    }
  else
    {
      short_name = mus_expand_filename(name);
      free_name = true;
    }
  if (short_name) 
    short_len = strlen(short_name);

  if (name)
    {
      for (i = 0; i < sound_table_size; i++)
	if ((sound_table[i]) &&
	    (((sound_table[i]->file_name_length == len) &&
	      (sound_table[i]->file_name[len2] == name[len2]) &&
	      (strcmp(name, sound_table[i]->file_name) == 0)) ||
	     ((short_name) && 
	      (sound_table[i]->file_name_length == short_len) &&
	      (strcmp(short_name, sound_table[i]->file_name) == 0))))
	  {
	    free_sound_file(sound_table[i]);
	    sound_table[i] = NULL;
	  }
    }
  
  if (free_name) free(short_name);
  return(MUS_NO_ERROR);
}


static sound_file *check_write_date(const char *name, sound_file *sf)
{
  if (sf)
    {
      time_t date;
      date = local_file_write_date(name);

      if (date == sf->write_date)
	return(sf);

      if ((sf->header_type == MUS_RAW) && (mus_header_no_header(name)))
	{
	  int chan;
	  mus_long_t data_size;
	  /* sound has changed since we last read it, but it has no header, so
	   * the only sensible thing to check is the new length (i.e. caller
	   * has set other fields by hand)
	   */
	  sf->write_date = date;
	  chan = mus_file_open_read(name);
	  data_size = lseek(chan, 0L, SEEK_END);
	  sf->true_file_length = data_size;
	  sf->samples = mus_bytes_to_samples(sf->data_format, data_size);
	  CLOSE(chan, name);  
	  return(sf);
	}
      /* otherwise our data base is out-of-date, so clear it out */
      free_sound_file(sf);
    }
  return(NULL);
}


static sound_file *find_sound_file(const char *name)
{
  int i, len, len2;
  sound_file *sf;

  if ((!name) ||
      (sound_table_size == 0))
    return(NULL);

  len = strlen(name);
  if (len > 6)
    len2 = len - 6; /* the names probably all start with '/' and end with ".snd", so try to find a changing character... */
  else len2 = len / 2;
  i = 0;
  while (i <= sound_table_size4)
    {
      sf = sound_table[i];
      if ((sf) &&
	  (sf->file_name_length == len) &&
	  (name[len2] == sf->file_name[len2]) &&
	  (strcmp(name, sf->file_name) == 0))
	return(check_write_date(name, sf));
      i++;

      sf = sound_table[i];
      if ((sf) &&
	  (sf->file_name_length == len) &&
	  (name[len2] == sf->file_name[len2]) &&
	  (strcmp(name, sf->file_name) == 0))
	return(check_write_date(name, sf));
      i++;

      sf = sound_table[i];
      if ((sf) &&
	  (sf->file_name_length == len) &&
	  (name[len2] == sf->file_name[len2]) &&
	  (strcmp(name, sf->file_name) == 0))
	return(check_write_date(name, sf));
      i++;

      sf = sound_table[i];
      if ((sf) &&
	  (sf->file_name_length == len) &&
	  (name[len2] == sf->file_name[len2]) &&
	  (strcmp(name, sf->file_name) == 0))
	return(check_write_date(name, sf));
      i++;
    }
  for (; i < sound_table_size; i++)
    {
      sf = sound_table[i];
      if ((sf) &&
	  (sf->file_name_length == len) &&
	  (name[len2] == sf->file_name[len2]) &&
	  (strcmp(name, sf->file_name) == 0))
	return(check_write_date(name, sf));
    }
  return(NULL);
}


static void display_sound_file_entry(FILE *fp, const char *name, sound_file *sf)
{
  #define TIME_BUFFER_SIZE 64
  int i, lim;
  time_t date;
  char timestr[TIME_BUFFER_SIZE];
  char *comment;

  date = sf->write_date;
  if (date != 0)
    strftime(timestr, TIME_BUFFER_SIZE, "%a %d-%b-%Y %H:%M:%S", localtime(&date));
  else snprintf(timestr, TIME_BUFFER_SIZE, "(date cleared)");

  fprintf(fp, "  %s: %s, chans: %d, srate: %d, type: %s, format: %s, samps: %lld",
	  name,
	  timestr,
	  sf->chans,
	  sf->srate,
	  mus_header_type_name(sf->header_type),
	  mus_data_format_name(sf->data_format),
	  sf->samples);

  if (sf->loop_modes)
    {
      if (sf->loop_modes[0] != 0)
	fprintf(fp, ", loop mode %d: %d to %d", sf->loop_modes[0], sf->loop_starts[0], sf->loop_ends[0]);
      if (sf->loop_modes[1] != 0)
	fprintf(fp, ", loop mode %d: %d to %d, ", sf->loop_modes[1], sf->loop_starts[1], sf->loop_ends[1]);
      fprintf(fp, ", base: %d, detune: %d", sf->base_note, sf->base_detune);
    }

  if (sf->maxamps)
    {
      lim = sf->maxamps_size;
      if (lim > 0)
	{
	  if (lim > 64) 
	    lim = 64;
	  for (i = 0; i < lim; i++)
	    {
	      if (i > 1) fprintf(fp, ", ");
	      fprintf(fp, " %.3f at %.3f ",
		      sf->maxamps[i],
		      (sf->srate > 0) ? (float)((double)(sf->maxtimes[i]) / (double)(sf->srate)) : (float)(sf->maxtimes[i]));
	    }
	}
    }

  if (mus_file_probe(name))
    {
      comment = mus_sound_comment(name);
      if (comment)
	{
	  fprintf(fp, "\n      comment: %s", comment);
	  free(comment);
	}
    }
  else fprintf(fp, " [defunct]");
  fprintf(fp, "\n");
}


void mus_sound_report_cache(FILE *fp)
{
  sound_file *sf;
  int entries = 0;
  int i;
  fprintf(fp, "sound table:\n");
  for (i = 0; i < sound_table_size; i++)
    {
      sf = sound_table[i];
      if (sf) 
	{
	  display_sound_file_entry(fp, sf->file_name, sf);
	  entries++;
	}
    }
  fprintf(fp, "\nentries: %d\n", entries); 
  fflush(fp);
}


static sound_file *fill_sf_record(const char *name, sound_file *sf)
{
  int i;

  sf->data_location = mus_header_data_location();
  sf->samples = mus_header_samples();
  sf->data_format = mus_header_format();
  sf->srate = mus_header_srate();
  /* if (sf->srate < 0) sf->srate = 0; */
  sf->chans = mus_header_chans();
  /* if (sf->chans < 0) sf->chans = 0; */
  sf->datum_size = mus_bytes_per_sample(sf->data_format);
  sf->header_type = mus_header_type();
  sf->original_sound_format = mus_header_original_format();
  sf->true_file_length = mus_header_true_length();

  sf->comment_start = mus_header_comment_start();
  sf->comment_end = mus_header_comment_end();
  if (((sf->header_type == MUS_AIFC) || 
       (sf->header_type == MUS_AIFF) || 
       (sf->header_type == MUS_RF64) || 
       (sf->header_type == MUS_RIFF)) &&
      (mus_header_aux_comment_start(0) != 0))

    {
      sf->aux_comment_start = (mus_long_t *)calloc(4, sizeof(mus_long_t));
      sf->aux_comment_end = (mus_long_t *)calloc(4, sizeof(mus_long_t));
      for (i = 0; i < 4; i++)
	{
	  sf->aux_comment_start[i] = mus_header_aux_comment_start(i);
	  sf->aux_comment_end[i] = mus_header_aux_comment_end(i);
	}
    }

  sf->type_specifier = mus_header_type_specifier();
  sf->bits_per_sample = mus_header_bits_per_sample();
  sf->fact_samples = mus_header_fact_samples();
  sf->block_align = mus_header_block_align();
  sf->write_date = local_file_write_date(name);

  if ((sf->header_type == MUS_AIFF) || (sf->header_type == MUS_AIFC))
    {
      int *marker_ids, *marker_positions;
      sf->markers = mus_header_mark_info(&marker_ids, &marker_positions);
      if (sf->markers > 0)
	{
	  sf->marker_ids = (int *)malloc(sf->markers * sizeof(int));
	  sf->marker_positions = (int *)malloc(sf->markers * sizeof(int));
	  memcpy((void *)(sf->marker_ids), (void *)marker_ids, sizeof(int) * sf->markers);
	  memcpy((void *)(sf->marker_positions), (void *)marker_positions, sizeof(int) * sf->markers);
	}
    }

  if (mus_header_loop_mode(0) > 0)
    {
      sf->loop_modes = (int *)calloc(2, sizeof(int));
      sf->loop_starts = (int *)calloc(2, sizeof(int));
      sf->loop_ends = (int *)calloc(2, sizeof(int));
      for (i = 0; i < 2; i++)
	{
	  sf->loop_modes[i] = mus_header_loop_mode(i);
	  if ((sf->header_type == MUS_AIFF) || 
	      (sf->header_type == MUS_AIFC))
	    {
	      sf->loop_starts[i] = mus_header_mark_position(mus_header_loop_start(i)); 
	      sf->loop_ends[i] = mus_header_mark_position(mus_header_loop_end(i));
	    }
	  else
	    {
	      sf->loop_starts[i] = mus_header_loop_start(i); 
	      sf->loop_ends[i] = mus_header_loop_end(i);
	    }
	}
      sf->base_detune = mus_header_base_detune();
      sf->base_note = mus_header_base_note();
    }

  return(sf);
}


static sound_file *read_sound_file_header(const char *name) /* 2 calls on this: mus_sound_open_input and get_sf */
{
  int result;
  mus_sound_initialize();
  result = mus_header_read(name);
  /* this portion won't trigger mus_error */
  if (result != MUS_ERROR)
    return(fill_sf_record(name, add_to_sound_table(name))); /* only call on fill_sf_record and add_to_sound_table */
  return(NULL);
}


static sound_file *get_sf(const char *arg) 
{
  sound_file *sf = NULL;
  if (arg == NULL) return(NULL);
  sf = find_sound_file(arg);
  if (sf) return(sf);
  return(read_sound_file_header(arg));
}


mus_long_t mus_sound_samples(const char *arg)       
{
  sound_file *sf;
  mus_long_t result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->samples;
  return(result);
}


mus_long_t mus_sound_frames(const char *arg)        
{
  sound_file *sf;
  mus_long_t result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = (sf->chans > 0) ? (sf->samples / sf->chans) : 0;
  return(result);
}


int mus_sound_datum_size(const char *arg)      
{
  sound_file *sf;
  int result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->datum_size;
  return(result);
}


mus_long_t mus_sound_data_location(const char *arg) 
{
  sound_file *sf;
  mus_long_t result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->data_location;
  return(result);
}


int mus_sound_chans(const char *arg)           
{
  sound_file *sf;
  int result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->chans;
  return(result);
}


int mus_sound_srate(const char *arg)           
{
  sound_file *sf;
  int result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->srate;
  return(result);
}


int mus_sound_header_type(const char *arg)     
{
  sound_file *sf;
  int result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->header_type;
  return(result);
}


int mus_sound_data_format(const char *arg)     
{
  sound_file *sf;
  int result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->data_format;
  return(result);
}


int mus_sound_original_format(const char *arg) 
{
  sound_file *sf;
  int result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->original_sound_format;
  return(result);
}


mus_long_t mus_sound_comment_start(const char *arg) 
{
  sound_file *sf;
  mus_long_t result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->comment_start;
  return(result);
}


mus_long_t mus_sound_comment_end(const char *arg)   
{
  sound_file *sf;
  mus_long_t result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->comment_end;
  return(result);
}


mus_long_t mus_sound_length(const char *arg)        
{
  sound_file *sf;
  mus_long_t result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->true_file_length;
  return(result);
}


int mus_sound_fact_samples(const char *arg)    
{
  sound_file *sf;
  int result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->fact_samples;
  return(result);
}


time_t mus_sound_write_date(const char *arg)   
{
  sound_file *sf;
  time_t result = (time_t)(MUS_ERROR);
  sf = get_sf(arg);
  if (sf) result = sf->write_date;
  return(result);
}


int mus_sound_type_specifier(const char *arg)  
{
  sound_file *sf;
  int result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->type_specifier;
  return(result);
}


int mus_sound_block_align(const char *arg)     
{
  sound_file *sf;
  int result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->block_align;
  return(result);
}


int mus_sound_bits_per_sample(const char *arg) 
{
  sound_file *sf;
  int result = MUS_ERROR;
  sf = get_sf(arg);
  if (sf) result = sf->bits_per_sample;
  return(result);
}


float mus_sound_duration(const char *arg) 
{
  float val = -1.0;
  sound_file *sf; 
  sf = get_sf(arg); 
  if (sf) 
    {
      if ((sf->chans > 0) && (sf->srate > 0))
	val = (float)((double)(sf->samples) / ((float)(sf->chans) * (float)(sf->srate)));
      else val = 0.0;
    }
  return(val);
}


int *mus_sound_loop_info(const char *arg)
{
  sound_file *sf; 
  sf = get_sf(arg); 
  if ((sf) && (sf->loop_modes))
    {
      int *info;
      info = (int *)calloc(MUS_LOOP_INFO_SIZE, sizeof(int));
      if (sf->loop_modes[0] != 0)
	{
	  info[0] = sf->loop_starts[0];
	  info[1] = sf->loop_ends[0];
	  info[6] = sf->loop_modes[0];
	}
      if (sf->loop_modes[1] != 0)
	{
	  info[2] = sf->loop_starts[1];
	  info[3] = sf->loop_ends[1];
	  info[7] = sf->loop_modes[1];
	}
      info[4] = sf->base_note;
      info[5] = sf->base_detune;
      return(info);
    }
  return(NULL);
}


void mus_sound_set_loop_info(const char *arg, int *loop)
{
  sound_file *sf; 
  sf = get_sf(arg); 
  if (sf)
    {
      if (sf->loop_modes == NULL)
	{
	  sf->loop_modes = (int *)calloc(2, sizeof(int));
	  sf->loop_starts = (int *)calloc(2, sizeof(int));
	  sf->loop_ends = (int *)calloc(2, sizeof(int));
	}
      sf->loop_modes[0] = loop[6]; 
      if (loop[6] != 0)
	{
	  sf->loop_starts[0] = loop[0];
	  sf->loop_ends[0] = loop[1];
	}
      else
	{
	  sf->loop_starts[0] = 0;
	  sf->loop_ends[0] = 0;
	}
      sf->loop_modes[1] = loop[7];
      if (loop[7] != 0)
	{
	  sf->loop_starts[1] = loop[2];
	  sf->loop_ends[1] = loop[3];
	}
      else
	{
	  sf->loop_starts[1] = 0;
	  sf->loop_ends[1] = 0;
	}
      sf->base_note = loop[4];
      sf->base_detune = loop[5];
    }
}


int mus_sound_mark_info(const char *arg, int **mark_ids, int **mark_positions)
{
  sound_file *sf; 
  int result = 0;
  sf = get_sf(arg); 
  if (sf)
    {
      (*mark_ids) = sf->marker_ids;
      (*mark_positions) = sf->marker_positions;
      result = sf->markers;
    }
  return(result);
}


char *mus_sound_comment(const char *name)
{
  mus_long_t start, end, len;
  char *sc = NULL;
  sound_file *sf = NULL;

  sf = get_sf(name); 
  if (sf)
    {
      start = sf->comment_start;
      end = sf->comment_end;
      if (end == 0) 
	{
	  if (sf->aux_comment_start)
	    {
	      if ((sf->header_type == MUS_RIFF) ||
		  (sf->header_type == MUS_RF64))
		sc = mus_header_riff_aux_comment(name, 
						 sf->aux_comment_start, 
						 sf->aux_comment_end);
	      if ((sf->header_type == MUS_AIFF) || 
		  (sf->header_type == MUS_AIFC)) 
		sc = mus_header_aiff_aux_comment(name, 
						 sf->aux_comment_start, 
						 sf->aux_comment_end);
	    }
	}
      else
	{
	  if (end <= sf->true_file_length)
	    {
	      len = end - start + 1;
	      if (len > 0)
		{
		  /* open and get the comment */
		  ssize_t bytes;
		  int fd;
		  char *auxcom;
		  fd = mus_file_open_read(name);
		  if (fd == -1) return(NULL);
		  lseek(fd, start, SEEK_SET);
		  sc = (char *)calloc(len + 1, sizeof(char));
		  bytes = read(fd, sc, len);
		  CLOSE(fd, name);
		  if (((sf->header_type == MUS_AIFF) || 
		       (sf->header_type == MUS_AIFC)) &&
		      (sf->aux_comment_start) &&
		      (bytes != 0))
		    {
		      auxcom = mus_header_aiff_aux_comment(name, 
							   sf->aux_comment_start, 
							   sf->aux_comment_end);
		      if (auxcom)
			{
			  size_t full_len;
			  full_len = strlen(auxcom) + strlen(sc) + 2;
			  sc = (char *)realloc(sc, full_len * sizeof(char));
			  strcat(sc, "\n");
			  strcat(sc, auxcom);
			}
		    }
		}
	    }
	}
    }
  return(sc);
}


int mus_sound_open_input(const char *arg) 
{
  int fd = -1;
  if (!(mus_file_probe(arg)))
    mus_error(MUS_CANT_OPEN_FILE, S_mus_sound_open_input " can't open %s: %s", arg, STRERROR(errno));
  else
    {
      sound_file *sf = NULL;
      mus_sound_initialize();
      sf = get_sf(arg);
      if (sf) 
	{
	  fd = mus_file_open_read(arg);
	  if (fd == -1)
	    mus_error(MUS_CANT_OPEN_FILE, S_mus_sound_open_input " can't open %s: %s", arg, STRERROR(errno));
	  else
	    {
	      mus_file_open_descriptors(fd, arg, sf->data_format, sf->datum_size, sf->data_location, sf->chans, sf->header_type);
	      lseek(fd, sf->data_location, SEEK_SET);
	    }
	}
    }
  return(fd);
}


int mus_sound_open_output(const char *arg, int srate, int chans, int data_format, int header_type, const char *comment)
{
  int fd = MUS_ERROR, err;
  mus_sound_initialize();
  mus_sound_forget(arg);
  err = mus_write_header(arg, header_type, srate, chans, 0, data_format, comment);
  if (err != MUS_ERROR)
    {
      fd = mus_file_open_write(arg);
      if (fd != -1)
	mus_file_open_descriptors(fd,
				  arg,
				  data_format,
				  mus_bytes_per_sample(data_format),
				  mus_header_data_location(),
				  chans,
				  header_type);
    }
  return(fd);
}


int mus_sound_reopen_output(const char *arg, int chans, int format, int type, mus_long_t data_loc)
{
  int fd;
  mus_sound_initialize();
  fd = mus_file_reopen_write(arg);
  if (fd != -1)
    mus_file_open_descriptors(fd, arg, format, mus_bytes_per_sample(format), data_loc, chans, type);
  return(fd);
}


int mus_sound_close_input(int fd) 
{
  return(mus_file_close(fd)); /* this closes the clm file descriptors */
}


int mus_sound_close_output(int fd, mus_long_t bytes_of_data) 
{
  char *name;
  name = mus_file_fd_name(fd);
  if (name)
    {
      int err = MUS_ERROR, old_type;
      char *fname;
      fname = mus_strdup(name); 
      old_type = mus_file_header_type(fd);
      err = mus_file_close(fd);        /* this frees the original fd->name, so we copied above */
      /* fd is NULL now */
      mus_sound_forget(fname);
      mus_header_change_data_size(fname, old_type, bytes_of_data);
      free(fname);
      return(err);
    }
  return(MUS_ERROR);
}


typedef enum {SF_CHANS, SF_SRATE, SF_TYPE, SF_FORMAT, SF_LOCATION, SF_SIZE} sf_field_t;

static int mus_sound_set_field(const char *arg, sf_field_t field, int val)
{
  sound_file *sf;
  int result = MUS_NO_ERROR;

  sf = get_sf(arg); 
  if (sf) 
    {
      switch (field)
	{
	case SF_CHANS:    
	  sf->chans = val;       
	  break;

	case SF_SRATE:    
	  sf->srate = val;       
	  break;

	case SF_TYPE:     
	  sf->header_type = val; 
	  break;

	case SF_FORMAT:   
	  sf->data_format = val; 
	  sf->datum_size = mus_bytes_per_sample(val); 
	  break;

	default: 
	  result = MUS_ERROR; 
	  break;
	}
    }
  else result = MUS_ERROR;
  return(result);
}


static int mus_sound_set_mus_long_t_field(const char *arg, sf_field_t field, mus_long_t val)
{
  sound_file *sf; 
  int result = MUS_NO_ERROR;

  sf = get_sf(arg); 
  if (sf) 
    {
      switch (field)
	{
	case SF_SIZE:     
	  sf->samples = val; 
	  break;

	case SF_LOCATION: 
	  sf->data_location = val; 
	  break;

	default: 
	  result = MUS_ERROR;
	  break;
	}
    }
  else result = MUS_ERROR;

  return(result);
}


int mus_sound_set_chans(const char *arg, int val)           {return(mus_sound_set_field(arg,       SF_CHANS,    val));}
int mus_sound_set_srate(const char *arg, int val)           {return(mus_sound_set_field(arg,       SF_SRATE,    val));}
int mus_sound_set_header_type(const char *arg, int val)     {return(mus_sound_set_field(arg,       SF_TYPE,     val));}
int mus_sound_set_data_format(const char *arg, int val)     {return(mus_sound_set_field(arg,       SF_FORMAT,   val));}
int mus_sound_set_data_location(const char *arg, mus_long_t val) {return(mus_sound_set_mus_long_t_field(arg, SF_LOCATION, val));}
int mus_sound_set_samples(const char *arg, mus_long_t val)       {return(mus_sound_set_mus_long_t_field(arg, SF_SIZE,     val));}


int mus_sound_override_header(const char *arg, int srate, int chans, int format, int type, mus_long_t location, mus_long_t size)
{
  sound_file *sf; 
  int result = MUS_NO_ERROR;
  /* perhaps once a header has been over-ridden, we should not reset the relevant fields upon re-read? */

  sf = get_sf(arg); 
  if (sf)
    {
      if (location != -1) sf->data_location = location;
      if (size != -1) sf->samples = size;
      if (format != -1) 
	{
	  sf->data_format = format;
	  sf->datum_size = mus_bytes_per_sample(format);
	}
      if (srate != -1) sf->srate = srate;
      if (chans != -1) sf->chans = chans;
      if (type != -1) sf->header_type = type;
    }
  else result = MUS_ERROR;

  return(result);
}


bool mus_sound_maxamp_exists(const char *ifile)
{
  sound_file *sf; 
  sf = get_sf(ifile); 
  if ((sf) && (sf->maxtimes))
    {
      int i;
      for (i = 0; i < sf->maxamps_size; i++)
	if (sf->maxtimes[i] == -1)
	  return(false);
      return(true);
    }
  return(false);
}


mus_long_t mus_sound_maxamps(const char *ifile, int chans, mus_float_t *vals, mus_long_t *times)
{
  mus_long_t frames;
  int i, ichans, chn;
  sound_file *sf; 
    
  sf = get_sf(ifile); 
  if (sf->chans <= 0)
    return(MUS_ERROR);
  
  if ((sf) && (sf->maxamps))
    {
      if (chans > sf->maxamps_size) 
	ichans = sf->maxamps_size; 
      else ichans = chans;
      for (chn = 0; chn < ichans; chn++)
	{
	  times[chn] = sf->maxtimes[chn];
	  vals[chn] = sf->maxamps[chn];
	}
      frames = sf->samples / sf->chans;
      return(frames);
    }

  {
    int j, bufnum, ifd;
    mus_long_t n, curframes;
    mus_float_t *buffer, *samp;
    mus_long_t *time;
    mus_float_t **ibufs;

    ifd = mus_sound_open_input(ifile);
    if (ifd == MUS_ERROR) return(MUS_ERROR);
    ichans = mus_sound_chans(ifile);
    frames = mus_sound_frames(ifile);
    if (frames == 0) 
      {
	mus_sound_close_input(ifd);
	return(0);
      }

    mus_file_seek_frame(ifd, 0);

    ibufs = (mus_float_t **)calloc(ichans, sizeof(mus_float_t *));
    bufnum = 8192;
    for (j = 0; j < ichans; j++) 
      ibufs[j] = (mus_float_t *)calloc(bufnum, sizeof(mus_float_t));

    time = (mus_long_t *)calloc(ichans, sizeof(mus_long_t));
    samp = (mus_float_t *)calloc(ichans, sizeof(mus_float_t));

    for (n = 0; n < frames; n += bufnum)
      {
	if ((n + bufnum) < frames) 
	  curframes = bufnum; 
	else curframes = (frames - n);
	mus_file_read(ifd, 0, curframes - 1, ichans, ibufs);
	for (chn = 0; chn < ichans; chn++)
	  {
	    buffer = (mus_float_t *)(ibufs[chn]);
	    for (i = 0; i < curframes; i++) 
	      {
		mus_float_t abs_samp;
		abs_samp = fabs(buffer[i]);
		if (abs_samp > samp[chn])
		  {
		    time[chn] = i + n; 
		    samp[chn] = abs_samp;
		  }
	      }
	  }
      }

    mus_sound_close_input(ifd);
    mus_sound_set_maxamps(ifile, ichans, samp, time); /* save the complete set */

    if (ichans > chans) ichans = chans;
    for (chn = 0; chn < ichans; chn++)
      {
	times[chn] = time[chn];
	vals[chn] = samp[chn];
      }
    free(time);
    free(samp);
    for (j = 0; j < ichans; j++) free(ibufs[j]);
    free(ibufs);
    return(frames);
  }
}


int mus_sound_set_maxamps(const char *ifile, int chans, mus_float_t *vals, mus_long_t *times)
{
  sound_file *sf; 
  int result = MUS_NO_ERROR;

  sf = get_sf(ifile); 
  if (sf)
    {
      int i, ichans = 0;

      if (sf->maxamps)
	{
	  if (chans > sf->maxamps_size) 
	    ichans = sf->maxamps_size; 
	  else ichans = chans;
	  for (i = 0; i < ichans; i++)
	    {
	      sf->maxtimes[i] = times[i];
	      sf->maxamps[i] = vals[i];
	    }
	}
      else
	{
	  ichans = sf->chans;
	  if (sf->maxamps == NULL) 
	    {
	      /* here we need to use the max, since the caller may be confused */
	      int max_chans;
	      max_chans = ichans;
	      if (max_chans < chans)
		max_chans = chans;

	      sf->maxamps = (mus_float_t *)calloc(max_chans, sizeof(mus_float_t));
	      sf->maxtimes = (mus_long_t *)calloc(max_chans, sizeof(mus_long_t));
	      sf->maxamps_size = max_chans;
	    }
	  if (ichans > sf->maxamps_size)
	    ichans = sf->maxamps_size;
	  
	  if (ichans > chans) 
	    ichans = chans;
	  for (i = 0; i < ichans; i++)
	    {
	      sf->maxtimes[i] = times[i];
	      sf->maxamps[i] = vals[i];
	    }
	}
    }
  else result = MUS_ERROR;
  return(result);
}


bool mus_sound_channel_maxamp_exists(const char *file, int chan)
{
  sound_file *sf; 
  sf = get_sf(file); 
  return((sf) && 
	 (sf->maxtimes) && 
	 (sf->maxamps_size > chan) && 
	 (sf->maxtimes[chan] != -1));
}


mus_float_t mus_sound_channel_maxamp(const char *file, int chan, mus_long_t *pos)
{
  sound_file *sf; 
  sf = get_sf(file); 
  if ((chan < sf->maxamps_size) &&
      (sf->maxtimes))
    {
      (*pos) = sf->maxtimes[chan];
      return(sf->maxamps[chan]);
    }
  return(-1.0);
}


void mus_sound_channel_set_maxamp(const char *file, int chan, mus_float_t mx, mus_long_t pos)
{
  sound_file *sf; 
  sf = get_sf(file); 
  if ((sf) &&
      (sf->chans > chan))
    {
      if (!(sf->maxamps))
	{
	  int i;
	  sf->maxamps = (mus_float_t *)malloc(sf->chans * sizeof(mus_float_t));
	  sf->maxtimes = (mus_long_t *)malloc(sf->chans * sizeof(mus_long_t));
	  sf->maxamps_size = sf->chans;
	  for (i = 0; i < sf->chans; i++)
	    {
	      sf->maxamps[i] = -1.0;
	      sf->maxtimes[i] = -1;
	    }
	}
      sf->maxamps[chan] = mx;
      sf->maxtimes[chan] = pos;
    }
}


mus_long_t mus_file_to_array(const char *filename, int chan, mus_long_t start, mus_long_t samples, mus_float_t *array)
{
  int ifd, chans;
  mus_long_t total_read;
  mus_float_t **bufs;

  ifd = mus_sound_open_input(filename);
  if (ifd == MUS_ERROR) return(MUS_ERROR);

  chans = mus_sound_chans(filename);
  if (chan >= chans) 
    {
      mus_sound_close_input(ifd);      
      return(mus_error(MUS_NO_SUCH_CHANNEL, "mus_file_to_array can't read %s channel %d (file has %d chans)", filename, chan, chans));
    }

  bufs = (mus_float_t **)calloc(chans, sizeof(mus_float_t *));
  bufs[chan] = array;

  mus_file_seek_frame(ifd, start);
  total_read = mus_file_read_any(ifd, 0, chans, samples, bufs, bufs);

  mus_sound_close_input(ifd);
  free(bufs);

  return(total_read);
}


const char *mus_array_to_file_with_error(const char *filename, mus_float_t *ddata, mus_long_t len, int srate, int channels)
{
  /* put ddata into a sound file, taking byte order into account */
  /* assume ddata is interleaved already if more than one channel */
  int fd, err = MUS_NO_ERROR;
  mus_long_t oloc;
  mus_float_t *bufs[1];
  mus_sound_forget(filename);

  err = mus_write_header(filename, MUS_NEXT, srate, channels, len * channels, MUS_OUT_FORMAT, "array->file");
  if (err != MUS_NO_ERROR)
    return("mus_array_to_file can't create output file");
  oloc = mus_header_data_location();
  fd = mus_file_reopen_write(filename);
  lseek(fd, oloc, SEEK_SET);
  err = mus_file_open_descriptors(fd, filename,
				  MUS_OUT_FORMAT,
				  mus_bytes_per_sample(MUS_OUT_FORMAT),
				  oloc, channels, MUS_NEXT);
  if (err != MUS_ERROR)
    {
      bufs[0] = ddata;
      err = mus_file_write(fd, 0, len - 1, 1, bufs); /* 1 = chans?? */
    }
  mus_file_close(fd);
  if (err == MUS_ERROR)
    return("mus_array_to_file write error");
  return(NULL);
}

int mus_array_to_file(const char *filename, mus_float_t *ddata, mus_long_t len, int srate, int channels)
{
  const char *errmsg;
  errmsg = mus_array_to_file_with_error(filename, ddata, len, srate, channels);
  if (errmsg)
    return(mus_error(MUS_CANT_OPEN_FILE, "%s", errmsg));
  return(MUS_NO_ERROR);
}


mus_long_t mus_file_to_float_array(const char *filename, int chan, mus_long_t start, mus_long_t samples, mus_float_t *array)
{
  return(mus_file_to_array(filename, chan, start, samples, array));
}


int mus_float_array_to_file(const char *filename, mus_float_t *ddata, mus_long_t len, int srate, int channels)
{
  const char *errmsg;
  errmsg = mus_array_to_file_with_error(filename, ddata, len, srate, channels);
  if (errmsg)
    return(mus_error(MUS_CANT_OPEN_FILE, "%s", errmsg));

  return(MUS_NO_ERROR);
}


int mus_sound_initialize(void)
{
  static bool sndlib_initialized = false;
  if (!sndlib_initialized)
    {
      int err = MUS_NO_ERROR;
      sndlib_initialized = true;
      mus_error_handler = default_mus_error;
      err = mus_header_initialize();
      return(err);
    }
  return(MUS_NO_ERROR);
}


