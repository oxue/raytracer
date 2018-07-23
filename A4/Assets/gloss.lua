-- test for hierarchical ray-tracers.
-- Thomas Pflaum 1996

glass = gr.material({0.1,0.1,0.1}, {0.1, 0.1, 0.1}, 0, .2, 0.9)
checker = gr.material({0.9, 0.9, 0.9}, {0.4, 0.4, 0.4}, 2, 0, 0)
wood = gr.material({0.0, 0.0, 0.9}, {0.8, 0.8, 0.8}, 25, 0.1, 0.0)
wood2 = gr.material({0.9, 0.0, 0.0}, {0.8, 0.8, 0.8}, 25, 0.1, 0.0)
wood3 = gr.material({0.9, 0.9, 0.9}, {0.8, 0.8, 0.8}, 0, 0.1, 0.0)
glossy_mirror = gr.material({0, 0, 0}, {0.0, 0.0, 0.0}, 0.2, 0.9, 0)

scene = gr.node('scene')
scene:rotate('Y', -50)
scene:rotate('X', 35)
scene:translate(0, -1, 0)

checker_tex = gr.texture("Assets/checkerboard.png")
wood_tex = gr.texture("Assets/wood.png")

s3 = gr.cyl('glass sphere')
s3:scale(2,3,2)
s3:rotate('X', 90)
s3:rotate('Y', 120)


s3:translate(-1,1,-2)
--s3:translate(0,1,0.4)
s3:set_material(wood2);
scene:add_child(s3);

s1 = gr.cone('wood sphere')
s1:scale(1.5,1.5,1.5)
s1:translate(0,1.5,1)

s1:set_material(wood);
--s1:set_texture(wood_tex);
scene:add_child(s1);

-- the floor
plane = gr.mesh( 'plane', 'plane.obj' )
scene:add_child(plane)
plane:set_material(checker)
plane:set_texture(checker_tex);
plane:scale(4, 4, 4)
plane:translate(0,0,-1);

plane = gr.mesh( 'plane', 'water_plane.obj' )
scene:add_child(plane)
plane:set_material(wood3)
plane:set_texture(wood_tex);
plane:scale(2, 2, 2)
plane:translate(0,0,-1);

plane = gr.mesh( 'plane', 'tree_plane.obj' )
scene:add_child(plane)
plane:set_material(glossy_mirror)
plane:scale(8, 8, 8)
plane:rotate('Z', -90);
plane:translate(-1,0,0);

l1 = gr.light({4,0.2,4}, {1, 1, 1}, {1, 0, 0})
--l2 = gr.light({0, 5, -20}, {0.8, 0.8, 0.8}, {1, 0, 0})

gr.render(scene, 'gloss.png', 400, 400, 
	  {0, 0, 10}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {l1})
