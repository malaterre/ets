// Same license as OpenSlide
// Written by Mathieu Malaterre

/*
 * Very simply VSI + ETS parser (should work for WTP + ETS). It dumps basic
 * format info and then extract all tiles from level=0.
 *
 * Compile me:
 *
 * $ cc -o dumpets dumpets.c
 *
 * This is a debug code, please compile me with assert() enabled.
 *
 * Usage: ./dumpets /path/to/frame_t.ets
 *
 * Warning: it will generate tons of *.jpg file in current working directory
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

struct sis_header
{
/* 4 */char magic[4]; // SIS0
/* 4 */uint32_t nbytes;
/* 4 */uint32_t version;
/* 4 */uint32_t dim;
/* 8 */uint64_t etsoffset;
/* 4 */uint32_t etsnbytes;
/* 4 */uint32_t dummy0;
/* 8 */uint64_t offsettiles;
/* 4 */uint32_t ntiles;
/* 4 */uint32_t dummy1;
/* 4 */uint32_t dummy2;
/* 4 */uint32_t dummy3;
/* 4 */uint32_t dummy4;
/* 4 */uint32_t dummy5;
};

static void sis_header_read( struct sis_header * self, FILE * stream )
{
  static const char SIS_MAGIC[4] = "SIS";
  fread( self->magic, 1, sizeof(self->magic), stream );
  assert( strncmp( self->magic, SIS_MAGIC, 4 ) == 0 );
  fread( (char*)&self->nbytes, 1, sizeof(self->nbytes), stream );
  assert( self->nbytes == 64 ); // size of struct
  fread( (char*)&self->version, 1, sizeof(self->version), stream );
  assert( self->version == 2 ); // version ??
  fread( (char*)&self->dim, 1, sizeof(self->dim), stream );
  assert( self->dim == 4 || self->dim == 6 ); // dim ?
  fread( (char*)&self->etsoffset, 1, sizeof(self->etsoffset), stream );
  assert( self->etsoffset == 64 ); // offset of ETS struct
  fread( (char*)&self->etsnbytes, 1, sizeof(self->etsnbytes), stream );
  assert( self->etsnbytes == 228 ); // size of ETS struct
  fread( (char*)&self->dummy0, 1, sizeof(self->dummy0), stream );
  assert( self->dummy0 == 0 ); // ??
  fread( (char*)&self->offsettiles, 1, sizeof(self->offsettiles), stream ); // offset to tiles
  fread( (char*)&self->ntiles, 1, sizeof(self->ntiles), stream ); // number of tiles
  fread( (char*)&self->dummy1, 1, sizeof(self->dummy1), stream ); // ??
  assert( self->dummy1 == 0 ); // always zero ?
  fread( (char*)&self->dummy2, 1, sizeof(self->dummy2), stream ); // some kind of offset ?
  //assert( dummy2 == 0 ); // not always
  fread( (char*)&self->dummy3, 1, sizeof(self->dummy3), stream );
  assert( self->dummy3 == 0 ); // always zero ?
  fread( (char*)&self->dummy4, 1, sizeof(self->dummy4), stream );
  //assert( dummy4 == 0 ); // not always
  fread( (char*)&self->dummy5, 1, sizeof(self->dummy5), stream );
  assert( self->dummy5 == 0 ); // always zero ?
}

static void sis_header_print( struct sis_header * self, FILE * stream )
{
  fprintf( stream, "magic : %s\n"  , self->magic );
  fprintf( stream, "nbytes: %d\n"  , self->nbytes );
  fprintf( stream, "versi : %d\n"  , self->version );
  fprintf( stream, "dim   : %d\n"  , self->dim );
  fprintf( stream, "etsoff: %ld\n" , self->etsoffset );
  fprintf( stream, "etsnby: %d\n"  , self->etsnbytes );
  fprintf( stream, "dummy0: %d\n"  , self->dummy0 );
  fprintf( stream, "offtil: %ld\n" , self->offsettiles );
  fprintf( stream, "ntiles: %d\n"  , self->ntiles );
  fprintf( stream, "dummy1: %d\n"  , self->dummy1 );
  fprintf( stream, "dummy2: %d\n"  , self->dummy2 );
  fprintf( stream, "dummy3: %d\n"  , self->dummy3 );
  fprintf( stream, "dummy4: %d\n"  , self->dummy4 );
  fprintf( stream, "dummy5: %d\n"  , self->dummy5 );
}

