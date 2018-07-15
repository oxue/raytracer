rootnode = gr.node('root') --0
rootnode:scale( .25, .25, .25 )
rootnode:translate(0, 0, -2)

red = gr.material({1, 0, 0}, {.1, .1, .1}, 10)
pink = gr.material({1, .7, .7}, {.1, .1, .1}, 10)
saddle_brown = gr.material({139/255,69/255,19/255}, {.1, .1, .1}, 10)
white = gr.material({1, 1, 1}, {.1, .1, .1}, 10)
yellow = gr.material({1, 1, 0}, {.1, .1, .1}, 10)

torso = gr.mesh('sphere', 'torso') --1
rootnode:add_child(torso)
torso:set_material(pink)
torso:scale(.7, .7, .6)

shoulders = gr.mesh('sphere', 'shoulders') --2
torso:add_child(shoulders)
shoulders:set_material(saddle_brown)
shoulders:scale(1, 1, 1/.5)
shoulders:scale(1, .5, .5)
shoulders:translate(0, .6, 0)

shoulderHidden = gr.mesh('sphere', 'shoulderHidden') --3
shoulders:add_child(shoulderHidden)
shoulderHidden:set_material(saddle_brown)
shoulderHidden:scale(1/1, 1/.5, 1/.5)
shoulderHidden:scale(.2, .2, .2)
shoulderHidden:translate(0, .8, 0)

shoulderjoint = gr.joint('neckjoint', {0, 0, 0}, {-90, 0, 90}) --4
shoulderHidden:add_child(shoulderjoint)

neckSphere = gr.mesh('sphere', 'neckSphere') --5
shoulderjoint:add_child(neckSphere)
neckSphere:scale(1/.2, 1/.2, 1/.2)
neckSphere:scale(.1, .1, .1)
neckSphere:translate(0, 1, 0)
neckSphere:set_material(saddle_brown)

headjoint = gr.joint('headjoint', {-30, 0, 30}, {0, 0, 0}) --6
neckSphere:add_child(headjoint)

head = gr.mesh('suzanne', 'head') --7
headjoint:add_child(head)
head:scale(1/.1, 1/.1, 1/.1)
head:scale(.7, .7, .7)
head:translate(0, 2.5, 0)
head:set_material(saddle_brown)

lShoulderSphere = gr.mesh('sphere', 'lShoulderSphere') --8
torso:add_child(lShoulderSphere)
lShoulderSphere:scale(1/.7, 1/.7, 1/.6)
lShoulderSphere:scale(.2, .2, .2)
lShoulderSphere:set_material(saddle_brown)
lShoulderSphere:translate(-1.2, .6, 0)

lUpperJoint = gr.joint('lUpperJoint', {-60, 0, 60}, {0, 0, 0}) --9
lShoulderSphere:add_child(lUpperJoint)

lUpperArm = gr.mesh('sphere', 'lUpperArm') --10
lUpperJoint:add_child(lUpperArm)
lUpperArm:scale(1/.2, 1/.2, 1/.2)
lUpperArm:scale(.1, .5, .1)
lUpperArm:set_material(pink)
lUpperArm:translate(0, -1.5, 0)

lElbowSphere = gr.mesh('sphere', 'lElbowSphere')  --11
lUpperArm:add_child(lElbowSphere)
lElbowSphere:scale(1/.1, 1/.5, 1/.1)
lElbowSphere:scale(.2, .2, .2)
lElbowSphere:set_material(saddle_brown)
lElbowSphere:translate(0, -.5, 0)

lElbowJoint = gr.joint('lElbowJoint', {-60, 0, 60}, {0, 0, 0}) --12
lElbowSphere:add_child(lElbowJoint)

lLowerArm = gr.mesh('sphere', 'lLowerArm')  --13
lElbowJoint:add_child(lLowerArm)
lLowerArm:scale(1/.2, 1/.2, 1/.2)
lLowerArm:scale(.1, .5, .1)
lLowerArm:set_material(pink)
lLowerArm:translate(0, -1, 0)

lHandSphere = gr.mesh('sphere', 'lHandSphere') --014
lLowerArm:add_child(lHandSphere)
lHandSphere:scale(1/.1, 1/.5, 1/.1)
lHandSphere:scale(.2, .2, .2)
lHandSphere:set_material(saddle_brown)
lHandSphere:translate(0, -.8, 0)

lHandJoint = gr.joint('lHandJoint', {-60, 0, 60}, {0, 0, 0}) --015
lHandSphere:add_child(lHandJoint)

