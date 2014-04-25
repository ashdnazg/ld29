#include <SDL2/SDL.h>
#include <assert.h>
#include <string.h>

#include "sdl.h"
#include "sdl_video.h"
#include "stb_image.h"

#include "core/mem_wrap.h"
#include "core/int_list.h"
#include "core/macros.h"
#include "core/asset_cache.h"
#include "core/tween.h"

animation_playback_t * animation_playback_new(renderable_t * renderable, animation_t * animation,
                                                unsigned int interval, bool loop) {
    animation_playback_t *playback = mem_alloc(sizeof(animation_playback_t));
    playback->interval = interval;
    playback->total_steps = 0;
    playback->renderable = renderable;
    playback->animation = animation;
    playback->loop = loop;
    link_init(&(playback->animation_playbacks_link));
    
    return playback;
}

void animation_playback_free(animation_playback_t * playback) {
    link_remove_from_list(&(playback->animation_playbacks_link));
    playback->renderable->animation_playback = NULL;
    playback->renderable->sprite = playback->renderable->default_sprite;
    mem_free(playback);
}

void animation_playback_animate(animation_playback_t * playback) {
    if (!(playback->loop) && (playback->total_steps >= playback->interval * playback->animation->num_frames)) {
        animation_playback_free(playback);
        return;
    }
    playback->renderable->sprite = playback->animation->frames[(playback->total_steps / playback->interval) %
                                                                playback->animation->num_frames];
    ++playback->total_steps;
}

sprite_t *render_manager_get_sprite(render_manager_t *r_manager, const char *name) {
    MAYBE(sprite_t *) maybe_spr = asset_cache_get(&(r_manager->sprites), name);
    if (UNMAYBE(maybe_spr) != NULL) {
        return (sprite_t *) UNMAYBE(maybe_spr);
    }
    return load_sprite(r_manager, name);
}

void renderable_init(renderable_t *renderable, sprite_t *default_sprite, int x, int y, int depth) {
    list_init(&(renderable->tweens), tween_t, tweens_link);
    renderable->sprite = default_sprite;
    renderable->default_sprite = default_sprite;
    renderable->x = x;
    renderable->y = y;
    renderable->depth = depth;
    renderable->scale = 1.0;
    renderable->angle = 0;
    renderable->center = NULL;
    renderable->flip = SDL_FLIP_NONE;
    renderable->animation_playback = NULL;
    link_init(&(renderable->renderables_link));
}


renderable_t * renderable_new(sprite_t *default_sprite, int x, int y, int depth) {
    renderable_t *renderable = mem_alloc(sizeof(renderable_t));
    renderable_init(renderable, default_sprite, x, y, depth);
    return renderable;
}

void renderable_clean(renderable_t *renderable) {
    if (renderable->animation_playback != NULL) {
        animation_playback_free(renderable->animation_playback);
    }
    list_for_each(&(renderable->tweens), tween_t *, tween) {
        tween_free(tween);
    }
    link_remove_from_list(&(renderable->renderables_link));
}

void renderable_free(renderable_t *renderable) {
    renderable_clean(renderable);
    mem_free(renderable);
}

animation_t * animation_new(sprite_t **frames, unsigned int num_frames) {
    animation_t *animation = mem_alloc(sizeof(animation_t));
    animation->num_frames = num_frames;
    animation->frames = mem_alloc(sizeof(sprite_t *) * num_frames);
    memcpy(animation->frames, frames, sizeof(sprite_t *) * num_frames);
    return animation;
}

bool file_exists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        return FALSE;
    } else {
        fclose(f);
        return TRUE;
    }
}

animation_t * load_animation(render_manager_t *r_manager, const char * name) {
    int i = 0;
    animation_t *anim;
    char path_buffer[BUFFER_SIZE];
    sprite_t *frames[MAX_FRAMES];
    while (TRUE) {
        sprintf(path_buffer, "%s%s%02d%s", ASSETS_DIR, name, i, SPRITE_EXTENSION);
        if (!file_exists(path_buffer) || i >= MAX_FRAMES) {
            break;
        }
        sprintf(path_buffer, "%s%02d", name, i);
        frames[i] = render_manager_get_sprite(r_manager, path_buffer);
        ++i;
    }
    anim = animation_new(frames, i);
    asset_cache_add(&(r_manager->animations), anim, name);
    return anim;
}

void animation_free(animation_t *animation) {
    mem_free(animation->frames);
    mem_free(animation);
}

