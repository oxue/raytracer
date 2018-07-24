bricks = gr.material({0.2, 0.2, 0.2}, {0.4, 0.4, 0.4}, 3, 0, 0)
fish_mat = gr.material({0.99, 0.5, 0.0}, {0.9, 0.9, 0.9}, 25, 0.2, 0.0)
glass2 = gr.material({0.9, 0.9, 0.9}, {0.5, 0.5, 0.5}, 3, 0, 0.0)
glass = gr.material({0.1,0.1,0.1}, {1, 1, 1}, 0, 0.1, 0.9)
leaf_mat = gr.material({0.9, 0.9, 0.9}, {0.5, 0.5, 0.5}, 0, 0, 0.0)
tree_mat = gr.material({0.9, 0.9, 0.9}, {0.5, 0.5, 0.5}, 0, 0, 0.0)
water_mat = gr.material({0.0, 0.0, 0.0}, {0, 0, 0}, 0, 1, 0.1)

red = gr.material({0.8, 0.0, 0.0}, {0.4, 0.4, 0.4}, 3, 0, 0)
green = gr.material({0.8, 0.0, 0.8}, {0.4, 0.4, 0.4}, 3, 0, 0)
blue = gr.material({0.0, 0.0, 0.8}, {0.4, 0.4, 0.4}, 3, 0, 0)

colors = {}
colors[0] = red;
colors[1] = green;
colors[2] = blue;

scene = gr.node('scene')
--scene:rotate('Y', 90)
--scene:rotate('X', 5)
scene:translate(0, 0, -3)
scene:rotate('X', 35)

math.randomseed(8)


wood_texture = gr.texture("Assets/wood.png", false)
bricks_texture = gr.texture("Assets/bricks.png", false)
ground_texture = gr.texture("Assets/ground.png", false)
leaf_texture = gr.texture("Assets/leaf.png", false)
tree_tex = gr.texture("Assets/treeimage.png", true)
water_norm = gr.texture("Assets/water_norm.png", false)

-- LEAF --
leaf = gr.sphere('sphere')
leaf:rotate('Z', 90)
leaf:scale(1.3,0.05,1.3)
leaf:scale(0.2,0.2,0.2)
leaf:translate(0,0.04,0)
leaf:set_material(leaf_mat)
leaf:set_texture(leaf_texture)

-- TREE -- 
tree_mesh = gr.mesh('tree', "tree_plane.obj")
tree_mesh:scale(2.5,4,4)
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
tree2:translate(5,2,-18)
scene:add_child(tree2)

tree3 = gr.node("tree3")
tree3:add_child(tree_mesh)
--tree3:rotate('Y', 180)
tree3:translate(-5,2,-18)
scene:add_child(tree3)

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
water2:translate(0,0,-12)

-- FLOOR -- 
floor_mesh = gr.mesh('ground', 'plane.obj')
floor_mesh:set_material(bricks)
floor_mesh:set_texture(bricks_texture)

floor = gr.node("floor")
floor:add_child(floor_mesh)
scene:add_child(floor)
floor:scale(6,6,6)
floor:translate(0,-5,0)

floor2 = gr.node("floor2")
floor2:add_child(floor_mesh)
scene:add_child(floor2)
floor2:scale(6,6,6)
floor2:translate(0,-5,-12)

floor3 = gr.node("floor3")
floor3:add_child(floor_mesh)
scene:add_child(floor3)
floor3:scale(6,6,6)
floor3:rotate('Z', -90);
floor3:translate(-6,-5,-12)

floor4 = gr.node("floor4")
floor4:add_child(floor_mesh)
scene:add_child(floor4)
floor4:scale(6,6,6)
floor4:rotate('Z', 90);
floor4:translate(6,-5,-12)

-- WALL -- 
wall_mesh = gr.mesh( 'wall', 'plane.obj' )
wall_mesh:set_material(bricks)
wall1 = gr.node("wall")
wall1:add_child(wall_mesh)
scene:add_child(wall1)
wall1:scale(6,6,6)
wall1:rotate('X', 90)
wall1:translate(0,-5,-18)

pond_rad = 6;

for i = 100,1,-1 
do 
   xr = (math.random()-.5) * 12;
   zr = (math.random()-.5) * 24 - 6;
   rr = math.random() * 180;

   leaf_instance = gr.node('leaf')
   scene:add_child(leaf_instance)
   leaf_instance:add_child(leaf)
   leaf_instance:rotate('Y', rr)
   leaf_instance:translate(xr, 0, zr)

   sr = math.floor(math.random() * 10)
   cr = math.floor(math.random() * 3)
   if sr == 0 then
   		shape = gr.cone('cone')
   		shape:set_material(colors[cr])
   		scene:add_child(shape)
   		shape:scale(0.20,0.17,0.20)
   		shape:translate(xr, 0.05 + 0.17, zr)
   	elseif sr == 1 then
   		shape = gr.sphere('sphere')
   		shape:set_material(colors[cr])
   		scene:add_child(shape)
   		shape:scale(0.12,0.12,0.12)
   		shape:translate(xr, 0.05 + 0.12, zr)
   	elseif sr == 2 then
   		shape = gr.cube('cube')
   		shape:set_material(colors[cr])
   		scene:add_child(shape)
   		shape:scale(0.24,0.24,0.24)
   		shape:translate(xr - 0.12, 0.05, zr - 0.12)
    end
end

ball_instance = gr.sphere('ball')
ball_instance:scale(0.5,0.5,0.5)
ball_instance:set_material(glass);
scene:add_child(ball_instance)
ball_instance:translate(0, 1, 0)

white = gr.material({0.9, 0.9, 0.9}, {0, 0, 0}, 0, 0, 0)
black = gr.material({0.1, 0.1, 0.1}, {0.9, 0.9, 0.9}, 25, 0, 0)

goldfishsize = 1;
fish = gr.mesh('goldfish', 'goldfish.obj')
fish:scale(3,1.5,1.5)
fish:set_material(fish_mat);

fisheye = gr.sphere('eye');
fisheye:set_material(white);
fisheye:scale(0.05,0.05,0.05);
fisheye:translate(-0.18,0.10,0.4);

fisheye2 = gr.sphere('eye');
fisheye2:set_material(white);
fisheye2:scale(0.05,0.05,0.05);
fisheye2:translate(0.18,0.10,0.4);

fishpupil = gr.sphere('pupil');
fishpupil:set_material(black);
fishpupil:scale(0.03,0.03,0.03);
fishpupil:translate(-0.21,0.10,0.4);

fishpupil2 = gr.sphere('pupil');
fishpupil2:set_material(black);
fishpupil2:scale(0.03,0.03,0.03);
fishpupil2:translate(0.21,0.10,0.4);

fishnode = gr.node('fish')
fishnode:add_child(fish)
fishnode:add_child(fisheye)
fishnode:add_child(fisheye2)
fishnode:add_child(fishpupil)
fishnode:add_child(fishpupil2)

fish1 = gr.node('fish1')
fish1:add_child(fishnode)
fish1:rotate('X', 10)
fish1:rotate('Y', 65)
fish1:translate(0,-0.5,4)

scene:add_child(fish1);

l1 = gr.light({5,5,5}, {0.8, 0.8, 0.8}, {1, 0, 0})
l2 = gr.light({-5,5,5}, {0.8, 0.8, 0.8}, {1, 0, 0})

resolution = 150
gr.render(scene, 'mesh.png', resolution, 1.2 * resolution, 
	  {0, 0, 4}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {l1})