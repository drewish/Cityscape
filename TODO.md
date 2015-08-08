## Bugs
- fix missing segments in sawtooth roof

## Global

- Decide how to best pass references down the hierarchy so that a lot can find
  its overall position in the city
- Draw backdrop for sky
- Draw a yellowish ground color (or just use that as the background color if we 
  need no fade)

## Block

- Use skeleton to find property line then sub-divide lots.

## Building

- Limit shader's pallet to 3-colors for the building faces
- Separate types of buildings
  - house
  - urban (fills lot, perhaps with small set back on front and back?)
  - sky scraper
  - warehouse
  - san francisco (bay windows on street side of building)
- Optimize mesh building and rendering.
  - Separate meshes for roof and walls. Scale walls for floors.
  - Cache plans by outline so they're not rebuilt every time the layout changes.
- Expand roof outline to overlap house (soffit)
- More roof styles:
  - saltbox
  - gabled
  - round

## Lot

- Leave occasional ones empty
- If the building doesn't fit try varying the rotation and/or position 
- Scale building to fit free space
- Track/mark portions of lot that face a road
- Orient buildings towards street
