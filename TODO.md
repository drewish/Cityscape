## Block

- Use skeleton to find propertly line then sub-divide lots.

## Building

- Add roof shape
- Color sides
- Optimize mesh building and rendering.
  - Might need a FloorPlan class so we only build the mesh once.
  - If we had separate roof and wall meshes we could scale walls based on height.
- Expand roof outline to overlap house
- More roof styles:
  - gabled
  - gambrel
  - shed

## Lot

- Scale building to fit free space
- Randomly choose building outlines, roof style, height
- Track/mark portions of lot that face a road
- Orient buildings towards street
