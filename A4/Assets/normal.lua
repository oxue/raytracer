bricks = gr.material({0.9, 0.9, 0.9}, {0.4, 0.4, 0.4}, 3, 0, 0)
glass = gr.material({0.1,0.1,0.1}, {1, 1, 1}, 0, 0.1, 0.9)

bricks_norm = gr.texture("Assets/bricks_normal.png")
bricks_texture = gr.texture("Assets/bricks.png")

scene = gr.node('scene')
scene:rotate('X', 30)
scene:translate(0, -1, 0)

plane = gr.mesh( 'plane', 'plane.obj' )
scene:add_child(plane)
plane:set_material(bricks)
plane:set_texture(bricks_texture);
plane:set_normalmap(bricks_norm);
plane:scale(4, 4, 4)
plane:translate(0,0,-1);


s3 = gr.sphere('glass sphere')
s3:scale(1,1,1)
s3:translate(0,1.5,0)
s3:set_material(glass);
scene:add_child(s3);

l1 = gr.light({10,5,10}, {0.8, 0.0, 0.8}, {1, 0, 0})
l2 = gr.light({-10,5, 10}, {0.0, 0.8, 0.0}, {1, 0, 0})

gr.render(scene, 'normal.png', 400, 400, 
	  {0, 0, 10}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {l1, l2})