lHand = gr.mesh('sphere', 'lHand') --16
lHandJoint:add_child(lHand)
lHand:scale(1/.2, 1/.2, 1/.2)
lHand:scale(.3, .2, .3)
lHand:set_material(pink)
lHand:translate(0, -1, 0)


rShoulderSphere = gr.mesh('sphere', 'rShoulderSphere') --17
torso:add_child(rShoulderSphere)
rShoulderSphere:scale(1/.7, 1/.7, 1/.6)
rShoulderSphere:scale(.2, .2, .2)
rShoulderSphere:set_material(saddle_brown)
rShoulderSphere:translate(1.2, .6, 0)

rUpperJoint = gr.joint('rUpperJoint', {-60, 0, 60}, {0, 0, 0}) --18
rShoulderSphere:add_child(rUpperJoint)

rUpperArm = gr.mesh('sphere', 'rUpperArm') --19
rUpperJoint:add_child(rUpperArm)
rUpperArm:scale(1/.2, 1/.2, 1/.2)
rUpperArm:scale(.1, .5, .1)
rUpperArm:set_material(pink)
rUpperArm:translate(0, -1.5, 0)

rElbowSphere = gr.mesh('sphere', 'rElbowSphere') --20
rUpperArm:add_child(rElbowSphere)
rElbowSphere:scale(1/.1, 1/.5, 1/.1)
rElbowSphere:scale(.2, .2, .2)
rElbowSphere:set_material(saddle_brown)
rElbowSphere:translate(0, -.5, 0)

rElbowJoint = gr.joint('rElbowJoint', {-60, 0, 60}, {0, 0, 0})--21
rElbowSphere:add_child(rElbowJoint)


rLowerArm = gr.mesh('sphere', 'rLowerArm')--22
rElbowJoint:add_child(rLowerArm)
rLowerArm:scale(1/.2, 1/.2, 1/.2)
rLowerArm:scale(.1, .5, .1)
rLowerArm:set_material(pink)
rLowerArm:translate(0, -1, 0)

rHandSphere = gr.mesh('sphere', 'rHandSphere')--23
rLowerArm:add_child(rHandSphere)
rHandSphere:scale(1/.1, 1/.5, 1/.1)
rHandSphere:scale(.2, .2, .2)
rHandSphere:set_material(saddle_brown)
rHandSphere:translate(0, -.8, 0)

rHandJoint = gr.joint('rHandJoint', {-60, 0, 60}, {0, 0, 0})--24
rHandSphere:add_child(rHandJoint)

rHand = gr.mesh('sphere', 'rHand')--25
rHandJoint:add_child(rHand)
rHand:scale(1/.2, 1/.2, 1/.2)
rHand:scale(.3, .2, .3)
rHand:set_material(pink)
rHand:translate(0, -1, 0)

bananaJoint = gr.joint('bananaJoint', {-60, 0, 60}, {0, 0, 0})--24
rHand:add_child(bananaJoint)

banana = gr.mesh('sphere', 'rHand')--25
bananaJoint:add_child(banana);
banana:scale(1/.3, 1/.2, 1/.3);
banana:scale(.15,.15,.7);
banana:rotate('y', 20);
banana:rotate('x', 30);

banana:set_material(yellow);

hip = gr.mesh('sphere', 'hip')--26
torso:add_child(hip)
hip:set_material(saddle_brown)
hip:scale(1, 1, 1/.5)
hip:scale(1, .6, .6)
hip:translate(0, -.5, 0)

bellybutton = gr.mesh('sphere', 'bellybutton')
torso:add_child(bellybutton);
bellybutton:scale(1, 1, 1/.5)
bellybutton:scale(.15,.15,.1);
bellybutton:translate(0,0,.9);
bellybutton:set_material(pink);


lHipSphere = gr.mesh('sphere', 'lHipSphere')--27
torso:add_child(lHipSphere)
lHipSphere:scale(1/.7, 1/.7, 1/.6)
lHipSphere:scale(.2, .2, .2)
lHipSphere:set_material(saddle_brown)
lHipSphere:translate(-.6, -.65, 0)

lLegUpperJoint = gr.joint('lLegUpperJoint', {-60, 0, 60}, {0, 0, 0})--28
lHipSphere:add_child(lLegUpperJoint)

lUpperLeg = gr.mesh('sphere', 'lUpperLeg')--29
lLegUpperJoint:add_child(lUpperLeg)
lUpperLeg:scale(1/.2, 1/.2, 1/.2)
lUpperLeg:scale(.1, .5, .1)
lUpperLeg:set_material(pink)
lUpperLeg:translate(0, -1.5, 0)

