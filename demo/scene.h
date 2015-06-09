#ifndef DEMO_SCENE_H
#define DEMO_SCENE_H

#include <cstdint>

#include "etl/data/maybe.h"

namespace demo {

/*
 * A chunk of a demo, which will render frames until it stops doing so.
 */
class Scene {
public:
  virtual ~Scene() = default;

  /*
   * Calls vga::configure_band_list with the right set of bands for this scene.
   */
  virtual void configure_band_list() = 0;

  /*
   * Renders a single frame, given its number since this scene began.  This will
   * be called by the Runner shortly after entering vblank.  Implementations
   * should typically follow this pattern:
   *
   * 1. Do any work that needs to happen before video begins.  If the renderer
   *    is using double-buffering, this may include flipping the visible frame.
   *
   * 2. Perform any updates that can occur during video, for example, rendering
   *    into a background buffer.
   *
   * 3. Return 'true' to continue with this scene, 'false' to move on.
   *
   * If the implementation of this function runs long, the next frame will be
   * delayed until the next *complete* vblank.  That is, if the implementation
   * crosses into vblank, the Runner will delay until the *next* vblank before
   * calling it again.  Implementations with a lot of work to do can
   * deliberately synchronize with a subdivided frame rate by calling
   * vga::wait_for_vblank themselves.
   *
   * TODO: currently, the renderer cannot *detect* frameskips like this, so
   * renderers with intermittent performance problems that are animating may
   * slow down.
   *
   * TODO: it would be nice if the Scene could permanently subdivide the frame
   * rate, e.g. always get called every third vblank, to do something
   * particularly expensive.
   */
  virtual bool render_frame(unsigned frame) = 0;

protected:
  Scene() = default;
};

}  // namespace demo

#endif  // DEMO_SCENE_H
