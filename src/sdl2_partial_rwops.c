#include "sdl2_partial_rwops.h"
#include "sdl2.h"
#include "mruby/value.h"
#include "mruby/string.h"
#include "sdl2_render.h"
#include "sdl2_surface.h"
#include "sdl2-mixer.h"
#include <SDL_image.h>
#include <SDL_mixer.h>


SDL_RWops* new_partial_file_rwops(char* file, int start, int size)
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
  partial_rw->hidden.unknown.data1 = buffer;
  return partial_rw;
}

void free_partial_file_rwops(SDL_RWops *rwops)
{
  free(rwops->hidden.unknown.data1);
  SDL_RWclose(rwops);
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
  result = mrb_sdl2_video_texture(mrb, IMG_LoadTexture_RW(renderer, rwops, 0));
  free_partial_file_rwops(rwops);

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
  surface = IMG_Load_RW(rwops, 0);
  result = mrb_sdl2_video_surface(mrb, surface, true);
  free_partial_file_rwops(rwops);

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
   chunk = Mix_LoadWAV_RW(rwops, 0);
   result = mrb_sdl2_chunk(mrb, chunk);
   free_partial_file_rwops(rwops);

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

    mod_Video      = mrb_module_get_under(mrb, mod_SDL2,  "Video");
    class_Renderer = mrb_class_get_under (mrb, mod_Video, "Renderer");
    class_Surface  = mrb_class_get_under (mrb, mod_Video, "Surface");
    mod_Mixer      = mrb_module_get_under(mrb, mod_SDL2, "Mixer");
    class_Chunk    = mrb_class_get_under (mrb, mod_Mixer, "Chunk");

    mrb_define_method(mrb, class_Renderer, "load_partial", mrb_sdl2_partial_rwops_renderer_load_partial, MRB_ARGS_REQ(3));
    mrb_define_class_method(mrb, class_Surface, "load_partial", mrb_sdl2_partial_rwops_surface_load_partial, MRB_ARGS_REQ(3));
    mrb_define_class_method(mrb, class_Chunk, "load_partial", mrb_sdl2_partial_rwops_chunk_load_partial, MRB_ARGS_REQ(3));
}

void
mrb_mruby_sdl2_partial_rwops_gem_final(mrb_state *mrb)
{
}
