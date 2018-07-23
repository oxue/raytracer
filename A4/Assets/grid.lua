red = gr.material({0.8, 0,0}, {0.4, 0.4, 0.4}, 3, 0, 0)
green = gr.material({0, 0.8, 0}, {0.4, 0.4, 0.4}, 3, 0, 0)
blue = gr.material({0,0, 0.8}, {0.4, 0.4, 0.4}, 3, 0, 0)

colors = {}
colors[0] = red;
colors[1] = green;
colors[2] = blue;
size = 4

scene = gr.node('scene')

math.randomseed(10);

for i = 10000,1,-1 
do 
   xr = (math.random()) * size
   yr = (math.random()) * size
   zr = 1 
   --(math.random()) * -size

   ball = gr.sphere("ball")
   scene:add_child(ball)
   ball:scale(0.1,0.1,0.1)
   ball:set_material(colors[math.floor(math.random() * 3)])
   ball:translate(xr,yr,zr)
end


l1 = gr.light({4,0.2,4}, {1, 1, 1}, {1, 0, 0})
--l2 = gr.light({0, 5, -20}, {0.8, 0.8, 0.8}, {1, 0, 0})

gr.render(scene, 'grid.png', 400, 400, 
	  {2.5, 2.5, 10}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {l1})