animation_t * render_manager_get_animation(render_manager_t *r_manager, const char * name) {
    MAYBE(animation_t *) maybe_anim = asset_cache_get(&(r_manager->animations), name);
    if (UNMAYBE(maybe_anim) != NULL) {
        return (animation_t *) UNMAYBE(maybe_anim);
    }
    return load_animation(r_manager, name);
}

void render_manager_init(render_manager_t *r_manager, SDL_Renderer *renderer) {
    r_manager->renderer = renderer;
    list_init(&(r_manager->renderables), renderable_t, renderables_link);
    list_init(&(r_manager->animation_playbacks), animation_playback_t, animation_playbacks_link);
    asset_cache_init(&(r_manager->animations) , MAYBIFY_FUNC(animation_free));
    asset_cache_init(&(r_manager->textures) , MAYBIFY_FUNC(SDL_DestroyTexture));
    asset_cache_init(&(r_manager->sprites) , MAYBIFY_FUNC(sprite_free));
    r_manager->x_offset = 0;
    r_manager->y_offset = 0;
}

render_manager_t * render_manager_new(SDL_Renderer *renderer) {
    render_manager_t *r_manager = mem_alloc(sizeof(render_manager_t));
    render_manager_init(r_manager, renderer);
    return r_manager;
}
void render_manager_clean(render_manager_t *r_manager) {
    list_for_each(&(r_manager->renderables), renderable_t *, renderable) {
        renderable_free(renderable);
    }
    assert(list_head(&(r_manager->animation_playbacks)) == NULL);
    asset_cache_clean(&(r_manager->animations));
    asset_cache_clean(&(r_manager->textures));
    asset_cache_clean(&(r_manager->sprites));
}

void render_manager_free(render_manager_t *r_manager) {
    render_manager_clean(r_manager);
    mem_free(r_manager);
}

void render_manager_clear(render_manager_t *r_manager) {
    list_for_each(&(r_manager->renderables), renderable_t *, renderable) {
        renderable_free(renderable);
    }
}

renderable_t * render_manager_create_renderable(render_manager_t *r_manager, const char *default_sprite_name, int x, int y, int depth) {
    sprite_t *default_sprite = render_manager_get_sprite(r_manager, default_sprite_name);
    renderable_t *renderable = renderable_new(default_sprite, x , y, depth);
    list_insert_tail(&(r_manager->renderables), renderable);
    return renderable;
}


void render_manager_play_animation(render_manager_t *r_manager, renderable_t *renderable, 
                                    const char *animation_name, unsigned int interval, bool loop) {
    animation_t *animation = render_manager_get_animation(r_manager, animation_name);
    render_manager_stop_animation(r_manager, renderable);
    animation_playback_t * playback = animation_playback_new(renderable, animation, interval, loop);
    renderable->animation_playback = playback;
    list_insert_tail(&(r_manager->animation_playbacks), playback);
}

void render_manager_stop_animation(render_manager_t *r_manager, renderable_t *renderable) {
    if (renderable->animation_playback != NULL) {
        animation_playback_free(renderable->animation_playback);
    }
}

void render_manager_animate(render_manager_t *r_manager) {
    list_for_each(&(r_manager->animation_playbacks), animation_playback_t *, playback){
        animation_playback_animate(playback);
    }
}
int cmp_y(renderable_t *renderable_a, renderable_t *renderable_b) {
    return -(renderable_a->y + renderable_a->depth) >= -(renderable_b->y + renderable_b->depth);
}
void render_manager_draw(render_manager_t *r_manager) {
    SDL_RenderClear(r_manager->renderer);
    list_sort(&(r_manager->renderables), (cmp_cb_t) cmp_y);
    list_for_each(&(r_manager->renderables), renderable_t *, renderable){
        draw_sprite(r_manager, renderable->sprite, renderable->x + r_manager->x_offset, renderable->y + r_manager->y_offset, 
                                renderable->scale, renderable->angle, renderable->center, renderable->flip);
    }
    SDL_RenderPresent(r_manager->renderer);
}


void exit_on_SDL_error(void * pt) {
    if(pt == NULL) {
        printf("SDL Error: %s\n", SDL_GetError());
        exit(1);
    }
}

void exit_on_stbi_error(void * pt) {
    if(pt == NULL) {
        printf("STBI Error: %s\n", stbi_failure_reason());
        exit(1);
    }
}

