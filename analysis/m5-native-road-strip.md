# Native projected road-strip boundary

This is the first native rendering boundary in the OpenGL direction. It is
intentionally split from the unresolved original road-word decoder.

## Implemented contract

```text
four projected vertices + color
  -> software triangle-strip rasterizer (deterministic fixture path)
  -> OpenGL GL_TRIANGLE_STRIP sink (current-context path)
```

The native game loop separately preserves the observed original transport:

```text
384-word course-table fan-out
  -> base/twin road buffers
  -> 192 even-word FIFO submissions
```

The two boundaries are not silently conflated. The fixture strip is a
rendering test shape, not a claim that its coordinates decode any particular
original RAM field.

## Verification

- `native-road-strip-c` verifies deterministic software rasterization of the
  fixture trapezoid.
- `stunrun-opengl-backend` compiles the same strip sink against the installed
  OpenGL library.
- `STUNRUN_ROAD_STRIP_FIXTURE=1` makes the native shell emit
  `mode=road-strip-fixture` and a nonblank PPM.
- The full native CTest suite passes 26/26.

## Next evidence-required step

Pair one captured road-buffer interval with its contemporaneous GSP display
state, identify a coordinate/depth interpretation for one strip, and replace
the fixture vertices with that decoded record. Until that pairing exists, the
fixture must remain labeled as intermediate rather than original-game output.