struct ets_header
{
/* 4 */char magic[4]; // ETS0
/* 4 */uint32_t version;
/* 4 */uint32_t dummy1;
/* 4 */uint32_t dummy2;
/* 4 */uint32_t dummy3;
/* 4 */uint32_t compression;
/* 4 */uint32_t quality;
/* 4 */uint32_t dimx;
/* 4 */uint32_t dimy;
/* 4 */uint32_t dimz;
};

static void ets_header_read( struct ets_header * self, FILE * stream )
{
  static const char ETS_MAGIC[4] = "ETS";
  fread( self->magic, 1, sizeof(self->magic), stream );
  assert( strncmp( self->magic, ETS_MAGIC, 4 ) == 0 );
  fread( (char*)&self->version, 1, sizeof(self->version), stream );
  assert( self->version == 0x30001 || self->version == 0x30003 ); // some kind of version ?
  fread( (char*)&self->dummy1, 1, sizeof(self->dummy1), stream );
  assert( self->dummy1 == 2 || self->dummy1 == 4 /* when sis_header->dim == 4 */ );
  fread( (char*)&self->dummy2, 1, sizeof(self->dummy2), stream );
  assert( self->dummy2 == 3 || self->dummy2 == 1 );
  fread( (char*)&self->dummy3, 1, sizeof(self->dummy3), stream );
  assert( self->dummy3 == 4 || self->dummy3 == 1 );
  fread( (char*)&self->compression, 1, sizeof(self->compression), stream ); // codec
  // 0 -> ?
  // 2 -> JPEG ?
  // 3 -> JPEG 2000 ?
  assert( self->compression == 2 || self->compression == 3 || self->compression == 0 );
  fread( (char*)&self->quality, 1, sizeof(self->quality), stream );
  assert( self->quality == 90 || self->quality == 100 ); // some kind of JPEG quality ?
  fread( (char*)&self->dimx, 1, sizeof(self->dimx), stream );
  //assert( self->dimx == 512 ); // always tile of 512x512 ?
  fread( (char*)&self->dimy, 1, sizeof(self->dimy), stream );
  //assert( self->dimy == 512 ); //
  fread( (char*)&self->dimz, 1, sizeof(self->dimz), stream );
  assert( self->dimz == 1 ); // dimz ?
}
static const char *ets_header_getcomp( struct ets_header * self)
{
  if( self->compression == 0 ) return "raw";
  else if( self->compression == 2 ) return "jpg";
  else if( self->compression == 3 ) return "jp2";
  assert( 0 );
  return NULL;
}
static void ets_header_print( struct ets_header * self, FILE * stream )
{
  fprintf( stream, "magic : %s\n", self->magic );
  fprintf( stream, "versio: %d\n", self->version );
  fprintf( stream, "dummy1: %d\n", self->dummy1 );
  fprintf( stream, "dummy2: %d\n", self->dummy2 );
  fprintf( stream, "dummy3: %d\n", self->dummy3 );
  fprintf( stream, "compre: %s\n", ets_header_getcomp(self) );
  fprintf( stream, "qualit: %d\n", self->quality );
  fprintf( stream, "dimx  : %d\n", self->dimx );
  fprintf( stream, "dimy  : %d\n", self->dimy );
  fprintf( stream, "dimz  : %d\n", self->dimz );
}

struct tile
{
/* 4 */uint32_t dummy1;
/* 4 */uint32_t coord[3];
/* 4 */uint32_t level;
/* 8 */uint64_t offset;
/* 4 */uint32_t numbytes;
/* 4 */uint32_t dummy2;
};

static void tile_read( struct tile * self, FILE * stream )
{
  fread( (char*)&self->dummy1, 1, sizeof(self->dummy1), stream );
  //assert( self->dummy1 == 4 ); // wotsit ?
  fread( (char*)self->coord, 1, sizeof(self->coord), stream );
  fread( (char*)&self->level, 1, sizeof(self->level), stream );
  fread( (char*)&self->offset, 1, sizeof(self->offset), stream );
  fread( (char*)&self->numbytes, 1, sizeof(self->numbytes), stream );
  fread( (char*)&self->dummy2, 1, sizeof(self->dummy2), stream );
  /*
  assert( dummy2 == 0x18c78d || dummy2 == 0x6171e8
  || dummy2 == 0x610138 || dummy2 == 0x21394dc8
  || dummy2 == 0x0 || dummy2 == 0x21893008
  || dummy2 == 0x215bcd88 || dummy2 == 0x2142ef78 || dummy2 == 0x617208 );
   */
}