SDL_Texture * load_image(render_manager_t *r_manager, const char * path) {
    int im_w, im_h;
    unsigned char *image = NULL;
    SDL_Surface *bitmap = NULL;
    SDL_Texture *texture;
    MAYBE(SDL_Texture *) maybe_texture = asset_cache_get(&(r_manager->textures), path);
    if (UNMAYBE(maybe_texture) != NULL) {
        return (SDL_Texture *) UNMAYBE(maybe_texture);
    }
    image = stbi_load(path, &im_w, &im_h, NULL, RGBA);
    exit_on_stbi_error(image);
    
    bitmap = SDL_CreateRGBSurfaceFrom(image, im_w, im_h, RGBA * 8, RGBA * im_w,
                                   RMASK, GMASK, BMASK, AMASK);
    exit_on_SDL_error(bitmap);
    texture = SDL_CreateTextureFromSurface(r_manager->renderer, bitmap);
    
    SDL_FreeSurface(bitmap);
    free(image);
    asset_cache_add(&(r_manager->textures), texture, path);
    return texture;
}

sprite_t * sprite_new(SDL_Texture *texture, int x, int y, int w, int h) {
    sprite_t *sprite = mem_alloc(sizeof(sprite_t));
    SDL_Rect *rect = mem_alloc(sizeof(SDL_Rect));
    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;
    sprite->rect = rect;
    sprite->texture = texture;
    return sprite;
}

void sprite_free(sprite_t *sprite) {
    mem_free(sprite->rect);
    mem_free(sprite);
}

void draw_sprite(render_manager_t *r_manager, sprite_t *sprite, int x, int y, double scale, 
                 double angle, SDL_Point *center, SDL_RendererFlip flip) {
    SDL_Rect pos;
    pos.w = (int) (sprite->rect->w * scale);
    pos.h = (int) (sprite->rect->h * scale);
    pos.x = x + (sprite->rect->w - pos.w) / 2;
    pos.y = y + (sprite->rect->h - pos.h) / 2;
    
    //SDL_RenderCopy(r_manager->renderer, sprite->texture, sprite->rect, &pos);
    SDL_RenderCopyEx(r_manager->renderer, sprite->texture, sprite->rect, &pos, angle, center, flip);
}

void draw_image(render_manager_t *r_manager, SDL_Texture *texture, int x, int y) {
    SDL_Rect pos;
    pos.x = x;
    pos.y = y;
    SDL_QueryTexture(texture, NULL, NULL, &pos.w, &pos.h);
 
    SDL_RenderCopy(r_manager->renderer, texture, NULL, &pos);
}

sprite_t * load_sprite(render_manager_t *r_manager, const char * name) {
    sprite_t *spr;
    char path_buffer[BUFFER_SIZE];
    sprintf(path_buffer, "%s%s%s", ASSETS_DIR, name, SPRITE_EXTENSION);
    int texture_width, texture_height;
    SDL_Texture *texture = load_image(r_manager, path_buffer);
    SDL_QueryTexture(texture, NULL, NULL, &texture_width, &texture_height);
    spr = sprite_new(texture, 0, 0, texture_width, texture_height);
    asset_cache_add(&(r_manager->sprites), spr, name);
    return spr;
}

// sprite_t * load_sprite(render_manager_t *r_manager, const char * path) {
    // int texture_width, texture_height;
    // SDL_Texture *texture = load_image(r_manager, path);
    // SDL_QueryTexture(texture, NULL, NULL, &texture_width, &texture_height);
    // return sprite_new(texture, 0, 0, texture_width, texture_height);
// }

sprite_t ** load_sprite_sheet(render_manager_t *r_manager, const char * path, 
                        int spr_width, int spr_height, int padding, unsigned int *out_num_sprites) {
    SDL_Texture *texture = NULL;
    sprite_t ** sprites = NULL;
    int num_rows, num_cols, texture_width, texture_height, i;
    
    texture = load_image(r_manager, path);
    SDL_QueryTexture(texture, NULL, NULL, &texture_width, &texture_height);
    num_cols = ((texture_width - padding) / (spr_width + padding));
    num_rows = ((texture_height - padding) / (spr_height + padding));
    *out_num_sprites = num_rows * num_cols;

    sprites = mem_alloc(sizeof(sprite_t *) * (*out_num_sprites));
    
    for (i = 0;i < (*out_num_sprites); ++i) {
        sprites[i] = sprite_new(texture, (i % num_cols) * (spr_width + padding) + padding, 
                                        (i / num_cols) * (spr_height + padding) + padding, spr_width, spr_height);
    }
    
    return sprites;
}
