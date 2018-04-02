#include "sdl2_partial_rwops.h"
#include "sdl2.h"
#include "mruby/value.h"
#include "mruby/string.h"
#include "sdl2_render.h"
#include "sdl2_surface.h"
#include "sdl2-mixer.h"
#include "sdl2-ttf.h"
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

static int myclose(SDL_RWops * context)
{
    free(context->hidden.unknown.data1);
    if (context) {
        SDL_FreeRW(context);
    }
    return 0;
}

static SDL_RWops* new_partial_file_rwops(char* file, int start, int size)
{
  SDL_RWops *file_rw;
  SDL_RWops* partial_rw;
  void *buffer;

  buffer = malloc(size);

  file_rw = SDL_RWFromFile(file,"rb");
  SDL_RWseek( file_rw, start, RW_SEEK_SET );
  SDL_RWread( file_rw, buffer, size, 1 );
  SDL_RWclose(file_rw);
  partial_rw = SDL_RWFromConstMem( buffer, size );
  /* memorize buffer address, override close method. */
  partial_rw->hidden.unknown.data1 = buffer;
  partial_rw->close = myclose;
  return partial_rw;
}

/*
 * Renderer
 */

static mrb_value
mrb_sdl2_partial_rwops_renderer_load_partial(mrb_state *mrb, mrb_value self)
{
  mrb_value file;
  mrb_int start, size;
  SDL_Renderer *renderer;
  SDL_RWops * rwops;
  mrb_value result;

  mrb_get_args(mrb, "Sii", &file, &start, &size);
  rwops = new_partial_file_rwops(RSTRING_PTR(file), start, size);
  renderer = mrb_sdl2_video_renderer_get_ptr(mrb, self);
  result = mrb_sdl2_video_texture(mrb, IMG_LoadTexture_RW(renderer, rwops, 1));

  return result;
}

/*
 * Surface
 */

static mrb_value
mrb_sdl2_partial_rwops_surface_load_partial(mrb_state *mrb, mrb_value self)
{
  mrb_value file;
  mrb_int start, size;
  SDL_Surface *surface;
  SDL_RWops * rwops;
  mrb_value result;

  mrb_get_args(mrb, "Sii", &file, &start, &size);
  rwops = new_partial_file_rwops(RSTRING_PTR(file), start, size);
  surface = IMG_Load_RW(rwops, 1);
  result = mrb_sdl2_video_surface(mrb, surface, true);

  return result;
}

/*
 * Mixer Chunk
 */

 static mrb_value
 mrb_sdl2_partial_rwops_chunk_load_partial(mrb_state *mrb, mrb_value self)
 {
   mrb_value file;
   mrb_int start, size;
   Mix_Chunk *chunk;
   SDL_RWops * rwops;
   mrb_value result;

   mrb_get_args(mrb, "Sii", &file, &start, &size);
   rwops = new_partial_file_rwops(RSTRING_PTR(file), start, size);
   chunk = Mix_LoadWAV_RW(rwops, 1);
   result = mrb_sdl2_chunk(mrb, chunk);

   return result;
}

/*
 * Mixer Music
 */

static mrb_value
mrb_sdl2_partial_rwops_music_load_partial(mrb_state *mrb, mrb_value self)
{
  mrb_value file;
  mrb_int start, size;
  Mix_Music *music;
  SDL_RWops * rwops;
  mrb_value result;

  mrb_get_args(mrb, "Sii", &file, &start, &size);
  rwops = new_partial_file_rwops(RSTRING_PTR(file), start, size);
  music = Mix_LoadMUS_RW(rwops, 1);
  result = mrb_sdl2_music(mrb, music);

  return result;
}

/*
 * Font
 */

static mrb_value
mrb_sdl2_partial_rwops_font_load_partial(mrb_state *mrb, mrb_value self)
{
  mrb_value file;
  mrb_int start, size;
  mrb_int fontsize;
  TTF_Font *font;
  SDL_RWops * rwops;
  mrb_value result;

  mrb_get_args(mrb, "Siii", &file, &start, &size,&fontsize);
  rwops = new_partial_file_rwops(RSTRING_PTR(file), start, size);
  font = TTF_OpenFontRW(rwops, 1, fontsize);
  result = mrb_sdl2_font(mrb, font);

  return result;
}

/*
 * gem init and final
 */

void
mrb_mruby_sdl2_partial_rwops_gem_init(mrb_state *mrb)
{
    struct RClass *mod_Video;
    struct RClass *class_Renderer;
    struct RClass *class_Surface;
    struct RClass *mod_Mixer;
    struct RClass *class_Chunk;
    struct RClass *class_Music;
    struct RClass *mod_Ttf;
    struct RClass *class_Font;

    mod_Video      = mrb_module_get_under(mrb, mod_SDL2,  "Video");
    class_Renderer = mrb_class_get_under (mrb, mod_Video, "Renderer");
    class_Surface  = mrb_class_get_under (mrb, mod_Video, "Surface");
    mod_Mixer      = mrb_module_get_under(mrb, mod_SDL2, "Mixer");
    class_Chunk    = mrb_class_get_under (mrb, mod_Mixer, "Chunk");
    class_Music    = mrb_class_get_under (mrb, mod_Mixer, "Music");
    mod_Ttf        = mrb_module_get_under(mrb, mod_SDL2, "TTF");
    class_Font     = mrb_class_get_under (mrb, mod_Ttf, "Font");

    mrb_define_method(mrb, class_Renderer, "load_partial", mrb_sdl2_partial_rwops_renderer_load_partial, MRB_ARGS_REQ(3));
    mrb_define_class_method(mrb, class_Surface, "load_partial", mrb_sdl2_partial_rwops_surface_load_partial, MRB_ARGS_REQ(3));
    mrb_define_class_method(mrb, class_Chunk, "load_partial", mrb_sdl2_partial_rwops_chunk_load_partial, MRB_ARGS_REQ(3));
    mrb_define_class_method(mrb, class_Music, "load_partial", mrb_sdl2_partial_rwops_music_load_partial, MRB_ARGS_REQ(3));
    mrb_define_class_method(mrb, class_Font, "load_partial", mrb_sdl2_partial_rwops_font_load_partial, MRB_ARGS_REQ(4));
}

void
mrb_mruby_sdl2_partial_rwops_gem_final(mrb_state *mrb)
{
}