static void tile_print( const struct tile * self, FILE * stream )
{
  fprintf( stream, "coord: %d,%d,%d\n", self->coord[0], self->coord[1], self->coord[2] );
  fprintf( stream, "level:    %d\n"   , self->level );
  fprintf( stream, "offset:   %ld\n"  , self->offset );
  fprintf( stream, "numbytes: %d\n"   , self->numbytes );
  fprintf( stream, "dummy2:   %d\n"   , self->dummy2 );
}

static const struct tile *findtile( struct tile *tiles, size_t ntiles, const uint32_t coord[3] )
{
  const struct tile *ret = NULL;
  size_t n;
  for( n = 0; n < ntiles; ++n )
    {
    const struct tile * t = tiles + n;
    if( t->level == 0 )
      {
      if( t->coord[0] == coord[0] && t->coord[1] == coord[1] )
        {
        ret = t;
        }
      }
    }
  return ret;
}

#define std_max( x, y ) x > y ? x : y

int main(int argc, char *argv[])
{
  if( argc < 2 ) return 1;
  const char * filename = argv[1];
  FILE * stream = fopen( filename, "rb" );

  // SIS:
  struct sis_header sh;
  //assert( sizeof( sis_header ) == 64 );
  sis_header_read( &sh, stream );
  sis_header_print( &sh, stdout );

  // ETS:
  struct ets_header eh;
  ets_header_read( &eh, stream );
  ets_header_print( &eh, stdout );

  // individual tiles
  fseek( stream, sh.offsettiles, SEEK_SET );
  struct tile * tiles;
  tiles = (struct tile*)malloc( sh.ntiles * sizeof( struct tile ) );
  uint32_t n;
  for( n = 0; n < sh.ntiles; ++n )
    {
    struct tile t;
    tile_read( &t, stream );
    //tile_print( &t, stdout );
    tiles[n] = t;
    }

  // computes tiles dims
  uint32_t tilexmax = 0;
  uint32_t tileymax = 0;

  for( n = 0; n < sh.ntiles; ++n )
    {
    const struct tile *t = tiles+n;
    if( t->level == 0 )
      {
      tilexmax = std_max( t->coord[0], tilexmax );
      tileymax = std_max( t->coord[1], tileymax );
      }
    }
  //assert( tilexmax + 1 == 34 );
  //assert( tileymax + 1 == 14 );

  // compute image info:
  size_t ImageWidth = eh.dimx*(tilexmax + 1);
  size_t ImageLength = eh.dimy*(tileymax + 1);
  assert( eh.dimz == 1 );
  size_t TileWidth = eh.dimx;
  size_t TileLength = eh.dimy;

  size_t TilesAcross = (ImageWidth + TileWidth - 1) / TileWidth;
  size_t TilesDown = (ImageLength + TileLength - 1) / TileLength;
  size_t TilesPerImage = TilesAcross * TilesDown;

  fprintf( stdout , "TilesAcross:   %ld\n", TilesAcross );
  fprintf( stdout , "TilesDown:     %ld\n", TilesDown );
  fprintf( stdout , "TilesPerImage: %ld\n", TilesPerImage );
  size_t linestripe = TilesAcross * TileWidth * 3;

  // extract images from level=0
  const size_t ntiles = TilesPerImage;
  size_t tileidx;
  unsigned char *buffer = NULL;
  const char format[] = "dumpets%04ld.%s";
  char outname[512];
  for( tileidx = 0; tileidx < ntiles; ++tileidx )
    {
    const size_t tilex = tileidx % TilesAcross;
    const size_t tiley = tileidx / TilesAcross;
    uint32_t ref[3];
    ref[0] = tilex;
    ref[1] = tiley;
    ref[2] = 0;

    const struct tile *t = findtile( tiles, ntiles, ref );
    if( t )
      {
      assert( t->level == 0 );
      if( t->level == 0 )
        {
        tile_print( t, stdout );
        fseek( stream, t->offset, SEEK_SET );
        buffer = realloc(buffer, t->numbytes );
        fread( buffer, 1, t->numbytes, stream );

        const char *ext = ets_header_getcomp( &eh );
        sprintf( outname, format, tileidx, ext );
        FILE * out = fopen( outname, "wb" );
        fwrite( buffer, 1, t->numbytes, out );
        fclose( out );
        fprintf( stdout, "tile: %s\n", outname );
        }
      }
    else
      {
      // need to make a fake tile
      fprintf( stdout, "Empty tile %u!\n", tileidx );
      }
    }
  // cleanup
  free( buffer );
  free( tiles );
  fclose( stream );

  return 0;
}
