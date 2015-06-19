## World

- Add options to toggle lot subdivision and building styles
- Draw a yellowish ground color (or just use that as the background color if we need no fade)
- Add Lot mode
- Create object specific options. Perhaps as static variables on the object 
  class? Otherwise class specific structs.

## Building

- Limit shader's pallet to 3-colors for the building faces
- Separate types of buildings
  - house
  - urban (fills lot, perhaps with small set back on front and back?)
  - sky scraper
  - warehouse
- Optimize mesh building and rendering.
  - Cache plans by outline so they're not rebuilt every time the layout changes.
  - If we had separate roof and wall meshes we could scale walls based on height.
- Expand roof outline to overlap house (soffit)
- More roof styles:
  - sawtooth (warehouses)
  - saltbox
  - gabled
  - shed

## Lot

- If the building doesn't fit try varying the rotation and/or position 
- Scale building to fit free space
- Track/mark portions of lot that face a road
- Orient buildings towards street

## Block

- Use skeleton to find property line then sub-divide lots.

