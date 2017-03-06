// Same license as OpenSlide
// Written by Mathieu Malaterre

/*
 * Compile me:
 *
 * $ cc -o dumpwtp dumpwtp.c
 *
 * This is a debug code, please compile me with assert() enabled.
 *
 * Usage: ./dumpwtp /path/to/input.wtp
 *
 * Warning: it will generate tons of *.jpg file in current working directory
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

struct tile
{
/* 8 */uint64_t offset;
/* 4 */uint32_t numbytes;
/* 4 */uint32_t dummy2;
};

static void tile_read( struct tile * self, FILE * stream )
{
  fread( (char*)&self->offset, 1, sizeof(self->offset), stream );
  fread( (char*)&self->numbytes, 1, sizeof(self->numbytes), stream );
  fread( (char*)&self->dummy2, 1, sizeof(self->dummy2), stream );
}

static void tile_print( const struct tile * self, FILE * stream )
{
  fprintf( stream, "offset:    %ld\n"   , self->offset );
  fprintf( stream, "numbytes:  %d\n"   , self->numbytes);
  fprintf( stream, "dummy2:    %d\n"   , self->dummy2);
}

struct wtp_header
{
/* 4 */char magic[4]; // WTP0
/* 4 */uint32_t nbytes;
/* 4 */uint32_t dummy1;
/* 4 */uint32_t dummy2;
/* 4 */uint32_t dummy3;
/* 4 */uint32_t dummy4;
/* 4 */uint32_t dummy5;
/* 4 */uint32_t dummy6;
/* 4 */uint32_t dummy7;
/* 4 */uint32_t dummy8;
/* 4 */uint32_t ntiles;
/* 4 */uint32_t dummy10;
/* 4 */uint32_t tiledim;
/* 4 */uint32_t dummy12;
/* 4 */uint32_t dummy13;
/* 4 */uint32_t dummy14;
/* 4 */uint32_t firstoffset;
/* 4 */uint32_t dummy16;
/* 4 */uint32_t compression; // 2 -> JPEG, 3 -> JPEG 2000
/* 4 */uint32_t quality;
};

static void wtp_header_read( struct wtp_header * self, FILE * stream )
{
  static const char WTP_MAGIC[4] = "WTP";
  fread( self->magic, 1, sizeof(self->magic), stream );
  assert( strncmp( self->magic, WTP_MAGIC, 4 ) == 0 );
  fread( (char*)&self->nbytes, 1, sizeof(self->nbytes), stream );
  assert( self->nbytes == 272 );
  fread( (char*)&self->dummy1, 1, sizeof(self->dummy1), stream );
  //assert( self->dummy1 == 65536 ); // 0x10000 or 2 ??
  fread( (char*)&self->dummy2, 1, sizeof(self->dummy2), stream );
  assert( self->dummy2 == 2 );
  fread( (char*)&self->dummy3, 1, sizeof(self->dummy3), stream );
  assert( self->dummy3 == 3 );
  fread( (char*)&self->dummy4, 1, sizeof(self->dummy4), stream );
  assert( self->dummy4 == 4 );
  fread( (char*)&self->dummy5, 1, sizeof(self->dummy5), stream );
  fread( (char*)&self->dummy6, 1, sizeof(self->dummy6), stream );
  fread( (char*)&self->dummy7, 1, sizeof(self->dummy7), stream );
  assert( self->dummy7 == 9 );
  fread( (char*)&self->dummy8, 1, sizeof(self->dummy8), stream );
  fread( (char*)&self->ntiles, 1, sizeof(self->ntiles), stream );
  fread( (char*)&self->dummy10, 1, sizeof(self->dummy10), stream );
  fread( (char*)&self->tiledim, 1, sizeof(self->tiledim), stream );
  fread( (char*)&self->dummy12, 1, sizeof(self->dummy12), stream );
  assert( self->dummy12 == 0 );
  fread( (char*)&self->dummy13, 1, sizeof(self->dummy13), stream );
  fread( (char*)&self->dummy14, 1, sizeof(self->dummy14), stream );
  fread( (char*)&self->firstoffset, 1, sizeof(self->firstoffset), stream );
  fread( (char*)&self->dummy16, 1, sizeof(self->dummy16), stream );
  assert( self->dummy16 == 0 );
  fread( (char*)&self->compression, 1, sizeof(self->compression), stream );
  fread( (char*)&self->quality, 1, sizeof(self->quality), stream );
}

