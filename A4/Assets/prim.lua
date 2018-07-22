-- test for hierarchical ray-tracers.
-- Thomas Pflaum 1996

glass = gr.material({0.1,0.1,0.1}, {0.5, 0.5, 0.5}, 5, 0.0, 0.6)
checker = gr.material({0.9, 0.9, 0.9}, {0.4, 0.4, 0.4}, 2, 0, 0)
wood = gr.material({0.9, 0.9, 0.9}, {0.3, 0.3, 0.3}, 2, 0.1, 0)
glossy_mirror = gr.material({0, 0, 0}, {0.0, 0.0, 0.0}, 0, 0.8, 0)

scene = gr.node('scene')
scene:rotate('X', 15)
scene:translate(0, -1, 0)

checker_tex = gr.texture("Assets/checkerboard.png")
--wood_tex = gr.texture("Assets/wood.png")

s3 = gr.sphere('glass sphere')
s3:scale(1,1,1)
s3:translate(0,1,0.4)
s3:set_material(glass);
scene:add_child(s3);

s1 = gr.sphere('wood sphere')
s1:scale(1,1,1)
s1:translate(-1,1.5,-2)
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

l1 = gr.light({1,4,1}, {1, 1, 1}, {1, 0, 0})
--l2 = gr.light({0, 5, -20}, {0.8, 0.8, 0.8}, {1, 0, 0})

gr.render(scene, 'prim.png', 400, 400, 
	  {0, 0, 10}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {l1})
