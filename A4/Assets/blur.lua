bricks = gr.material({0.9, 0.9, 0.9}, {0.4, 0.4, 0.4}, 3, 0, 0)
glass = gr.material({0.9, 0.9, 0.9}, {0.2, 0.2, 0.2}, 3, 0, 0.0)
glass2 = gr.material({0.9, 0.9, 0.9}, {0.2, 0.2, 0.2}, 3, 0, 0.0)

scene = gr.node('scene')
scene:rotate('X', 10)
scene:translate(0, -0.5, 0)

wood_texture = gr.texture("Assets/wood.png")
bricks_texture = gr.texture("Assets/bricks.png")


plane = gr.mesh( 'plane', 'plane.obj' )
scene:add_child(plane)
plane:set_material(bricks)
--plane:set_texture(wood_texture);

plane:scale(8, 8, 8)
plane:translate(0,0,-1);

s3 = gr.sphere('glass sphere')
s3:scale(1,1,1)
s3:translate(1,0,0)
s3:set_material(glass);
s3:set_texture(bricks_texture);
scene:add_child(s3);
s3:acceleration(0.00,-0.01,0);
s3:rotational_velocity(0.02,0,0);

s2 = gr.sphere('glass sphere')
s2:scale(1,1,1)
s2:translate(-1,0,3)
s2:set_material(glass2);
s2:set_texture(wood_texture);
scene:add_child(s2);
s2:acceleration(0.00,-0.01,0);
s2:rotational_velocity(0.02,0,0);

l1 = gr.light({10,5,10}, {0.8, 0.8, 0.8}, {1, 0, 0})
l2 = gr.light({-10,5, 10}, {0.0, 0.8, 0.0}, {1, 0, 0})

gr.render(scene, 'blur.png', 400, 400, 
	  {0, 0, 8}, {0, 0, -1}, {0, 1, 0}, 40,
	  {0.4, 0.4, 0.4}, {l1})