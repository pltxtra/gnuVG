gnuVG -- An OpenVG implementation
==================================

## Overview
gnuVG is a vector graphics library implementing an OpenVG like API. gnuVG uses OpenGL ES 2.0 as a rendering back end for high performance dynamic vector graphics.

It's only been used on Android currently, but non-android platforms should also be working with no or minimal
additional work.

## Copyright & License
Copyright (C) 2014-2016 by Anton Persson

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

## Requirements:

To successfully build gnuVG, the following is required:

```
GCC
Autotools (autoconf, automake, libtool etc.)
libfreetype (compiled and ready)
```

## How to setup and compile

If you have just cloned the GIT repository you must first bootstrap
the build configuration scripts:

```
bootstrap.sh
```

After this you can run configure:

```
# just plainly
./configure

# ---- OR - an example of cross compiling using the Android NDK ---
CFLAGS=-O4 CXXFLAGS=-O4 ./configure --host=arm-linux
```

If configuration is OK, then you can compile and install:

```
make install
```

## Supported APIs

gnuVG is only a partial implementation of the OpenVG API.

* vgSet/GetParameter*()
  * Supported - specifics depend on the applied object
* vgFlush/vgFinish()
  * Callable - no effect
* vgSet/Get*() - The following are supported:
  * VG_MATRIX_MODE
  * VG_BLEND_MODE - only VG_BLEND_SRC and VG_BLEND_SRC_OVER are supported
  * VG_MAX_COLOR_RAMP_STOPS
  * VG_MAX_DASH_COUNT
  * VG_MAX_FLOAT
  * VG_MAX_SCISSOR_RECTS
  * VG_COLOR_TRANSFORM_VALUES
  * VG_COLOR_TRANSFORM
  * VG_SCISSOR_RECTS
  * VG_STROKE_LINE_WIDTH
  * VG_STROKE_JOIN_STYLE - VG_JOIN_ROUND is forced to VG_JOIN_BEVEL
  * VG_STROKE_MITER_LIMIT
  * VG_STROKE_DASH_PATTERN
  * VG_STROKE_DASH_PHASE
  * VG_STROKE_DASH_PHASE_RESET
  * VG_CLEAR_COLOR
  * VG_GLYPH_ORIGIN
  * VG_SCISSORING
* Non supported vgSet/Get*()
  * VG_FILL_RULE - Always VG_EVEN_ODD
  * VG_IMAGE_QUALITY - Always VG_IMAGE_QUALITY_BETTER
  * VG_RENDERING_QUALITY - Always VG_RENDERING_QUALITY_BETTER
  * VG_IMAGE_MODE - Always VG_DRAW_IMAGE_NORMAL
  * VG_MASKING
  * VG_STROKE_CAP_STYLE - Always VG_CAP_BUTT
  * VG_TILE_FILL_COLOR
  * VG_SCREEN_LAYOUT
  * VG_FILTER_FORMAT_LINEAR
  * VG_FILTER_FORMAT_PREMULTIPLIED
  * VG_FILTER_CHANNEL_MASK
  * VG_MAX_KERNEL_SIZE
  * VG_MAX_SEPARABLE_KERNEL_SIZE
  * VG_MAX_GAUSSIAN_STD_DEVIATION
  * VG_MAX_IMAGE_WIDTH
  * VG_MAX_IMAGE_HEIGHT
  * VG_MAX_IMAGE_PIXELS
  * VG_MAX_IMAGE_BYTES
* All matrix operations
* Masking - Non-supported
  * vgMask() - Partially implemented, mask parameter not supported, only VG_CLEAR/FILL_MASK
  * vgRenderToMask() - Supported
  * vgCreateMaskLayer() - Not supported
  * vgDestroyMaskLayer() - Not supported
  * vgFillMaskLayer() - Not supported
  * vgCopyMask() - Not supported
* Paths - supported features:
  * vgCreatePath(), vgClearPath(), vgDestroyPath()
  * vgAppendPathData() - Internally only VG_PATH_DATATYPE_F is used, automatically converted
  * vgPathBounds(), vgPathTransformedBounds()
  * vgDrawPath() - some segment types not supported:
    * VG_SQUAD_TO, VG_SCUBIC_TO
* Paths - non-supported features
  * vgGet/RemovePathCapabilities()
  * vgAppendPath()
  * vgModifyPathCoords()
  * vgTransformPath()
  * vgInterpolatePath()
  * vgPathLength()
  * vgPointAlongPath()
* Paint - everything supported, except patterns and premultiplied color ramps.
* Images - partially supported
  * vgCreateImage - only VG_sRGBA_8888 and VG_IMAGE_QUALITY_BETTER
  * vgDestroyImage
  * vgClearImage
  * vgImageSubData - only VG_sRGBA_8888 and stride = 0
  * vgDrawImage
* Text - glyph rendering using paths supported, not images.
* Image filters - Only vgGaussianBlur is partially implemented, with a gnuVG extension.
  * blur can be rendered directly to the current target by using VG_INVALID_HANDLE as dst.

* Blending - only VG_BLEND_SRC and VG_BLEND_SRC_OVER
* VGU - supported

## Using gnuVG in your program

gnuVG can be used in many ways like regular OpenVG, but with some caveats. A special gnuVG
context must be created before you can call any of the API:s.

```
VGHandle gnuvgCreateContext();  // Creates a new gnuVG context, and returns a handle
void gnuvgDestroyContext(VGHandle context); // Destroys a previously created context
void gnuvgUseContext(VGHandle context); // Indicate to gnuVG which context to use (call for each frame)
void gnuvgResize(VGint pixel_width, VGint pixel_height); // Must be called to set the pixel size
```

## gnuVG API extensions

gnuVG comes with the aforementioned APIs for creating and managing gnuVG contexts. It also
has some other API extensions:

```

// Redirect all rendering to the image instead of on-screen rendering
// pass VG_INVALID_HANDLE to reset to on-screen rendering
VG_API_CALL void VG_API_ENTRY gnuvgRenderToImage(VGImage image) VG_API_EXIT;


// Load a font using libfreetype (currently hardcoded for Android...)
VGFont gnuvgLoadFont(const char* family, gnuVGFontStyle fontStyle);

// Render a UTF-8 encoded string
void gnuvgRenderText(VGFont font, VGfloat size, gnuVGTextAnchor anchor,
                     const char* utf8, VGfloat x_anchor, VGfloat y_anchor);

// Reset compounded boundingbox
void gnuvgResetBoundingBox();

// Get a compounded boundingbox for all render operations
// since last call to gnuvgResetBoundingBox();
void gnuvgGetBoundingBox(VGfloat corner_coordinates[4]);

```