static const char *wtp_header_getcomp( struct wtp_header * self )
{
  if( self->compression == 2 ) return "JPEG";
  else if( self->compression == 3 ) return "JPEG 2000";
  assert( 0 );
  return NULL;
}

static void wtp_header_print( struct wtp_header * self, FILE * stream )
{
  fprintf( stream, "magic : %s\n", self->magic );
  fprintf( stream, "nbytes: %d\n", self->nbytes);
  fprintf( stream, "dummy1: %d\n", self->dummy1 );
  fprintf( stream, "dummy2: %d\n", self->dummy2 );
  fprintf( stream, "dummy3: %d\n", self->dummy3 );
  fprintf( stream, "dummy4: %d\n", self->dummy4 );
  fprintf( stream, "dummy5: %d\n", self->dummy5 );
  fprintf( stream, "dummy6: %d\n", self->dummy6 );
  fprintf( stream, "dummy7: %d\n", self->dummy7 );
  fprintf( stream, "dummy8: %d\n", self->dummy8 );
  fprintf( stream, "ntiles: %d\n", self->ntiles);
  fprintf( stream, "dummy10: %d\n", self->dummy10 );
  fprintf( stream, "tiledim: %d\n", self->tiledim);
  fprintf( stream, "dummy12: %d\n", self->dummy12 );
  fprintf( stream, "dummy13: %d\n", self->dummy13 );
  fprintf( stream, "dummy14: %d\n", self->dummy14 );
  fprintf( stream, "firstof: %d\n", self->firstoffset);
  fprintf( stream, "dummy16: %d\n", self->dummy16 );
  fprintf( stream, "compres: %s\n", wtp_header_getcomp(self) );
  fprintf( stream, "quality: %d\n", self->quality);
}

int main(int argc, char *argv[])
{
  if( argc < 2 ) return 1;
  const char *filename = argv[1];
  FILE * stream = fopen( filename, "rb" );
  if( ! stream ) return 1;

  // WTP
  struct wtp_header wh;
  wtp_header_read( &wh, stream );
  wtp_header_print( &wh, stdout );

  fseek( stream, 0x200, SEEK_SET );

  struct tile * tiles;
  tiles = (struct tile*)malloc( wh.ntiles * sizeof( struct tile ) );
  uint32_t n;
  for( n = 0; n < wh.ntiles; ++n )
    {
    struct tile t;
    tile_read( &t, stream );
    //tile_print( &t, stdout );
    tiles[n] = t;
    }

  const struct tile * t = tiles;
  // firstoffset seems to work sometimes, and sometimes not...
  //assert( cur->offset == wh.firstoffset );

  const struct tile fake = { 0xffffffffffffffffL, 0, 0 };
  unsigned char *buffer = NULL;
  const char format[] = "dumpwtp%04d.jpg";
  char outname[512];
  uint32_t tileidx;
  for( tileidx = 0; tileidx < wh.ntiles; ++t, ++tileidx )
    {
    if( t->offset == fake.offset )
      {
      assert( t->numbytes == fake.numbytes );
      assert( t->dummy2 == fake.dummy2 );
      fprintf( stdout, "Empty tile %u!\n", tileidx );
      continue;
      }
    fseek( stream, t->offset, SEEK_SET );

    buffer = realloc(buffer, t->numbytes );
    fread( buffer, 1, t->numbytes, stream );

    sprintf( outname, format, tileidx );
    FILE * out = fopen( outname, "wb" );
    fwrite( buffer, 1, t->numbytes, out );
    fclose( out );
    fprintf( stdout, "tile: %s\n", outname );
    }
 
  return 0;
}
