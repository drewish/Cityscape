## Block

- Use skeleton to find propertly line then sub-divide lots.

## Building

- Separate types of buildings
  - house
  - urban (fills lot, perhaps with small set back on front and back?)
  - sky scraper
  - warehouse
- Use a shader to do 3-coloring of the building faces
- Optimize mesh building and rendering.
  - Might need a FloorPlan class so we only build the mesh once.
  - If we had separate roof and wall meshes we could scale walls based on height.
- Expand roof outline to overlap house (soffit)
- More roof styles:
  - sawtooth (warehouses)
  - saltbox
  - gabled
  - gambrel
  - shed

## Lot

- Scale building to fit free space
- Randomly choose building outlines, roof style, height
- Track/mark portions of lot that face a road
- Orient buildings towards street