lLegElbowSphere = gr.mesh('sphere', 'lLegElbowSphere')
lUpperLeg:add_child(lLegElbowSphere)
lLegElbowSphere:scale(1/.1, 1/.5, 1/.1)
lLegElbowSphere:scale(.2, .2, .2)
lLegElbowSphere:set_material(saddle_brown)
lLegElbowSphere:translate(0, -.5, 0)

lLegElbowJoint = gr.joint('lLegElbowJoint', {-60, 0, 60}, {0, 0, 0})
lLegElbowSphere:add_child(lLegElbowJoint)

lLowerLeg = gr.mesh('sphere', 'lLowerLeg')
lLegElbowJoint:add_child(lLowerLeg)
lLowerLeg:scale(1/.2, 1/.2, 1/.2)
lLowerLeg:scale(.1, .5, .1)
lLowerLeg:set_material(pink)
lLowerLeg:translate(0, -1, 0)

lFootSphere = gr.mesh('sphere', 'lFootSphere')
lLowerLeg:add_child(lFootSphere)
lFootSphere:scale(1/.1, 1/.5, 1/.1)
lFootSphere:scale(.2, .2, .2)
lFootSphere:set_material(saddle_brown)
lFootSphere:translate(0, -.8, 0)


lFootJoint = gr.joint('lFootJoint', {-60, 0, 60}, {0, 0, 0})
lFootSphere:add_child(lFootJoint)

lFoot = gr.mesh('sphere', 'lFoot')
lFootJoint:add_child(lFoot)
lFoot:scale(1/.2, 1/.2, 1/.2)
lFoot:scale(.3, .2, .3)
lFoot:set_material(pink)
lFoot:translate(0, -1, 0)

rHipSphere = gr.mesh('sphere', 'rHipSphere')
torso:add_child(rHipSphere)
rHipSphere:scale(1/.7, 1/.7, 1/.6)
rHipSphere:scale(.2, .2, .2)
rHipSphere:set_material(saddle_brown)
rHipSphere:translate(.6, -.65, 0)

rLegUpperJoint = gr.joint('rLegUpperJoint', {-60, 0, 60}, {0, 0, 0})
rHipSphere:add_child(rLegUpperJoint)

rUpperLeg = gr.mesh('sphere', 'rUpperLeg')
rLegUpperJoint:add_child(rUpperLeg)
rUpperLeg:scale(1/.2, 1/.2, 1/.2)
rUpperLeg:scale(.1, .5, .1)
rUpperLeg:set_material(pink)
rUpperLeg:translate(0, -1.5, 0)

rLegElbowSphere = gr.mesh('sphere', 'rLegElbowSphere')
rUpperLeg:add_child(rLegElbowSphere)
rLegElbowSphere:scale(1/.1, 1/.5, 1/.1)
rLegElbowSphere:scale(.2, .2, .2)
rLegElbowSphere:set_material(saddle_brown)
rLegElbowSphere:translate(0, -.5, 0)

rLegElbowJoint = gr.joint('rLegElbowJoint', {-60, 0, 60}, {0, 0, 0})
rLegElbowSphere:add_child(rLegElbowJoint)

rLowerLeg = gr.mesh('sphere', 'rLowerLeg')
rLegElbowJoint:add_child(rLowerLeg)
rLowerLeg:scale(1/.2, 1/.2, 1/.2)
rLowerLeg:scale(.1, .5, .1)
rLowerLeg:set_material(pink)
rLowerLeg:translate(0, -1, 0)

rFootSphere = gr.mesh('sphere', 'rFootSphere')
rLowerLeg:add_child(rFootSphere)
rFootSphere:scale(1/.1, 1/.5, 1/.1)
rFootSphere:scale(.2, .2, .2)
rFootSphere:set_material(saddle_brown)
rFootSphere:translate(0, -.8, 0)

rFootJoint = gr.joint('rFootJoint', {-60, 0, 60}, {0, 0, 0})
rFootSphere:add_child(rFootJoint)

rFoot = gr.mesh('sphere', 'rFoot')
rFootJoint:add_child(rFoot)
rFoot:scale(1/.2, 1/.2, 1/.2)
rFoot:scale(.3, .2, .3)
rFoot:set_material(pink)
rFoot:translate(0, -1, 0)

return rootnode

gr.render(scene_root, 'simple.png', 256, 256,
	  {0, 0, 800}, {0, 0, -800}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {orange_light, white_light})
