-- test for hierarchical ray-tracers.
-- Thomas Pflaum 1996

marble = gr.material({0.9, 0.8, 0.4}, {0.1, 0.1, 0.1}, 2)
wood = gr.material({0.9, 0.8, 0.4}, {0.1, 0.1, 0.1}, 2)
grass = gr.material({0.3, 0.3, 0.5}, {0.2, 0.2, 0.2}, 25)
metal = gr.material({0.4,0.4,0.4}, {0,0,0}, 25)

scene = gr.node('scene')
scene:rotate('X', 15)
scene:translate(5, -2, -20)

marble_tex = gr.texture("Assets/marble.png")
s1 = gr.sphere('marble sphere')
s1:set_material(marble);
s1:scale(4,4,4)
s1:translate(0,4,0)
s1:set_texture(marble_tex);
--scene:add_child(s1);

wood_tex = gr.texture("Assets/wood.png")
s2 = gr.sphere('wood sphere')
s2:set_material(wood);
s2:scale(3,3,3)
s2:translate(-8,3,4)
s2:set_texture(wood_tex);
--scene:add_child(s2);

s3 = gr.sphere('metal sphere')
s3:scale(4,4,4)
s3:translate(-7,4,-4)
s3:set_material(metal);
--s3:set_texture(wood_tex);
scene:add_child(s3);

mirror = gr.mesh( 'mirror', 'goldfish.obj' )
scene:add_child(mirror)
mirror:rotate('X', 90);
mirror:rotate('Y', -70);
mirror:set_material(metal)
mirror:scale(20, 20, 20)
mirror:translate(-3.5,3.5,-6)

-- the floor
plane = gr.mesh( 'plane', 'plane.obj' )
scene:add_child(plane)
plane:set_material(grass)
plane:scale(30, 30, 30)

-- The lights
l1 = gr.light({40,10,0}, {0.8, 0.8, 0.8}, {1, 0, 0})
--l2 = gr.light({0, 5, -20}, {0.8, 0.8, 0.8}, {1, 0, 0})

gr.render(scene, 'texture-test.png', 400, 400, 
	  {0, 0, 10}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {l1})
