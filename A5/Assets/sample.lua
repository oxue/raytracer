-- test for hierarchical ray-tracers.
-- Thomas Pflaum 1996

gold = gr.material({0.9, 0.8, 0.4}, {0.8, 0.8, 0.4}, 25)
grass = gr.material({0.6, 0.6, 1}, {0.3, 0.3, 0.3}, 2)
rock = gr.material({0.2, 0.2, 0.2}, {0.2, 0.2, 0.2}, 25)
snow = gr.material({1, 1, 1}, {0.0, 0.0, 0.0}, 25)
wood = gr.material({0.6, 0.3, 0.1}, {0.0, 0.0, 0.0}, 25)
leaves = gr.material({0.0, 0.4, 0.0}, {0.0, 0.0, 0.0}, 25)

scene = gr.node('scene')
scene:rotate('Y', 20)
scene:rotate('X', 23)
--
scene:translate(6, -2, -15)


-- the floor
plane = gr.mesh( 'plane', 'plane.obj' )
scene:add_child(plane)
plane:set_material(snow)
plane:rotate('X', 5)
plane:scale(30, 30, 30)
plane:translate(0,1.5,-7)

plane2 = gr.mesh( 'plane2', 'plane.obj' )
scene:add_child(plane2)
plane2:set_material(grass)
plane2:scale(30, 30, 30)
plane2:translate(0,1.5,0)
--plane:rotate('X', 30)

--tree
tree = gr.node("tree")
trunk = gr.mesh("cylinder", "cylinder.obj")
trunk:set_material(wood)
trunk:scale(0.5,3,0.5);
tree:add_child(trunk)

bush1 = gr.cube("cube1")
bush1:set_material(leaves)
bush1:scale(2,2,2)
bush1:translate(-1,2,-1)
tree:add_child(bush1)

bush2 = gr.cube("cube2")
bush2:set_material(leaves)
bush2:scale(1.3,2,1.3)
bush2:translate(-0.65,4,-0.65)
tree:add_child(bush2)

tree1 = gr.node("tree1")
tree1:add_child(tree);
scene:add_child(tree1);
tree1:translate(0,2,-15)

tree2 = gr.node("tree2")
tree2:add_child(tree);
scene:add_child(tree2);
tree2:translate(4,2,-16)

tree3 = gr.node("tree3")
tree3:add_child(tree);
scene:add_child(tree3);
tree3:translate(2,2.5,-17)

--cow 
cow = gr.mesh("cow", "cow.obj")
cow:set_material(wood)
scene:add_child(cow)
cow:translate(-23, 6.618034, 8)
cow:rotate('Y', -50);
cow:scale(0.5,0.5,0.5)

--snowman
snowman = gr.node("snowman")
scene:add_child(snowman);
s1 = gr.sphere('s1')
s1:set_material(snow)

s2 = gr.sphere('s2')
s2:set_material(snow)
s2:scale(0.8,0.8,0.8)
s2:translate(0,1.2,0)
s3 = gr.sphere('s3')
s3:set_material(snow)
s3:scale(0.4,0.4,0.4)
s3:translate(0,2,0)
snowman:add_child(s1);
snowman:add_child(s2);
snowman:add_child(s3);
snowman:scale(1.5, 1.5, 1.5)
snowman:translate(-5,3,-13)

-- sphere
rock1 = gr.mesh( 'rock1', 'dodeca.obj' )
scene:add_child(rock1)
rock1:translate(-2, 1.618034, 0)
rock1:set_material(rock)

rock2 = gr.mesh( 'rock2', 'dodeca.obj' )
scene:add_child(rock2)
rock2:translate(-9, 2.618034, 0)
rock2:scale(0.5,0.5,0.5);
rock2:set_material(rock)

-- The lights
l1 = gr.light({400,200,400}, {0.8, 0.8, 0.8}, {1, 0, 0})
--l2 = gr.light({0, 5, -20}, {0.4, 0.4, 0.8}, {1, 0, 0})

gr.render(scene, 'sample.png', 500, 500, 
	  {0, 0, 0,}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {l1, l2})
