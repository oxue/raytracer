bricks = gr.material({0.2, 0.2, 0.2}, {0.4, 0.4, 0.4}, 3, 0, 0)
fish_mat = gr.material({0.3, 0.5, 0.3}, {0.9, 0.9, 0.9}, 25, 0.2, 0.0)
glass2 = gr.material({0.9, 0.9, 0.9}, {0.5, 0.5, 0.5}, 3, 0, 0.0)
glass = gr.material({0.1,0.1,0.1}, {1, 1, 1}, 0, 0.1, 0.9)
leaf_mat = gr.material({0.9, 0.9, 0.9}, {0.5, 0.5, 0.5}, 0, 0, 0.0)
tree_mat = gr.material({0.9, 0.9, 0.9}, {0.5, 0.5, 0.5}, 0, 0, 0.0)

water_mat = gr.material({0.0, 0.0, 0.0}, {0, 0, 0}, 0, 1, 0.1)

scene = gr.node('scene')
--scene:rotate('Y', 90)
--scene:rotate('X', 5)
scene:translate(0, 0, -3)
scene:rotate('X', 35)

resolution = 200

wood_texture = gr.texture("Assets/wood.png", false)
bricks_texture = gr.texture("Assets/bricks.png", false)
ground_texture = gr.texture("Assets/ground.png", false)
leaf_texture = gr.texture("Assets/leaf.png", false)
tree_tex = gr.texture("Assets/treeimage.png", true)
water_norm = gr.texture("Assets/water_norm.png", false)

-- LEAF --
leaf = gr.sphere('sphere')
leaf:rotate('Z', 90)
leaf:scale(1,0.05,0.6)
leaf:scale(0.2,0.2,0.2)
leaf:translate(0,0.02,0)
leaf:set_material(leaf_mat)
leaf:set_texture(leaf_texture)

-- TREE -- 
tree_mesh = gr.mesh('tree', "tree_plane.obj")
tree_mesh:scale(4,4,4)
tree_mesh:set_material(tree_mat)
tree_mesh:set_texture(tree_tex)
tree_mesh:rotate('X', 90)

tree1 = gr.node("tree1")
tree1:add_child(tree_mesh)
tree1:translate(0, 4, -18)
scene:add_child(tree1)

tree2 = gr.node("tree2")
tree2:add_child(tree_mesh)
--tree2:rotate('Y', 180)
tree2:translate(3,-3,-18.3)
--scene:add_child(tree2)

-- WATER --
water = gr.mesh( 'water', 'water_plane.obj' )
scene:add_child(water)
water:set_material(water_mat)
water:set_normalmap(water_norm)
water:scale(6, 6, 6)

water2 = gr.mesh( 'water', 'water_plane.obj' )
scene:add_child(water2)
water2:set_material(water_mat)
water2:scale(6, 6, 6)
water2:translate(1,0,-12)

-- FLOOR -- 
floor_mesh = gr.mesh('ground', 'plane.obj')
floor_mesh:set_material(bricks)
floor_mesh:set_texture(bricks_texture)

floor = gr.node("floor")
floor:add_child(floor_mesh)
scene:add_child(floor)
floor:scale(6,6,6)
floor:translate(0,-2,0)

floor2 = gr.node("floor2")
floor2:add_child(floor_mesh)
scene:add_child(floor2)
floor2:scale(6,6,6)
floor2:translate(0,-2,-12)

-- WALL -- 
wall_mesh = gr.mesh( 'wall', 'plane.obj' )
wall_mesh:set_material(bricks)
wall1 = gr.node("wall")
wall1:add_child(wall_mesh)
scene:add_child(wall1)
wall1:scale(6,6,6)
wall1:rotate('X', 90)
wall1:translate(0,-5.8,-18)

pond_rad = 8;

for i = 100,1,-1 
do 
   xr = (math.random()-.5) * pond_rad;
   zr = (math.random()-.5) * pond_rad * 4 - pond_rad - 1;
   rr = math.random() * 180;

   leaf_instance = gr.node('leaf')
   scene:add_child(leaf_instance)
   leaf_instance:add_child(leaf)
   leaf_instance:rotate('Y', rr)
   leaf_instance:translate(xr, 0, zr)

end

ball_instance = gr.sphere('ball')
ball_instance:scale(0.3,0.3,0.3)
ball_instance:set_material(glass);
scene:add_child(ball_instance)
ball_instance:translate(0, 1, 0)

for i = 10,1,-1 
do 
   xr = (math.random()-.5) * pond_rad;
   zr = (math.random()-.5) * pond_rad * 4 - pond_rad - 1;
   --rr = math.random() * 180;

   

end


goldfishsize = 1;
fish = gr.mesh('goldfish', 'goldfish.obj')
fish:scale(5,5,2)
fish:rotate('X', 15)
fish:rotate('Y', 40)
fish:translate(0,-2,3)
fish:set_material(fish_mat);
--fish:set_texture(bricks_texture);
--scene:add_child(fish);

l1 = gr.light({5,5,5}, {0.8, 0.8, 0.8}, {1, 0, 0})
l2 = gr.light({-5,5,5}, {0.8, 0.8, 0.8}, {1, 0, 0})

gr.render(scene, 'mesh.png', resolution, 1.2 * resolution, 
	  {0, 0, 4}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {l1,l2})